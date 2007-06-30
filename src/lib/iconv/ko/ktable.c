/*
 * CDDL HEADER START
 *
 * The contents of this file are subject to the terms of the
 * Common Development and Distribution License (the "License").  
 * You may not use this file except in compliance with the License.
 *
 * You can obtain a copy of the license at src/OPENSOLARIS.LICENSE
 * or http://www.opensolaris.org/os/licensing.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 *
 * When distributing Covered Code, include this CDDL HEADER in each
 * file and include the License file at src/OPENSOLARIS.LICENSE.
 * If applicable, add the following below this CDDL HEADER, with the
 * fields enclosed by brackets "[]" replaced with your own identifying
 * information: Portions Copyright [yyyy] [name of copyright owner]
 *
 * CDDL HEADER END
 */
/*
 * Copyright (c) 1994 by Sun Microsystems, Inc.
 *
 * Came from as "@(#)ktable.c	1.6 93/10/01 SMI: ALE"
 */

#pragma	ident	"$Id: ktable.c,v 1.6 1998/10/12 16:19:01 hemant Exp $ SMI"


/*--------------------------------------------------------------*/
/*  Conversion table for initial and middle sound		*/
/*--------------------------------------------------------------*/


/*  7 bit Sound ---> 5 bit Combination Code		*/
/*	: give 5 bit combination code to each sound	*/

short X32_19[32] =	/* INITIAL SOUND	*/
	{         -1,
		0x0a,	/* gi-ug		*/
		0x0b,	/* double gi-ug		*/
		  -1,	/*			*/
		0x0c,	/* ni-un		*/
		  -1,	/*			*/
		  -1,	/*			*/
		0x0d,	/* di-gud		*/
		0x0e,	/* double di-gud	*/
		0x0f,	/* ri-ul		*/
		  -1,	/*			*/
		  -1,	/*			*/
		  -1,	/*			*/
		  -1,	/*			*/
		  -1,	/*			*/
		  -1,	/*			*/
		  -1,	/*			*/
		0x10,	/* mi-um		*/
		0x11,	/* bi-ub		*/
		0x12,	/* double bi-ub		*/
		  -1,	/*			*/
		0x13,	/* si-od		*/
		0x14,	/* double si-od		*/
		0x15,	/* i-ung		*/
		0x16,	/* ji-ud		*/
		0x17,	/* double ji-ud		*/
		0x18,	/* chi-ud		*/
		0x19,	/* ki-uk		*/
		0x1a,	/* ti-ud		*/
		0x1b,	/* pi-up		*/
		0x1c,	/* hi-ud		*/
		  -1,	/*			*/
	} ;

short X32_21[32] =	/* MIDDLE SOUND		*/
	{         -1,   /*			*/
		  -1,	/* FILL		        */
		0x02,	/* a            	*/
		0x03,	/* a + i		*/ 
		0x05,	/* ya   	 	*/
		0x06,	/* ya + i		*/
		0x07,	/* eo			*/
		0x09,	/* eo + i		*/
		  -1,	/* 			*/
		  -1,	/* 			*/
		0x0a,	/* yeo			*/
		0x0b,	/* yeo + i		*/
		0x0d,	/* o			*/
		0x0e,	/* o + a		*/
		0x0f,	/* o + a + i		*/
		0x11,	/* o + i 		*/
		  -1,	/* 			*/
		  -1,	/* 			*/
		0x12,	/* yo   		*/
		0x13,	/* u    		*/
		0x15,	/* u + eo       	*/
		0x16,	/* u + eo + i		*/
		0x17,	/* u + i        	*/
		0x19,	/* yu   	 	*/
		  -1,	/* 			*/
		  -1,	/* 			*/
		0x1a,	/* _            	*/
		0x1b,	/* _ + i  		*/
		0x1d,	/* i    	 	*/
		  -1,	/*			*/
		  -1,	/*			*/
		  -1,	/*			*/
	};

