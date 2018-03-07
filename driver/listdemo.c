/*************************************************************************
	> File Name: listdemo.c
	> Author: shuixianbing
	> Mail: shui6666@126.com 
	> Created Time: 2014年10月16日 星期四 16时29分24秒
 ************************************************************************/
#include <linux/init.h>
#include <linux/module.h>
#include <linux/list.h>
#include <linux/slab.h>

MODULE_LICENSE("GPL");

#define EMPLOYEE_NUMBEROF 5
static struct list_head employee_list;
struct employee{
	char name[20];
	int number;
	int salary;
	struct list_head list;
};

static struct employee *employeep = NULL;
static struct employee *employeep_tmp = NULL;
static struct list_head *pos = NULL;

static int __init listdemo_init(void){
	int i;
	INIT_LIST_HEAD(&employee_list);

	employeep = kmalloc(sizeof(struct employee)*EMPLOYEE_NUMBEROF,
			GFP_KERNEL);

	memset(employeep, 0, sizeof(struct employee)*EMPLOYEE_NUMBEROF);

	//fill data
	for(i=0; i<EMPLOYEE_NUMBEROF;i++){
		sprintf(employeep[i].name , "Employee%d", i+1);
		employeep[i].number = 1001 + i;
		employeep[i].salary = 5000 + 5000*i;
		list_add(&(employeep[i].list), &employee_list);
	}

	list_for_each(pos, &employee_list){
		employeep_tmp = list_entry(pos, struct employee, list);
		printk("Employee Name %s\t Number:%d\t Salary: %d\n",
				employeep_tmp->name, employeep_tmp->number, employeep_tmp->salary);
	}

	return 0;
}

static void __exit listdemo_exit(void){
	int i;
	for(i=0; i<EMPLOYEE_NUMBEROF; i++){
		list_del(&(employeep[i].list));
	}
	kfree(employeep);
}
module_init(listdemo_init);
module_exit(listdemo_exit);
