#include <linux/cpufreq.h>
#include <linux/kallsyms.h>

#define DRIVER_AUTHOR "Czar Dixon <rbheromax@gmail.com>"
#define DRIVER_DESCRIPTION "SGSVictory Overclock Driver"
#define DRIVER_VERSION "1.2"

MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESCRIPTION);
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE("GPL");

/* Speed of the HFPLL in KHz */
#define HFPLL_FREQ_KHZ 27000

/* Name of the acpu_freq_tbl symbols */
#define ACPU_FREQ_TBL_NOM_NAME "acpu_freq_tbl_8960_kraitv2_nom"
#define ACPU_FREQ_TBL_SLOW_NAME "acpu_freq_tbl_8960_kraitv2_slow"
#define ACPU_FREQ_TBL_FAST_NAME "acpu_freq_tbl_8960_kraitv2_fast"

/* Module parameters */
/* PLL L value. Controls overclocked frequency. 27 MHz * pll_l_val = X MHz */
static uint pll_l_val = 0x45;

/* Voltage of the overclocked frequency, in uV. Max supported is 1300000 uV */
static uint vdd_uv = 1508000;

module_param(pll_l_val, uint, 0444);
module_param(vdd_uv, uint, 0444);
MODULE_PARM_DESC(pll_l_val, "Frequency multiplier for overclocked frequency");
MODULE_PARM_DESC(vdd_uv, "Core voltage in uV for overclocked frequency");

/* New frequency table */
static struct cpufreq_frequency_table freq_table[] = {
  { 0, 384000 },
	{ 1, 486000 },
	{ 2, 594000 },
	{ 3, 702000 },
	{ 4, 810000 },
	{ 5, 918000 },
	{ 6, 1026000 },
	{ 7, 1134000 },
	{ 8, 1242000 },
	{ 9, 1350000 },
	{ 10, 1458000 },
	{ 11, 1512000 },
	/* We replace this row with our desired frequency later */
	{ 12, 2538000 },
	{ 13, CPUFREQ_TABLE_END },
};

struct core_speed {
	unsigned int		khz;
	int			src;
	unsigned int		pri_src_sel;
	unsigned int		sec_src_sel;
	unsigned int		pll_l_val;
};

struct acpu_level {
	unsigned int		use_for_scaling;
	struct core_speed	speed;
	struct l2_level		*l2_level;
	unsigned int		vdd_core;
};

struct cpufreq_frequency_table *orig_table;

/* Use a function pointer for cpufreq_cpu_get because the symbol version
 * differs in the HTC kernel and the Code Aurora kernel, so the kernel won't
 * let us call it normally.
 *
 * Call the function normally when the kernel source is released.
 */
typedef struct cpufreq_policy *(*cpufreq_cpu_get_type)(int); 
struct cpufreq_policy *(*cpufreq_cpu_get_new)(int);

/* Updates a row in a struct acpu_level with symbol name symbol_name */
static void __init acpu_freq_row_update
	(char *symbol_name, uint index, uint l_val, uint vdd)
{
	struct acpu_level *acpu_freq_tbl;
	ulong acpu_freq_tbl_addr;

	acpu_freq_tbl_addr = kallsyms_lookup_name(symbol_name);

	if(acpu_freq_tbl_addr == 0) {
		printk(KERN_WARNING "krait_oc: symbol not found\n");
		printk(KERN_WARNING "krait_oc: skipping this table\n");
		return;
	}

	acpu_freq_tbl = (struct acpu_level*) acpu_freq_tbl_addr;
	acpu_freq_tbl[index].speed.khz = l_val*HFPLL_FREQ_KHZ;
	acpu_freq_tbl[index].speed.pll_l_val = l_val;
	acpu_freq_tbl[index].vdd_core = vdd;
}

static int __init overclock_init(void)
{
	struct cpufreq_policy *policy;
	ulong cpufreq_cpu_get_addr;
	uint cpu;

	printk(KERN_INFO "krait_oc: %s version %s\n", DRIVER_DESCRIPTION,
		DRIVER_VERSION);
	printk(KERN_INFO "krait_oc: by %s\n", DRIVER_AUTHOR);
	printk(KERN_INFO "krait_oc: overclocking to %u at %u uV\n",
		pll_l_val*HFPLL_FREQ_KHZ, vdd_uv);

	printk(KERN_INFO "krait_oc: updating cpufreq policy\n");

	cpufreq_cpu_get_addr = kallsyms_lookup_name("cpufreq_cpu_get");

	if(cpufreq_cpu_get_addr == 0) {
		printk(KERN_WARNING "krait_oc: symbol not found\n");
		printk(KERN_WARNING "krait_oc: not attempting overclock\n");
		return 0;
	}

	cpufreq_cpu_get_new = (cpufreq_cpu_get_type) cpufreq_cpu_get_addr;

	policy = cpufreq_cpu_get_new(0);
	policy->cpuinfo.max_freq = pll_l_val*HFPLL_FREQ_KHZ;

	printk(KERN_INFO "krait_oc: updating cpufreq tables\n");
	freq_table[12].frequency = pll_l_val*HFPLL_FREQ_KHZ;

	/* Save a pointer to the freq original table to restore if unloaded */
	orig_table = cpufreq_frequency_get_table(0);

	for_each_possible_cpu(cpu) {
		cpufreq_frequency_table_put_attr(cpu);
		cpufreq_frequency_table_get_attr(freq_table, cpu);
	}

	/* Index 20 is not used for scaling in the acpu_freq_tbl, so fill it
         * with our new freq. Change all three tables to account for all
	 * possible bins. */
	printk(KERN_INFO "krait_oc: updating nominal acpu_freq_tbl\n");
	acpu_freq_row_update(ACPU_FREQ_TBL_NOM_NAME, 20, pll_l_val, vdd_uv);
	printk(KERN_INFO "krait_oc: updating slow acpu_freq_tbl\n");
	acpu_freq_row_update(ACPU_FREQ_TBL_SLOW_NAME, 20, pll_l_val, vdd_uv);
	printk(KERN_INFO "krait_oc: updating fast acpu_freq_tbl\n");
	acpu_freq_row_update(ACPU_FREQ_TBL_FAST_NAME, 20, pll_l_val, vdd_uv);

	return 0;
}

static void __exit overclock_exit(void)
{
	struct cpufreq_policy *policy;
	uint cpu;

	if(kallsyms_lookup_name("cpufreq_cpu_get") != 0) {
		printk(KERN_INFO "krait_oc: reverting cpufreq policy\n");
		policy = cpufreq_cpu_get_new(0);
		policy->cpuinfo.max_freq = 1512000;

		printk(KERN_INFO "krait_oc: reverting cpufreq tables\n");
		for_each_possible_cpu(cpu) {
			cpufreq_frequency_table_put_attr(cpu);
			cpufreq_frequency_table_get_attr(orig_table, cpu);
		}
	}

	printk(KERN_INFO "krait_oc: unloaded\n");
}

module_init(overclock_init);
module_exit(overclock_exit);