short X32_28[32] =	/* FINAL SOUND		*/
	{	  -1,	/* FILL			*/
		0x02,	/* gi-ug		*/
		0x03,	/* double gi-ug		*/
		0x04,	/* gi-ug si-od		*/
		0x05,	/* ni-un		*/
		0x06,	/* ni-un ji-od		*/
		0x07,	/* ni-un hi-ud		*/
		0x08,	/* di-gud		*/
		  -1,	/* double di-gud	*/
		0x09,	/* ri-ul		*/
		0x0a,	/* ri-ul gi-ug		*/
		0x0b,	/* ri-ul mi-um		*/
		0x0c,	/* ri-ul bi-ub		*/
		0x0d,	/* ri-ul si-od		*/
		0x0e,	/* ri-ul ti-gud		*/
		0x0f,	/* ri-ul pi-op		*/
		0x10,	/* ri-ul hi-ud		*/
		0x11,	/* mi-um		*/
		0x12,	/* bi-ub		*/
		  -1,	/* double bi-ub		*/
		0x13,	/* bi-ub si-od		*/
		0x14,	/* si-od		*/
		0x15,	/* double si-od		*/
		0x16,	/* i-ung		*/
		0x17,	/* ji-ud		*/
		  -1,	/* double ji-ud		*/
		0x18,	/* chi-ud		*/
		0x19,	/* ki-uk		*/
		0x1a,	/* ti-ud		*/
		0x1b,	/* pi-up		*/
		0x1c,	/* hi-ud		*/
		  -1,	/*			*/
	} ;


/*  5 bit Combination Code ---> 7 bit code 	*/
/*	: give 7 bit Code to each Sound		*/

short Y19_32[20] =	/* INITIAL SOUND	*/
	{	0x00,	/* (Fill) 		*/
		0x01,	/* gi-uk		*/
		0x02,	/* double gi-ug		*/
		0x04,	/* ni-un		*/
		0x07,	/* di-gud		*/
		0x08,	/* double di-gud	*/
		0x09,	/* ri-ul		*/
		0x11,	/* mi-um		*/
		0x12,	/* bi-ub		*/
		0x13,	/* double bi-ub		*/
		0x15,	/* si-od		*/
		0x16,	/* double si-od		*/
		0x17,	/* i-ung		*/
		0x18,	/* ji-ud		*/
		0x19,	/* double ji-ud		*/
		0x1a,	/* chi-ud		*/
		0x1b,	/* ki-uk		*/
		0x1c,	/* ti-ud		*/
		0x1d,	/* pi-up		*/
		0x1e,	/* hi-ud		*/
	} ;


short Y21_32[32] =	/* MIDDLE SOUND		*/
	{	  -1,   /*			*/
		0x00,	/* (FILL)	        */
		0x02,	/* a            	*/
		0x03,	/* a + i		*/ 
		  -1,	/* 			*/
		0x04,	/* ya   	 	*/
		0x05,	/* ya + i		*/
		0x06,	/* eo			*/
		  -1,	/* 			*/
		0x07,	/* eo + i		*/
		0x0a,	/* yeo			*/
		0x0b,	/* yeo + i		*/
		  -1,	/* 			*/
		0x0c,	/* o			*/
		0x0d,	/* o + a		*/
		0x0e,	/* o + a + i		*/
		  -1,	/* 			*/
		0x0f,	/* o + i 		*/
		0x12,	/* yo   		*/
		0x13,	/* u    		*/
		  -1,	/*			*/
		0x14,	/* u + eo       	*/
		0x15,	/* u + eo + i		*/
		0x16,	/* u + i        	*/
		  -1,	/*			*/
		0x17,	/* yu   	 	*/
		0x1a,	/* _            	*/
		0x1b,	/* _ + i  		*/
		  -1,	/* 		     	*/
		0x1c,	/* i    	 	*/
		  -1,	/*			*/
		  -1,	/*			*/
	} ;

short Y28_32[28] =	/* FINAL SOUND		*/
	{	0x00,	/* (FILL)		*/
		0x01,	/* gi-ug		*/
		0x02,	/* double gi-ug		*/
		0x03,	/* gi-ug si-od		*/
		0x04,	/* ni-un		*/
		0x05,	/* ni-un ji-od		*/
		0x06,	/* ni-un hi-ud		*/
		0x07,	/* di-gud		*/
		0x09,	/* ri-ul		*/
		0x0a,	/* ri-ul gi-ug		*/
		0x0b,	/* ri-ul mi-um		*/
		0x0c,	/* ri-ul bi-ub		*/
		0x0d,	/* ri-ul si-od		*/
		0x0e,	/* ri-ul ti-gud		*/
		0x0f,	/* ri-ul pi-op		*/
		0x10,	/* ri-ul hi-ud		*/
		0x11,	/* mi-um		*/
		0x12,	/* bi-ub		*/
		0x14,	/* bi-ub si-od		*/
		0x15,	/* si-od		*/
		0x16,	/* double si-od		*/
		0x17,	/* i-ung		*/
		0x18,	/* ji-ud		*/
		0x1a,	/* chi-ud		*/
		0x1b,	/* ki-uk		*/
		0x1c,	/* ti-ud		*/
		0x1d,	/* pi-up		*/
		0x1e,	/* hi-ud		*/
	} ;


