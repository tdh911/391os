/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

/* Interrupt masks to determine which interrupts
 * are enabled and disabled */
uint8_t master_mask; /* IRQs 0-7 */
uint8_t slave_mask; /* IRQs 8-15 */

/* i8259_init
 *
 * DESCRIPTION: Initializes Master and Slave PICs with four ICWs. Then mask
 *              all IRQ inputs on master and slave PICs. Then enable the slave PIC
 *              onto a master pin.
 * INPUT/OUTPUT: none
 * SIDE EFFECTS: Enables Slave PIC on IRQ2 of Master
 */
void
i8259_init(void)
{
    // Initialize the masks
    master_mask = 0xFF;
    slave_mask = 0xFF;

    // send ICW1, ICW2, ICW3 to master
    outb(ICW1, MASTER_8259_PORT);
    outb(ICW2_MASTER, MASTER_8259_PORT_DATA);
    outb(ICW3_MASTER, MASTER_8259_PORT_DATA);

    // send ICW1, ICW2, ICW3 to slave
    outb(ICW1, SLAVE_8259_PORT);
    outb(ICW2_SLAVE, SLAVE_8259_PORT_DATA);
    outb(ICW3_SLAVE, SLAVE_8259_PORT_DATA);

    // send ICW4 to master and slave
    outb(ICW4, MASTER_8259_PORT_DATA);
    outb(ICW4, SLAVE_8259_PORT_DATA);

    outb(master_mask, MASTER_8259_PORT_DATA); // mask master PIC
    outb(slave_mask, SLAVE_8259_PORT_DATA); // mask slave PIC

    // enable the IRQ associated with the slave PIC
    enable_irq(SLAVE_IRQ_NUM);
}

/* enable_irq
 *
 * DESCRIPTION: Enables device IRQ on Master or Slave PIC
 * INPUT/OUTPUT: irq_num -- number of IRQ pin that needs to be enabled
 * SIDE EFFECTS: none
 */
void
enable_irq(uint32_t irq_num)
{
    // handle irq less than 8 - master
    if(irq_num < 8)
    {
        master_mask = inb(MASTER_8259_PORT_DATA) & ~(1 << irq_num);
        outb(master_mask, MASTER_8259_PORT_DATA);
    }
    // handle irq greater than 8 - slave
    else
    {
        irq_num -= 8;
        slave_mask = inb(SLAVE_8259_PORT_DATA) & ~(1 << irq_num);
        outb(slave_mask, SLAVE_8259_PORT_DATA);
    }
}

/* disable_irq
 *
 * DESCRIPTION: Disables device IRQ on Master or Slave PIC
 * INPUT/OUTPUT: irq_num -- number of IRQ pin that needs to be disabled
 * SIDE EFFECTS: none
 */
void
disable_irq(uint32_t irq_num)
{
    // handle irq less than 8 - master
    if(irq_num < 8)
    {
        master_mask = inb(MASTER_8259_PORT_DATA) | (1 << irq_num);
        outb(master_mask, MASTER_8259_PORT_DATA);
    }
    // handle irq greater than 8 - slave
    else
    {
        irq_num -= 8;
        slave_mask = inb(SLAVE_8259_PORT_DATA) | (1 << irq_num);
        outb(slave_mask, SLAVE_8259_PORT_DATA);
    }
}

/* send_eoi
 *
 * DESCRIPTION: Sends EOI signal to Master and/or Slave PIC
 * INPUT/OUTPUT: irq_num -- number of IRQ pin that's interrupt has finished
 * SIDE EFFECTS: Computer can now receive new interrupt from PIC
 */
void
send_eoi(uint32_t irq_num)
{
    uint8_t PIC_EOI;

    // handle irq less than 8 - master
    if(irq_num < 8)
    {
        PIC_EOI = EOI | irq_num;
        outb(PIC_EOI, MASTER_8259_PORT);
    }
    // handle irq greater than 8 - slave
    else
    {
        irq_num -= 8;
        PIC_EOI = EOI | irq_num;
        outb(EOI | SLAVE_IRQ_NUM, MASTER_8259_PORT);
        outb(PIC_EOI, SLAVE_8259_PORT);
    }
}
