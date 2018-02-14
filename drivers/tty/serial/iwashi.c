// SPDX-License-Identifier: GPL-2.0
/*
 * uartlite.c: Serial driver for Xilinx uartlite serial controller
 *
 * Copyright (C) 2006 Peter Korsgaard <jacmet@sunsite.dk>
 * Copyright (C) 2007 Secret Lab Technologies Ltd.
 */

#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/console.h>
#include <linux/serial.h>
#include <linux/serial_core.h>
#include <linux/tty.h>
#include <linux/tty_flip.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/of_platform.h>

#define IWASHI_NAME		"ttyIW"
#define IWASHI_MAJOR		204
#define IWASHI_MINOR		187
#define IWASHI_NR_UARTS		500

static struct uart_port iwashi_ports[IWASHI_NR_UARTS];

/* ---------------------------------------------------------------------
 * Register definitions
 *
 * For register details see datasheet:
 * http://www.xilinx.com/support/documentation/ip_documentation/opb_uartlite.pdf
 */

static void iwashi_console_write(struct console *co, const char *s,
        unsigned int count)
{
    unsigned int c_tmp;
    for(c_tmp = count; c_tmp > 0; c_tmp--) {
        char c = *s;
        __asm__ __volatile__(
                "sw x1, -4(sp)\n\t"
                "lw x1, %0\n\t"
                ".word 0x0000802B\n\t"
                "lw x1, -4(sp)\n\t"
                : : "A"(c)
                );
        s++;
    }
}
static int iwashi_console_setup(struct console *co, char *options)
{
    struct uart_port *port;
    int baud = 115200;
    int bits = 8;
    int parity = 'n';
    int flow = 'n';

    if (co->index < 0 || co->index >= IWASHI_NR_UARTS)
        return -EINVAL;

    port = &iwashi_ports[co->index];

    /* Has the device been initialized yet? */
    if (!port->mapbase) {
        pr_debug("console on ttyUL%i not present\n", co->index);
        return -ENODEV;
    }

    /* not initialized yet? */
    /*
       if (!port->membase) {
       if (ulite_request_port(port))
       return -ENODEV;
       }
       */

    if (options)
        uart_parse_options(options, &baud, &parity, &bits, &flow);

    return uart_set_options(port, co, baud, parity, bits, flow);

}

static struct uart_driver iwashi_uart_driver;

static struct console iwashi_console = {
    .name	= IWASHI_NAME,
    .write	= iwashi_console_write,
    .device	= uart_console_device,
    .setup	= iwashi_console_setup,
    .flags	= CON_PRINTBUFFER | CON_ENABLED,
    .index	= -1, /* Specified on the cmdline (e.g. console=ttyUL0 ) */
    .data	= &iwashi_uart_driver,
};

static int __init iwashi_console_init(void)
{
    register_console(&iwashi_console);
    return 0;
}

console_initcall(iwashi_console_init);
