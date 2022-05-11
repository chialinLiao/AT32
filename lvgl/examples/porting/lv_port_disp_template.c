/**
 * @file lv_port_disp_templ.c
 *
 */

 /*Copy this file as "lv_port_disp.c" and set this value to "1" to enable content*/
#if 1

/*********************
 *      INCLUDES
 *********************/
#include "lv_port_disp_template.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
//lv_disp_drv_t disp_drv;

static void disp_init(void);
static void disp_flush(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p);

#if LV_USE_GPU
static void gpu_blend(lv_color_t * dest, const lv_color_t * src, uint32_t length, lv_opa_t opa);
static void gpu_fill(lv_color_t * dest, uint32_t length, lv_color_t color);
#endif

/**********************
 *  STATIC VARIABLES
 **********************/
static lv_disp_drv_t* lv_disp_drv_p = NULL;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
void DMA1_Channel1_IRQHandler()
{
    dma_flag_clear(DMA1_FDT1_FLAG);
    dma_channel_enable(DMA1_CHANNEL1, FALSE);

    if(lv_disp_drv_p != NULL)
    {
        /* tell lvgl that flushing is done */
        lv_disp_flush_ready(lv_disp_drv_p);
    }
}

void lv_port_disp_init(void)
{
    //* initialize your display

    disp_init();
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/* Initialize your display and the required peripherals. */
static void disp_init(void)
{
    dma_init_type dma_init_struct;
    crm_periph_clock_enable(CRM_DMA1_PERIPH_CLOCK, TRUE);
    dma_reset(DMA1_CHANNEL1);

    dma_init_struct.buffer_size             = 65535;
    dma_init_struct.direction               = DMA_DIR_MEMORY_TO_MEMORY;
    dma_init_struct.memory_base_addr        = (uint32_t)XMC_LCD_DATA; /* as  destination address */
    dma_init_struct.memory_data_width       = DMA_MEMORY_DATA_WIDTH_WORD;
    dma_init_struct.memory_inc_enable       = FALSE;
    dma_init_struct.peripheral_base_addr    = (uint32_t)XMC_LCD_DATA; /* as  source address */
    dma_init_struct.peripheral_data_width   = DMA_PERIPHERAL_DATA_WIDTH_WORD;
    dma_init_struct.peripheral_inc_enable   = TRUE;
    dma_init_struct.priority                = DMA_PRIORITY_MEDIUM;
    dma_init_struct.loop_mode_enable        = FALSE;
    dma_init(DMA1_CHANNEL1, &dma_init_struct);

    nvic_irq_enable(DMA1_Channel1_IRQn, 1, 0);
    dma_interrupt_enable(DMA1_CHANNEL1,DMA_FDT_INT,TRUE);

//  lcd_init();

	static lv_disp_buf_t disp_buf;

#if 0  /* dul buffer */
	static lv_color_t buf1[LV_HOR_RES_MAX * 10];                      /*A buffer for 10 rows*/
	static lv_color_t buf2[LV_HOR_RES_MAX * 10];                      /*An other buffer for 10 rows*/
	lv_disp_buf_init(&disp_buf, buf1, buf2, LV_HOR_RES_MAX * 10);     /*Initialize the display buffer*/
#else /* single buffer */
	static lv_color_t buf1[LV_HOR_RES_MAX * 10];
	lv_disp_buf_init(&disp_buf, buf1, NULL, LV_HOR_RES_MAX * 10);   /*Initialize the display buffer*/
#endif

	lv_disp_drv_t disp_drv;                  /*Descriptor of a display driver*/
	lv_disp_drv_init(&disp_drv);             /*Basic initialization*/

	/*Set up the functions to access to your display*/

	/*Set the resolution of the display*/
	disp_drv.hor_res = LV_HOR_RES_MAX;
	disp_drv.ver_res = LV_VER_RES_MAX;

	/*Used to copy the buffer's content to the display*/
	disp_drv.flush_cb = disp_flush;

	/*Set a display buffer*/
	disp_drv.buffer = &disp_buf;
	lv_disp_drv_register(&disp_drv);
}


/* Flush the content of the internal buffer the specific area on the display
 * You can use DMA or any hardware acceleration to do this operation in the background but
 * 'lv_disp_flush_ready()' has to be called when finished. */
static void disp_flush(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p)
{
    lv_disp_drv_p = disp_drv;

    uint32_t size = (area->x2 - area->x1 + 1) * (area->y2 - area->y1 + 1);
	lcd_setblock(area->x1, area->y1, area->x2, area->y2);

    DMA1_CHANNEL1->ctrl    &= ~(uint16_t)1;
    DMA1_CHANNEL1->paddr  = (uint32_t)color_p;
    DMA1_CHANNEL1->dtcnt = size / 2;
    DMA1_CHANNEL1->ctrl    |= (uint16_t)1;

    /* IMPORTANT!!!
    * Inform the graphics library that you are ready with the flushing*/
    // lv_disp_flush_ready(disp_drv);
}

/*OPTIONAL: GPU INTERFACE*/
#if LV_USE_GPU

/* If your MCU has hardware accelerator (GPU) then you can use it to blend to memories using opacity
 * It can be used only in buffered mode (LV_VDB_SIZE != 0 in lv_conf.h)*/
static void gpu_blend(lv_disp_drv_t * disp_drv, lv_color_t * dest, const lv_color_t * src, uint32_t length, lv_opa_t opa)
{
    /*It's an example code which should be done by your GPU*/
    uint32_t i;
    for(i = 0; i < length; i++) {
        dest[i] = lv_color_mix(dest[i], src[i], opa);
    }
}

/* If your MCU has hardware accelerator (GPU) then you can use it to fill a memory with a color
 * It can be used only in buffered mode (LV_VDB_SIZE != 0 in lv_conf.h)*/
static void gpu_fill_cb(lv_disp_drv_t * disp_drv, lv_color_t * dest_buf, lv_coord_t dest_width,
                    const lv_area_t * fill_area, lv_color_t color);
{
    /*It's an example code which should be done by your GPU*/
    uint32_t x, y;
    dest_buf += dest_width * fill_area->y1; /*Go to the first line*/

    for(y = fill_area->y1; y < fill_area->y2; y++) {
        for(x = fill_area->x1; x < fill_area->x2; x++) {
            dest_buf[x] = color;
        }
        dest_buf+=dest_width;    /*Go to the next line*/
    }
}

#endif  /*LV_USE_GPU*/

#else /* Enable this file at the top */

/* This dummy typedef exists purely to silence -Wpedantic. */
typedef int keep_pedantic_happy;


#endif
