ENTRY(_start)

SECTIONS
{
    . = 0x100000000;
	.text : {
		*(.text)
	}

	.rodata : {
		*(.rodata)
	}

	.stab : {
		*(.stab);
	}

	.stabstr : {
		*(.stabstr);
	}

	.data : {
		*(.data)
	}

	.bss : {
		*(.bss)
	}
}
