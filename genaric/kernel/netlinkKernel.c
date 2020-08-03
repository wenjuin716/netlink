#include <linux/module.h>
#include <net/netlink.h>
#include <net/genetlink.h>
#include <linux/version.h>

#define TEST_GENL_MSG_FROM_KERNEL   "Hello from kernel space!!!"
//#define GENL_GROUP	(1<<0)
#define GENL_GROUP	0x1
 
/* handler
 * message handling code goes here; return 0 on success, negative
 * values on failure
 */
static int doc_exmpl_echo(struct sk_buff *skb, struct genl_info *info);
static int doc_exmpl_tmp(struct sk_buff *skb, struct genl_info *info);
 
 
/* netlink attributes */
enum {
      DOC_EXMPL_A_UNSPEC,
      DOC_EXMPL_A_MSG,
      __DOC_EXMPL_A_MAX,
};
#define DOC_EXMPL_A_MAX (__DOC_EXMPL_A_MAX - 1)
 
/* attribute policy */
static struct nla_policy doc_exmpl_genl_policy[DOC_EXMPL_A_MAX + 1] = {
      [DOC_EXMPL_A_MSG] = { .type = NLA_NUL_STRING },
};
 
/* commands 定义命令类型，用户空间以此来表明需要执行的命令 */
enum {
    DOC_EXMPL_C_UNSPEC,
    DOC_EXMPL_C_ECHO,
    DOC_EXMPL_C_TMP,
    __DOC_EXMPL_C_MAX,
};
#define DOC_EXMPL_C_MAX (__DOC_EXMPL_C_MAX - 1)
 
/* operation definition 将命令command echo和具体的handler对应起来 */
static struct genl_ops doc_exmpl_genl_ops_echo[] = {
{
    .cmd = DOC_EXMPL_C_ECHO,
    .flags = 0,
    .policy = doc_exmpl_genl_policy,
    .doit = doc_exmpl_echo,
    .dumpit = NULL,
    .done = NULL,
},
{
    .cmd = DOC_EXMPL_C_TMP,
    .flags = 0,
    .doit = doc_exmpl_tmp,
    .dumpit = NULL,
    .done = NULL,
},
};
 
static struct genl_multicast_group doc_exmpl_genl_mcgrp[] = {
{
    .name = "DOC_EXMPL_GRP",
},
};

/* family definition */
static struct genl_family doc_exmpl_genl_family = {
    .hdrsize = 0,             //自定义的头部长度，参考genl数据包结构
    .name = "DOC_EXMPL",      //这里定义family的名称，user program需要根据这个名字来找到对应的family ID。
    .version = 1,
    .maxattr = DOC_EXMPL_A_MAX,
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,13,0)
    .id = GENL_ID_GENERATE,   //这里不指定family ID，由内核进行分配
#else
    .id = 0,   //这里不指定family ID，由内核进行分配
    .ops = doc_exmpl_genl_ops_echo,
    .n_ops = (sizeof(doc_exmpl_genl_ops_echo)/sizeof(struct genl_ops)),
    .mcgrps = doc_exmpl_genl_mcgrp,
    .n_mcgrps = (sizeof(doc_exmpl_genl_mcgrp)/sizeof(struct genl_multicast_group)),
#endif
};
 
static inline int genl_msg_prepare_usr_msg(u8 cmd, size_t size, pid_t pid, struct sk_buff **skbp)
{
	struct sk_buff *skb;

	/* create a new netlink msg */
	skb = genlmsg_new(size, GFP_KERNEL);
	if (skb == NULL) {
		return -ENOMEM;
	}

	/* Add a new netlink message to an skb */
	genlmsg_put(skb, pid, 0, &doc_exmpl_genl_family, 0, cmd);

	*skbp = skb;
	return 0;
}
 
static inline int genl_msg_mk_usr_msg(struct sk_buff *skb, int type, void *data, int len)
{
	int rc;

	/* add a netlink attribute to a socket buffer */
	if ((rc = nla_put(skb, type, len, data)) != 0) {
		return rc;
	}
	return 0;
}
 
/**
* genl_msg_send_to_user - 通过generic netlink发送数据到netlink
*
* @data: 发送数据缓存
* @len:  数据长度 单位：byte
* @pid:  发送到的客户端pid
*
* return:
*    0:       成功
*    -1:      失败
*/
int genl_msg_send_to_user(void *data, int len, pid_t pid)
{
	struct sk_buff *skb;
	size_t size;
	void *head;
	int rc;
 
	size = nla_total_size(len); /* total length of attribute including padding */
 
	rc = genl_msg_prepare_usr_msg(DOC_EXMPL_C_ECHO, size, pid, &skb);
	if (rc) {
		return rc;
	}
 
	rc = genl_msg_mk_usr_msg(skb, DOC_EXMPL_A_MSG, data, len);
	if (rc) {
		kfree_skb(skb);
		return rc;
	}

	head = genlmsg_data(nlmsg_data(nlmsg_hdr(skb)));

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,0,0)
	rc = genlmsg_end(skb, head);
	if (rc < 0) {
		kfree_skb(skb);
		return rc;
	}
#else
	genlmsg_end(skb, head);
#endif

#ifdef NETLINK_UNICAST
	rc = genlmsg_unicast(&init_net, skb, pid);
	if (rc < 0) {
		return rc;
	}
