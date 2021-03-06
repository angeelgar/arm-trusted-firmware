/*
 * Copyright (c) 2013-2016, ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#include <assert.h>
#include <bl_common.h>
#include <console.h>
#include <debug.h>
#include <generic_delay_timer.h>
#include <mcucfg.h>
#include <mmio.h>
#include <mtcmos.h>
#include <plat_arm.h>
#include <plat_private.h>
#include <platform.h>
#include <spm.h>

/*******************************************************************************
 * Declarations of linker defined symbols which will help us find the layout
 * of trusted SRAM
 ******************************************************************************/
unsigned long __RO_START__;
unsigned long __RO_END__;

unsigned long __COHERENT_RAM_START__;
unsigned long __COHERENT_RAM_END__;

/*
 * The next 3 constants identify the extents of the code, RO data region and the
 * limit of the BL31 image.  These addresses are used by the MMU setup code and
 * therefore they must be page-aligned.  It is the responsibility of the linker
 * script to ensure that __RO_START__, __RO_END__ & __BL31_END__ linker symbols
 * refer to page-aligned addresses.
 */
#define BL31_RO_BASE (unsigned long)(&__RO_START__)
#define BL31_RO_LIMIT (unsigned long)(&__RO_END__)
#define BL31_END (unsigned long)(&__BL31_END__)

/*
 * The next 2 constants identify the extents of the coherent memory region.
 * These addresses are used by the MMU setup code and therefore they must be
 * page-aligned.  It is the responsibility of the linker script to ensure that
 * __COHERENT_RAM_START__ and __COHERENT_RAM_END__ linker symbols
 * refer to page-aligned addresses.
 */
#define BL31_COHERENT_RAM_BASE (unsigned long)(&__COHERENT_RAM_START__)
#define BL31_COHERENT_RAM_LIMIT (unsigned long)(&__COHERENT_RAM_END__)

static entry_point_info_t bl32_ep_info;
static entry_point_info_t bl33_ep_info;

static void platform_setup_cpu(void)
{
	/* turn off all the little core's power except cpu 0 */
	mtcmos_little_cpu_off();

	/* setup big cores */
	mmio_write_32((uintptr_t)&mt8173_mcucfg->mp1_config_res,
		MP1_DIS_RGU0_WAIT_PD_CPUS_L1_ACK |
		MP1_DIS_RGU1_WAIT_PD_CPUS_L1_ACK |
		MP1_DIS_RGU2_WAIT_PD_CPUS_L1_ACK |
		MP1_DIS_RGU3_WAIT_PD_CPUS_L1_ACK |
		MP1_DIS_RGU_NOCPU_WAIT_PD_CPUS_L1_ACK);
	mmio_setbits_32((uintptr_t)&mt8173_mcucfg->mp1_miscdbg, MP1_AINACTS);
	mmio_setbits_32((uintptr_t)&mt8173_mcucfg->mp1_clkenm_div,
		MP1_SW_CG_GEN);
	mmio_clrbits_32((uintptr_t)&mt8173_mcucfg->mp1_rst_ctl,
		MP1_L2RSTDISABLE);

	/* set big cores arm64 boot mode */
	mmio_setbits_32((uintptr_t)&mt8173_mcucfg->mp1_cpucfg,
		MP1_CPUCFG_64BIT);

	/* set LITTLE cores arm64 boot mode */
	mmio_setbits_32((uintptr_t)&mt8173_mcucfg->mp0_rv_addr[0].rv_addr_hw,
		MP0_CPUCFG_64BIT);

	/* enable dcm control */
	mmio_setbits_32((uintptr_t)&mt8173_mcucfg->bus_fabric_dcm_ctrl,
		ADB400_GRP_DCM_EN | CCI400_GRP_DCM_EN | ADBCLK_GRP_DCM_EN |
		EMICLK_GRP_DCM_EN | ACLK_GRP_DCM_EN | L2C_IDLE_DCM_EN |
		INFRACLK_PSYS_DYNAMIC_CG_EN);
	mmio_setbits_32((uintptr_t)&mt8173_mcucfg->l2c_sram_ctrl,
		L2C_SRAM_DCM_EN);
	mmio_setbits_32((uintptr_t)&mt8173_mcucfg->cci_clk_ctrl,
		MCU_BUS_DCM_EN);
}

static void platform_setup_sram(void)
{
	/* protect BL31 memory from non-secure read/write access */
	mmio_write_32(SRAMROM_SEC_ADDR, (uint32_t)(BL31_END + 0x3ff) & 0x3fc00);
	mmio_write_32(SRAMROM_SEC_CTRL, 0x10000ff9);
}

/*******************************************************************************
 * Return a pointer to the 'entry_point_info' structure of the next image for
 * the security state specified. BL33 corresponds to the non-secure image type
 * while BL32 corresponds to the secure image type. A NULL pointer is returned
 * if the image does not exist.
 ******************************************************************************/
entry_point_info_t *bl31_plat_get_next_image_ep_info(uint32_t type)
{
	entry_point_info_t *next_image_info;

	next_image_info = (type == NON_SECURE) ? &bl33_ep_info : &bl32_ep_info;

	/* None of the images on this platform can have 0x0 as the entrypoint */
	if (next_image_info->pc)
		return next_image_info;
	else
		return NULL;
}

