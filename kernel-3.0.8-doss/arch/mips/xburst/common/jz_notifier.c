#include <jz_notifier.h>

static BLOCKING_NOTIFIER_HEAD(jz_notifier_chain_high);
static BLOCKING_NOTIFIER_HEAD(jz_notifier_chain_normal);
static BLOCKING_NOTIFIER_HEAD(jz_notifier_chain_low);
static int jz_notifier(struct notifier_block *nb, unsigned long cmd, void *data)
{
	struct jz_notifier *jz_nb = container_of(nb,struct jz_notifier,nb);
	int ret = 0;
	if(jz_nb->msg == cmd) {
		ret = jz_nb->jz_notify(jz_nb,data);
	}
	return ret;
}

int jz_notifier_register(struct jz_notifier *notify, unsigned int priority)
{
	unsigned int ret = 0;

	if((notify->level < NOTEFY_PROI_START) && (notify->level >= NOTEFY_PROI_END))
	{
		printk("notify level can not support this %d\n",notify->level);
		dump_stack();
		return -1;
	}
	if((int)notify->msg >= JZ_CMD_END && (int)notify->msg <= JZ_CMD_START)
	{
		printk("notify msg can not support this %d\n",notify->msg);
		dump_stack();
		return -1;
	}
	if(notify->jz_notify == NULL)
	{
		printk("notify function(jz_notify) cand not support NULL\n");
		dump_stack();
		return -1;
	}
	notify->nb.priority = notify->level;
	notify->nb.notifier_call = jz_notifier;

	if(priority == NOTEFY_PROI_HIGH)
		ret = blocking_notifier_chain_register(&jz_notifier_chain_high, &notify->nb);
	else if(priority == NOTEFY_PROI_NORMAL)
		ret = blocking_notifier_chain_register(&jz_notifier_chain_normal, &notify->nb);
	else if(priority == NOTEFY_PROI_LOW)
		ret = blocking_notifier_chain_register(&jz_notifier_chain_low, &notify->nb);
	else
		printk("not support\n");
	return ret;
}

int jz_notifier_unregister(struct jz_notifier *notify, unsigned int priority)
{
	unsigned int ret = 0;

	if(priority == NOTEFY_PROI_HIGH)
		ret = blocking_notifier_chain_unregister(&jz_notifier_chain_high, &notify->nb);
	else if(priority == NOTEFY_PROI_NORMAL)
		ret = blocking_notifier_chain_unregister(&jz_notifier_chain_normal, &notify->nb);
	else if(priority == NOTEFY_PROI_LOW)
		ret = blocking_notifier_chain_unregister(&jz_notifier_chain_low, &notify->nb);
	else
		printk("not support\n");
	return ret;
}
int jz_notifier_call(unsigned int priority, unsigned long val, void *v)
{
	unsigned int ret = 0;

	if(priority == NOTEFY_PROI_HIGH)
		ret = blocking_notifier_call_chain(&jz_notifier_chain_high, val, v);
	else if(priority == NOTEFY_PROI_NORMAL)
		ret = blocking_notifier_call_chain(&jz_notifier_chain_normal, val, v);
	else if(priority == NOTEFY_PROI_LOW)
		ret = blocking_notifier_call_chain(&jz_notifier_chain_low, val, v);
	else
		printk("not support\n");
	return ret;
}
