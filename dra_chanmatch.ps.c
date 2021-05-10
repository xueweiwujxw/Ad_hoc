/* dra_chanmatch.ps.c */                                                       
/* Default channel match model for radio link Transceiver Pipeline */

/****************************************/
/*		  Copyright (c) 1993-2008		*/
/*		by OPNET Technologies, Inc.		*/
/*		(A Delaware Corporation)		*/
/*	7255 Woodmont Av., Suite 250  		*/
/*     Bethesda, MD 20814, U.S.A.       */
/*			All Rights Reserved.		*/
/****************************************/

#define _USE_MATH_DEFINES
#include <math.h>
#include "opnet.h"


#if defined (__cplusplus)
extern "C"
#endif
void
dra_chanmatch_mt (OP_SIM_CONTEXT_ARG_OPT_COMMA Packet * pkptr)
	{
	double		tx_freq, tx_bw, tx_drate, tx_code;
	double		rx_freq, rx_bw, rx_drate, rx_code;
	double 		tx_x, tx_y, tx_z;
	double 		rx_x, rx_y, rx_z;
	double 		dif_x, dif_y, dif_z;
	Vartype		tx_mod;
	Vartype		rx_mod;

	/** Determine the compatibility between transmitter and receiver channels. **/
	FIN_MT (dra_chanmatch (pkptr));

	/* Obtain transmitting channel attributes. */
	tx_freq		= op_td_get_dbl (pkptr, OPC_TDA_RA_TX_FREQ);
	tx_bw		= op_td_get_dbl (pkptr, OPC_TDA_RA_TX_BW);
	tx_drate	= op_td_get_dbl (pkptr, OPC_TDA_RA_TX_DRATE);
	tx_code		= op_td_get_dbl (pkptr, OPC_TDA_RA_TX_CODE);
	tx_mod		= op_td_get_ptr (pkptr, OPC_TDA_RA_TX_MOD);	

	/* Obtain receiving channel attributes. */
	rx_freq		= op_td_get_dbl (pkptr, OPC_TDA_RA_RX_FREQ);
	rx_bw		= op_td_get_dbl (pkptr, OPC_TDA_RA_RX_BW);
	rx_drate	= op_td_get_dbl (pkptr, OPC_TDA_RA_RX_DRATE);
	rx_code		= op_td_get_dbl (pkptr, OPC_TDA_RA_RX_CODE);
	rx_mod		= op_td_get_ptr (pkptr, OPC_TDA_RA_RX_MOD);	

	/* For non-overlapping bands, the packet has no	*/
	/* effect; such packets are ignored entirely.	*/
	if ((tx_freq > rx_freq + rx_bw) || (tx_freq + tx_bw < rx_freq))
		{
		op_td_set_int (pkptr, OPC_TDA_RA_MATCH_STATUS, OPC_TDA_RA_MATCH_IGNORE);
		FOUT
		}

	/* Otherwise check for channel attribute mismatches which would	*/
	/* cause the in-band packet to be considered as noise.			*/
	if ((tx_freq != rx_freq) || (tx_bw != rx_bw) || 
		(tx_drate != rx_drate) || (tx_code != rx_code) || (tx_mod != rx_mod))
		{
		op_td_set_int (pkptr, OPC_TDA_RA_MATCH_STATUS, OPC_TDA_RA_MATCH_NOISE);
		FOUT
		}

	/* Otherwise the packet is considered a valid transmission which	*/
	/* could eventually be accepted at the error correction stage.		*/
	tx_x = op_td_get_dbl(pkptr, OPC_TDA_RA_TX_GEO_X);
	tx_y = op_td_get_dbl(pkptr, OPC_TDA_RA_TX_GEO_Y);
	tx_z = op_td_get_dbl(pkptr, OPC_TDA_RA_TX_GEO_Z);
	rx_x = op_td_get_dbl(pkptr, OPC_TDA_RA_RX_GEO_X);
	rx_y = op_td_get_dbl(pkptr, OPC_TDA_RA_RX_GEO_Y);
	rx_z = op_td_get_dbl(pkptr, OPC_TDA_RA_RX_GEO_Z);
	dif_x = tx_x - rx_x;
	dif_y = tx_y - rx_y;
	dif_z = tx_z - rx_z;
	if (sqrt(dif_x * dif_x + dif_y * dif_y + dif_z * dif_z) > 16e3) 
	{
		op_td_set_int (pkptr, OPC_TDA_RA_MATCH_STATUS, OPC_TDA_RA_MATCH_NOISE);
		FOUT
	}
	op_td_set_int (pkptr, OPC_TDA_RA_MATCH_STATUS, OPC_TDA_RA_MATCH_VALID);

	FOUT
	}         