/* Bit map of all possible Hangul Character compositions.
 *  first  sound = 19 consonants;
 *  middle sound = 21 vowels;
 *  final  sound = 28 consonants;
 * For each array element of first_sound and middle_sound, there is a bitmap
 *   of 28 final_sound in 32bits according to New Standard Code(87-3).
 * Each map looks like 
 *	left [...hi-ud...ri-ul..gi-ug..]right
 */

/* Appending Capital 'L' */
long cmp_bitmap[19][21] = {

/* gi-ug */
{0x1dfe0f26L, 0x760226L, 0x500226L, 0x222L, 0x1cf60b26L, 0x760222L, 0x476032eL, 0x140222L, 0xd72b26L, 0x560a26L, 0x640222L, 0x560226L, 0x140222L, 0xd70f26L, 0x600226L, 0x100002L, 0x160226L, 0x222L, 0x560726L, 0x2L, 0x8d60b26L},

/* Double gi-ug */
{0x4760a2eL, 0x760226L, 0x206L, 0x0L, 0x76022eL, 0x520026L, 0x4300222L, 0x2L, 0x1d602a6L, 0x600206L, 0x400006L, 0x460222L, 0x2L, 0xd70226L, 0x600202L, 0x260226L, 0x60222L, 0x2L, 0x4570aa6L, 0x0L, 0x560226L},

/* ni-un */
{0x15f60f2eL, 0x760226L, 0x420226L, 0x0L, 0x10761a36L, 0x760226L, 0x2660226L, 0x22L, 0x18560a26L, 0x200222L, 0x0L, 0x160222L, 0x540226L, 0x560326L, 0x200002L, 0x2L, 0x60222L, 0x460206L, 0x8d60e26L, 0x222L, 0x8560a26L},

/* di-gud */
{0x11f71f2eL, 0x760226L, 0x2L, 0x0L, 0x9561b2eL, 0x760226L, 0x600222L, 0x22L, 0x5562b26L, 0x222L, 0x200002L, 0x160222L, 0x2L, 0x560226L, 0x200002L, 0x400002L, 0x540222L, 0x420222L, 0x560b26L, 0x2L, 0xf60326L},

/* Double di-gud */
{0x10760226L, 0x760226L, 0x0L, 0x0L, 0x10761a26L, 0x760226L, 0x200002L, 0x0L, 0x400226L, 0x202L, 0x2L, 0x22L, 0x0L, 0x430226L, 0x0L, 0x2L, 0x460222L, 0x0L, 0x160326L, 0x60222L, 0x560222L},

/* ri-ul */
{0x18f60226L, 0x760226L, 0x500026L, 0x0L, 0x10760226L, 0x560226L, 0x760226L, 0x140022L, 0x560226L, 0x400022L, 0x200000L, 0x560222L, 0x540222L, 0x560226L, 0x200002L, 0x2L, 0x520226L, 0x560226L, 0xcd60226L, 0x0L, 0x560226L},

/* mi-um */
{0x14d60fa6L, 0xf60226L, 0x400206L, 0x0L, 0x10d60a26L, 0x760226L, 0x1700226L, 0x2L, 0x560a36L, 0x600022L, 0x0L, 0x540222L, 0x140222L, 0x14560f2eL, 0x140222L, 0x2L, 0x222L, 0x120222L, 0x120222L, 0x0L, 0x5760b26L},

/* bi-ob */
{0x4561f3eL, 0x4760226L, 0x40026L, 0x0L, 0xd60b26L, 0x760326L, 0x4740226L, 0x22L, 0x56022eL, 0x200022L, 0x200002L, 0x60226L, 0x22L, 0xc560f26L, 0x200202L, 0x2L, 0x400226L, 0x520222L, 0x160226L, 0x0L, 0x1d60a26L},

/* Double bi-ob */
{0x10760a26L, 0x760226L, 0x20006L, 0x0L, 0x720326L, 0x400002L, 0x760006L, 0x0L, 0x460226L, 0x0L, 0x0L, 0x2L, 0x400002L, 0x520226L, 0x0L, 0x0L, 0x0L, 0x400002L, 0x60222L, 0x0L, 0x560226L},

/* si-od */
{0x4760f36L, 0x760226L, 0x560226L, 0x420222L, 0x8761b3eL, 0x760226L, 0x760226L, 0x400222L, 0x4560a2eL, 0x400226L, 0x320222L, 0x160222L, 0x560226L, 0xd560326L, 0x200002L, 0x420226L, 0x560226L, 0x520206L, 0x560626L, 0x0L, 0x8570326L},

/* Double si-od */
{0x10660236L, 0x660226L, 0x400000L, 0x0L, 0x660a26L, 0x222L, 0x0L, 0x20L, 0x460b26L, 0x200026L, 0x200002L, 0x60222L, 0x2L, 0x460226L, 0x200002L, 0x2L, 0x22L, 0x400000L, 0x70a26L, 0x20222L, 0x560226L},

/* i-ung */
{0xc770ee6L, 0x760226L, 0x14561226L, 0x40222L, 0xafe0f66L, 0x560226L, 0x1c7e1a2eL, 0x360222L, 0x1572e26L, 0x760226L, 0x520026L, 0x560226L, 0x560226L, 0x560e26L, 0x660226L, 0x460226L, 0x560226L, 0x1560226L, 0x1fd68226L, 0x120222L, 0x8f70e26L},

/* ji-od */
{0xf60ba6L, 0x760226L, 0x4202a6L, 0x222L, 0xd60a26L, 0x560226L, 0x660222L, 0x2L, 0x11d60a26L, 0x540206L, 0x600002L, 0x560222L, 0x400026L, 0x560e26L, 0x200002L, 0x2L, 0x160226L, 0x20222L, 0x560226L, 0x0L, 0xcd60B26L},

/* Double ji-od */
{0x7612a6L, 0x760226L, 0x400022L, 0x0L, 0x760226L, 0x400002L, 0x200002L, 0x0L, 0x1560226L, 0x200206L, 0x200002L, 0x60222L, 0x400000L, 0x460226L, 0x600002L, 0x0L, 0x2L, 0x2L, 0x520002L, 0x0L, 0x10c60226L},

/* chi-od */
{0xf602a6L, 0x760226L, 0x4202a2L, 0x0L, 0x760226L, 0x560226L, 0x200022L, 0x400022L, 0x560226L, 0x400222L, 0x0L, 0x560222L, 0x20002L, 0x560226L, 0x200002L, 0x22L, 0x560222L, 0x420222L, 0x560226L, 0x0L, 0x560726L},

/* ki-ug */
{0x560226L, 0x760226L, 0x400006L, 0x0L, 0x760326L, 0x560226L, 0x760222L, 0x2L, 0x560226L, 0x420226L, 0x400002L, 0x202L, 0x2L, 0x560226L, 0x400222L, 0x400002L, 0x560226L, 0x20222L, 0x460226L, 0x0L, 0x560226L},

/* ti-gud */
{0x760626L, 0x760226L, 0x400002L, 0x0L, 0x760a26L, 0x560226L, 0x200022L, 0x22L, 0x8560226L, 0x22L, 0x2L, 0x500022L, 0x2L, 0x560226L, 0x200002L, 0x2L, 0x460226L, 0x420222L, 0x160b26L, 0x60222L, 0x560226L},

/* pi-ob */
{0x4760a2eL, 0x760226L, 0x6L, 0x0L, 0x760226L, 0x560226L, 0x660222L, 0x140202L, 0x560226L, 0x400002L, 0x0L, 0x22L, 0x140222L, 0x560b26L, 0x400002L, 0x0L, 0x120222L, 0x520222L, 0x160222L, 0x0L, 0x560226L},

/* hi-ud */
{0x564226L, 0x760226L, 0x400002L, 0x0L, 0x560a26L, 0x560226L, 0x760226L, 0x40222L, 0x4564226L, 0x500226L, 0x500026L, 0x540226L, 0x140222L, 0x524226L, 0x420222L, 0x400226L, 0x560226L, 0x520226L, 0x45607a6L, 0x460222L, 0x560226L}
};

