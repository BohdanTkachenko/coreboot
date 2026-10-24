/* SPDX-License-Identifier: GPL-2.0-or-later */

/* Included in Scope (\_SB) by both USB4 and Type-A-only TCSS descriptions. */
/*
 * Define PCH ACPIBASE IO as an ACPI operating region. The base address can be
 * found in Device 31, Function 2, Offset 40h.
 */
OperationRegion (PMIO, SystemIO, ACPI_BASE_ADDRESS, 0x80)
Field (PMIO, ByteAcc, NoLock, Preserve) {
	Offset(0x6C),   /* 0x6C, General Purpose Event 0 Status [127:96] */
	    ,  19,
	CPWS,  1,       /* CPU WAKE STATUS */
	Offset(0x7C),   /* 0x7C, General Purpose Event 0 Enable [127:96] */
	    ,  19,
	CPWE,  1        /* CPU WAKE EN */
}

Name (C2PW, 0)  /* Set default value to 0. */

/*
 * C2PM (CPU to PCH Method)
 *
 * This object is Enable/Disable GPE_CPU_WAKE_EN.
 * Arguments: (4)
 * Arg0 - An Integer containing the device wake capability
 * Arg1 - An Integer containing the target system state
 * Arg2 - An Integer containing the target device state
 * Arg3 - An Integer containing the request device type
 * Return Value:
 * return 0
 */
Method (C2PM, 4, NotSerialized)
{
	Local0 = 1 << Arg3
	/* This method is used to enable/disable wake from Tcss Device (WKEN). */
	If (Arg0 && Arg1)
	{  /* If entering Sx and enabling wake, need to enable WAKE capability. */
		If (CPWE == 0) {  /* If CPU WAKE EN is not set, Set it. */
			If (CPWS) {  /* If CPU WAKE STATUS is set, Clear it. */
				/* Clear CPU WAKE STATUS by writing 1. */
				CPWS = 1
			}
			CPWE = 1  /* Set CPU WAKE EN by writing 1. */
		}
		If ((C2PW & Local0) == 0) {
			/* Set Corresponding Device En BIT in C2PW. */
			C2PW |= Local0
		}
	} Else {  /* If Staying in S0 or Disabling Wake. */
		If (Arg0 || Arg2) {  /* Check if Exiting D0 and arming for wake. */
			/* If CPU WAKE EN is not set, Set it. */
			If (CPWE == 0) {
				/* If CPU WAKE STATUS is set, Clear it. */
				If (CPWS) {
					/* Clear CPU WAKE STATUS by writing 1. */
					CPWS = 1
				}
				CPWE = 1  /* Set CPU WAKE EN by writing 1. */
			}
			If ((C2PW & Local0) == 0) {
				/* Set Corresponding Device En BIT in C2PW. */
				C2PW |= Local0
			}
		} Else {
			/*
			 * Disable runtime PME, either because staying in D0 or
			 * disabling wake.
			 */
			If ((C2PW & Local0) != 0) {
				/*
				 * Clear Corresponding Device En BIT in C2PW.
				 */
				C2PW &= ~Local0
			}
			If ((CPWE != 0) && (C2PW == 0)) {
				/*
				 * If CPU WAKE EN is set, Clear it. Clear CPU WAKE EN
				 * by writing 0.
				 */
				CPWE = 0
			}
		}
	}
	Return (0)
}