extern uint64_t get_kernel_info_pc(void);
extern uint64_t get_kernel_info_r0(void);
extern uint64_t get_kernel_info_r1(void);
entry_point_info_t *bl31_plat_get_next_kernel_ep_info(uint32_t type)
{
	entry_point_info_t *next_image_info;

	assert(sec_state_is_valid(type));

	next_image_info = (type == NON_SECURE) ? &bl33_ep_info : &bl32_ep_info;

	next_image_info->spsr = plat_get_spsr_for_kernel_entry();
	next_image_info->pc = get_kernel_info_pc();
	next_image_info->args.arg0 = get_kernel_info_r0();
	next_image_info->args.arg1 = get_kernel_info_r1();

	INFO("pc=0x%lx, r0=0x%lx, r1=0x%lx\n",
		(unsigned long)next_image_info->pc,
		next_image_info->args.arg0,
		next_image_info->args.arg1);

	SET_SECURITY_STATE(next_image_info->h.attr, NON_SECURE);

	/* None of the images on this platform can have 0x0 as the entrypoint */
	if (next_image_info->pc)
		return next_image_info;
	else
		return NULL;
}

/*******************************************************************************
 * Perform any BL3-1 early platform setup. Here is an opportunity to copy
 * parameters passed by the calling EL (S-EL1 in BL2 & S-EL3 in BL1) before they
 * are lost (potentially). This needs to be done before the MMU is initialized
 * so that the memory layout can be used while creating page tables.
 * BL2 has flushed this information to memory, so we are guaranteed to pick up
 * good data.
 ******************************************************************************/
void bl31_early_platform_setup(bl31_params_t *from_bl2,
			       void *plat_params_from_bl2)
{
	atf_arg_t_ptr teearg = (atf_arg_t_ptr)(uintptr_t)TEE_BOOT_INFO_ADDR;

	console_init(MT8173_UART0_BASE, MT8173_UART_CLOCK, MT8173_BAUDRATE);

	/* Initialize the console to provide early debug support */
	INFO("LK boot argument location=0x%x\n", BOOT_ARGUMENT_LOCATION);
	INFO("LK boot argument size=0x%x\n", BOOT_ARGUMENT_SIZE);
	INFO("teearg->atf_magic=0x%x\n", teearg->atf_magic);
	INFO("teearg->tee_support=0x%x\n", teearg->tee_support);
	INFO("teearg->tee_entry=0x%x\n", teearg->tee_entry);
	INFO("teearg->tee_boot_arg_addr=0x%x\n", teearg->tee_boot_arg_addr);
	INFO("teearg->atf_log_port=0x%x\n", teearg->atf_log_port);
	INFO("teearg->atf_log_baudrate=0x%x\n", teearg->atf_log_baudrate);
	INFO("teearg->atf_log_buf_start=0x%x\n", teearg->atf_log_buf_start);
	INFO("teearg->atf_log_buf_size=0x%x\n", teearg->atf_log_buf_size);
	INFO("teearg->atf_irq_num=%d\n", teearg->atf_irq_num);
	INFO("BL33_START_ADDRESS=0x%x\n", BL33_START_ADDRESS);

	VERBOSE("bl31_setup\n");
#if RESET_TO_BL31
	assert(from_bl2 == NULL);
	assert(plat_params_from_bl2 == NULL);
	INFO("RESET_TO_BL31!\n");

	/* Populate entry point information for BL3-2 and BL3-3 */
	SET_PARAM_HEAD(&bl32_ep_info, PARAM_EP, VERSION_1, 0);
	SET_SECURITY_STATE(bl32_ep_info.h.attr, SECURE);
	if (teearg->tee_support)
		bl32_ep_info.pc = teearg->tee_entry;
	else
		bl32_ep_info.pc = 0; /* let it die if it runs to BL32 */
	bl32_ep_info.spsr = plat_get_spsr_for_bl32_entry();

	SET_PARAM_HEAD(&bl33_ep_info, PARAM_EP, VERSION_1, 0);
	/*
	* Tell BL3-1 where the non-trusted software image
	* is located and the entry state information
	*/
	bl33_ep_info.pc = BL33_START_ADDRESS;
	bl33_ep_info.spsr = plat_get_spsr_for_bl33_entry();
	bl33_ep_info.args.arg4 = (unsigned long)BOOT_ARGUMENT_LOCATION;
	bl33_ep_info.args.arg5 = (unsigned long)BOOT_ARGUMENT_SIZE;
	SET_SECURITY_STATE(bl33_ep_info.h.attr, NON_SECURE);
#else
	assert(from_bl2 != NULL);
	assert(from_bl2->h.type == PARAM_BL31);
	assert(from_bl2->h.version >= VERSION_1);

	bl32_ep_info = *from_bl2->bl32_ep_info;
	bl33_ep_info = *from_bl2->bl33_ep_info;
#endif
}

/*******************************************************************************
 * Perform any BL3-1 platform setup code
 ******************************************************************************/
void bl31_platform_setup(void)
{
	platform_setup_cpu();
	platform_setup_sram();

	/* return sram to ca53 l2 cache */
	mmio_write_32(0x10000000, 0x300);
	/* turn off l2c sram clock */
	mmio_write_32(0x10001040, 1 << 7);

	generic_delay_timer_init();

	/* Initialize the gic cpu and distributor interfaces */
	plat_arm_gic_driver_init();
	plat_arm_gic_init();

	/* Initialize spm at boot time */
	spm_boot_init();
}

/*******************************************************************************
 * Perform the very early platform specific architectural setup here. At the
 * moment this is only intializes the mmu in a quick and dirty way.
 ******************************************************************************/
void bl31_plat_arch_setup(void)
{
	plat_cci_init();
	plat_cci_enable();

	plat_configure_mmu_el3(BL31_RO_BASE,
			       (BL31_COHERENT_RAM_LIMIT - BL31_RO_BASE),
			       BL31_RO_BASE,
			       BL31_RO_LIMIT,
			       BL31_COHERENT_RAM_BASE,
			       BL31_COHERENT_RAM_LIMIT);
}

void bl31_plat_runtime_setup(void)
{
}