/*
 * Each cmp_srctbl[i][j] has 2-byte compeletion code 
 *   where i is initial_sound and j is middle_sound.
 * So, cmp_srctbl[i][0] is the code for some initial_sound and
 *   the first of middle_sound(always 'a').
 */

unsigned short cmp_srchtbl[19][21] = {

/* gi-ug */
{0xb0a1, 0xb0b3, 0xb0bc, 0xb0c2, 0xb0c5, 0xb0d4, 0xb0dc, 0xb0e8, 0xb0ed, 0xb0fa, 0xb1a5, 0xb1ab, 0xb1b3, 0xb1b8, 0xb1c5, 0xb1cb, 0xb1cd, 0xb1d4, 0xb1d7, 0xb1e1, 0xb1e2},

/* Double gi-ug */
{0xb1ee, 0xb1fa, 0xb2a5, 0x0000, 0xb2a8, 0xb2b2, 0xb2b8, 0xb2be, 0xb2bf, 0xb2ca, 0xb2cf, 0xb2d2, 0xb2d8, 0xb2d9, 0xb2e3, 0xb2e7, 0xb2ee, 0xb2f3, 0xb2f4, 0x0000, 0xb3a2 },

/* ni-un */
{0xb3aa, 0xb3bb, 0xb3c4, 0x0000, 0xb3ca, 0xb3d7, 0xb3e0, 0xb3e9, 0xb3eb, 0xb3f6, 0x0000, 0xb3fa, 0xb4a2, 0xb4a9, 0xb4b2, 0xb4b4, 0xb4b5, 0xb4ba, 0xb4c0, 0xb4cc, 0xb4cf},

/* di-gud */
{0xb4d9, 0xb4eb, 0xb4f4, 0x0000, 0xb4f5, 0xb5a5, 0xb5ae, 0xb5b3, 0xb5b5, 0xb5c2, 0xb5c5, 0xb5c7, 0xb5cd, 0xb5ce, 0xb5d6, 0xb5d8, 0xb5da, 0xb5e0, 0xb5e5, 0xb5ef, 0xb5f0},

/* Double di-gud */
{0xb5fb, 0xb6a7, 0x0000, 0x0000, 0xb6b0, 0xb6bc, 0xb6c5, 0x0000, 0xb6c7, 0xb6cc, 0xb6ce, 0xb6cf, 0x0000, 0xb6d1, 0x0000, 0xb6d8, 0xb6d9, 0x0000, 0xb6df, 0xb6e7, 0xb6ec},

/* ri-ul */
{0xb6f3, 0xb7a1, 0xb7aa, 0x0000, 0xb7af, 0xb7b9, 0xb7c1, 0xb7ca, 0xb7ce, 0xb7d6, 0xb7d9, 0xb7da, 0xb7e1, 0xb7e7, 0xb7ef, 0xb7f1, 0xb7f2, 0xb7f9, 0xb8a3, 0x0000, 0xb8ae},

/* mi-um */
{0xb8b6, 0xb8c5, 0xb8cf, 0x0000, 0xb8d3, 0xb8de, 0xb8e7, 0xb8ef, 0xb8f0, 0xb8fa, 0x0000, 0xb8fe, 0xb9a6, 0xb9ab, 0xb9b9, 0xb9be, 0xb9bf, 0xb9c2, 0xb9c7, 0x0000, 0xb9cc},

/* bi-ub */
{0xb9d9, 0xb9e8, 0xb9f2, 0x0000, 0xb9f6, 0xbaa3, 0xbaad, 0xbab6, 0xbab8, 0xbac1, 0xbac4, 0xbac6, 0xbacc, 0xbace, 0xbadb, 0xbade, 0xbadf, 0xbae4, 0xbaea, 0x0000, 0xbaf1},

/* Double bi-ub */
{0xbafc, 0xbba9, 0xbbb2, 0x0000, 0xbbb5, 0xbbbe, 0xbbc0, 0x0000, 0xbbc7, 0x0000, 0x0000, 0xbbce, 0xbbcf, 0xbbd1, 0x0000, 0x0000, 0x0000, 0xbbd8, 0xbbda, 0x0000, 0xbbdf},

/* si-os */
{0xbbe7, 0xbbf5, 0xbbfe, 0xbca8, 0xbcad, 0xbcbc, 0xbcc5, 0xbcce, 0xbcd2, 0xbcdd, 0xbce2, 0xbce8, 0xbcee, 0xbcf6, 0xbda4, 0xbda6, 0xbdac, 0xbdb4, 0xbdba, 0x0000, 0xbdc3},

/* Double si-os */
{0xbdce, 0xbdd8, 0xbde0, 0x0000, 0xbde1, 0xbdea, 0x0000, 0xbded, 0xbdee, 0xbdf7, 0xbdfb, 0xbdfd, 0xbea4, 0xbea5, 0xbeac, 0xbeae, 0xbeaf, 0xbeb1, 0xbeb2, 0xbeba, 0xbebe},

/* i-ung */
{0xbec6, 0xbed6, 0xbedf, 0xbeea, 0xbeee, 0xbfa1, 0xbfa9, 0xbfb9, 0xbfc0, 0xbfcd, 0xbfd6, 0xbfdc, 0xbfe4, 0xbfec, 0xbff6, 0xbffe, 0xc0a7, 0xc0af, 0xc0b8, 0xc0c7, 0xc0cc},

/* ji-od */
{0xc0da, 0xc0e7, 0xc0f0, 0xc0f7, 0xc0fa, 0xc1a6, 0xc1ae, 0xc1b5, 0xc1b6, 0xc1c2, 0xc1c8, 0xc1cb, 0xc1d2, 0xc1d6, 0xc1e0, 0xc1e2, 0xc1e3, 0xc1ea, 0xc1ee, 0x0000, 0xc1f6},

/* Double ji-od */
{0xc2a5, 0xc2b0, 0xc2b9, 0x0000, 0xc2bc, 0xc2c5, 0xc2c7, 0x0000, 0xc2c9, 0xc2d2, 0xc2d6, 0xc2d8, 0xc2dd, 0xc2de, 0xc2e5, 0x0000, 0xc2e8, 0xc2e9, 0xc2ea, 0x0000, 0xc2ee},

/* chi-od */
{0xc2f7, 0xc3a4, 0xc3ad, 0x0000, 0xc3b3, 0xc3bc, 0xc3c4, 0xc3c7, 0xc3ca, 0xc3d2, 0x0000, 0xc3d6, 0xc3dd, 0xc3df, 0xc3e7, 0xc3e9, 0xc3eb, 0xc3f2, 0xc3f7, 0x0000, 0xc4a1},

/* ki-ug */
{0xc4ab, 0xc4b3, 0xc4bc, 0x0000, 0xc4bf, 0xc4c9, 0xc4d1, 0xc4d9, 0xc4da, 0xc4e2, 0xc4e8, 0xc4ea, 0xc4ec, 0xc4ed, 0xc4f5, 0xc4f9, 0xc4fb, 0xc5a5, 0xc5a9, 0x0000, 0xc5b0},

/* ti-gud */
{0xc5b8, 0xc5c2, 0xc5cb, 0x0000, 0xc5cd, 0xc5d7, 0xc5df, 0xc5e2, 0xc5e4, 0xc5ed, 0xc5ef, 0xc5f0, 0xc5f4, 0xc5f5, 0xc5fd, 0xc6a1, 0xc6a2, 0xc6a9, 0xc6ae, 0xc6b7, 0xc6bc},

/* pi-ob */
{0xc6c4, 0xc6d0, 0xc6d9, 0x0000, 0xc6db, 0xc6e4, 0xc6ec, 0xc6f3, 0xc6f7, 0xc7a1, 0x0000, 0xc7a3, 0xc7a5, 0xc7aa, 0xc7b4, 0x0000, 0xc7b6, 0xc7bb, 0xc7c1, 0x0000, 0xc7c7},

/* hi-ud */
{0xc7cf, 0xc7d8, 0xc7e1, 0x0000, 0xc7e3, 0xc7ec, 0xc7f4, 0xc7fd, 0xc8a3, 0xc8ad, 0xc8b3, 0xc8b8, 0xc8bf, 0xc8c4, 0xc8cc, 0xc8d1, 0xc8d6, 0xc8de, 0xc8e5, 0xc8f1, 0xc8f7}

};	