#endif
#ifdef NETLINK_MULTICAST
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,13,0)
	/*
	static inline int genlmsg_multicast(struct sk_buff *skb, u32 portid,
					    unsigned int group, gfp_t flags)
	*/
	rc = genlmsg_multicast(skb, 0, GENL_GROUP, GFP_KERNEL);
#else
	/*
	static inline int genlmsg_multicast(const struct genl_family *family,
					    struct sk_buff *skb, u32 portid,
					    unsigned int group, gfp_t flags)
	*/
	rc = genlmsg_multicast(&doc_exmpl_genl_family, skb, 0, GENL_GROUP, GFP_KERNEL);
#endif
	if (rc < 0) {
                printk(KERN_ERR "Errno=%d\n", rc);
		return rc;
	}
#endif
	return 0;
}
 
//echo command handler, 命令处理函数，当接收到user program发出的命令后，这个函数会被内核调用
static int doc_exmpl_echo(struct sk_buff *skb, struct genl_info *info)
{
    /* message handling code goes here; return 0 on success, negative values on failure */
    struct nlmsghdr *nlhdr;
    struct genlmsghdr *genlhdr;
    struct nlattr *nlh;
    char *str;
    int ret;
 
    nlhdr = nlmsg_hdr(skb);
    genlhdr = nlmsg_data(nlhdr);
    nlh = genlmsg_data(genlhdr);
    str = nla_data(nlh);
    printk("%s get: %s\n", __FUNCTION__, str);
 
    ret = genl_msg_send_to_user(TEST_GENL_MSG_FROM_KERNEL,
            strlen(TEST_GENL_MSG_FROM_KERNEL) + 1,  nlhdr->nlmsg_pid);
 
    return ret;
}

static int doc_exmpl_tmp(struct sk_buff *skb, struct genl_info *info)
{
    /* message handling code goes here; return 0 on success, negative values on failure */
    struct nlmsghdr *nlhdr;
    struct genlmsghdr *genlhdr;
    struct nlattr *nlh;
    char *str;
    int ret;
 
    nlhdr = nlmsg_hdr(skb);
    genlhdr = nlmsg_data(nlhdr);
    nlh = genlmsg_data(genlhdr);
    str = nla_data(nlh);
    printk("%s get: %s\n", __FUNCTION__, str);
 
    ret = genl_msg_send_to_user(TEST_GENL_MSG_FROM_KERNEL,
            strlen(TEST_GENL_MSG_FROM_KERNEL) + 1,  nlhdr->nlmsg_pid);
 
    return ret;
}

static int __init genetlink_init(void)
{
    int rc;

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,13,0)
    printk("[%s] kernel version < 3.13\n", __FUNCTION__);
    /**
     * 1. Registering A Family
     * This function doesn't exist past linux 3.12
     */
    rc = genl_register_family(&doc_exmpl_genl_family);
    if (rc != 0)
        goto err_out1;
 
    rc = genl_register_ops(&doc_exmpl_genl_family, doc_exmpl_genl_ops_echo);
    if (rc != 0)
        goto err_out2;
 
    /*
     * for multicast
     */
    rc = genl_register_mc_group(&doc_exmpl_genl_family, doc_exmpl_genl_mcgrp);
    if (rc != 0)
        goto err_out3;
#elif LINUX_VERSION_CODE < KERNEL_VERSION(4,10,0)
    printk("[%s] kernel version < 4.10\n", __FUNCTION__);
    rc = genl_register_family_with_ops_groups(&doc_exmpl_genl_family, doc_exmpl_genl_ops_echo, doc_exmpl_genl_mcgrp);
    if (rc != 0)
        goto err_out3;
#else
    printk("[%s] kernel version >= 4.10\n", __FUNCTION__);
    rc = genl_register_family(&doc_exmpl_genl_family);
    if (rc != 0)
        goto err_out3;
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,13,0)
    printk("[%d]doc_exmpl_genl_family.id=%d\n", __LINE__, doc_exmpl_genl_family.id);
    printk("[%d]doc_exmpl_genl_mcgrp.id=%d\n", __LINE__, doc_exmpl_genl_mcgrp.id);
#else
    printk("[%d]doc_exmpl_genl_family.id=%d\n", __LINE__, doc_exmpl_genl_family.id);
#endif
    printk("genetlink_init OK\n");

    return rc; 
err_out3:
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,13,0)
    genl_unregister_ops(&doc_exmpl_genl_family, doc_exmpl_genl_ops_echo);
#endif
err_out2:
    genl_unregister_family(&doc_exmpl_genl_family);
err_out1:
    printk(KERN_ERR "Error occured while inserting generic netlink example module\n");
    return rc;
}
 
static void __exit genetlink_exit(void)
{
printk("[%s]=============\n", __FUNCTION__);
    printk("Generic Netlink Example Module unloaded.");
 
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,13,0)
    genl_unregister_mc_group(&doc_exmpl_genl_family, doc_exmpl_genl_mcgrp);
    genl_unregister_ops(&doc_exmpl_genl_family, doc_exmpl_genl_ops_echo);
#endif
    genl_unregister_family(&doc_exmpl_genl_family);
}

module_init(genetlink_init);
module_exit(genetlink_exit);
MODULE_LICENSE("GPL");
