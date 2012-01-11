/*
 *  PocketGnuboy - GameBoy emulator for PocketPC
 *  Copyright (C) 2003  Y.Nagamidori
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 *  based on gnuboy 1.0.3
 *  Copyright (C) 2000-2001 Laguna and Gilgamesh
 */

#ifndef __CPU_H__
#define __CPU_H__



#include "defs.h"


union reg
{
        byte b[2][2];
        word w[2];
        un32 d; /* padding for alignment, carry */
};

struct cpu
{
        union reg pc, sp, bc, de, hl, af;
        int ime;
        int speed;
        int halt;
        int div, tim;
        int lcdc;
        int snd;

        int emulate;
        int tac_speed;
        int tac_start;
};

extern struct cpu cpu;

#define ZFLAG(n) ( (n) ? 0 : FZ )
#define PUSH(w) ( (SP -= 2), (writew(xSP, (w))) )
#define POP(w) ( ((w) = readw(xSP)), (SP += 2) )


#define FETCH_OLD ( mbc.rmap[PC>>12] \
? mbc.rmap[PC>>12][PC++] \
: mem_read(PC++) )

#define FETCH (readb(PC++))

#define INC(r) { (r)++;     \
F = (F & FC) | ZFLAG((r)) | (((r) & 0x0F) ? 0 : FH); }

#define DEC(r) { (r)--;     \
F = FN | (F & FC) | ZFLAG((r)) | (((r) & 0x0F) == 0x0F ? FH : 0); }


#define INCW(r) ( (r)++ )

#define DECW(r) ( (r)-- )

#define ADD(n) {    \
W(acc) = A + (n);   \
F = HB(acc) | ZFLAG(LB(acc)) | ((A ^ (n) ^ LB(acc)) & FH); \
A = LB(acc); }

#define ADC(n) {  \
W(acc) = A + (n) + (F & FC); \
F = HB(acc) | ZFLAG(LB(acc)) | ((A ^ (n) ^ LB(acc)) & FH); \
A = LB(acc); }

#define ADDW(r) { \
W(acc) = HL + (r); \
F = (F & FZ) | (((HL ^ (r) ^ W(acc)) & 0x1000) ? FH : 0) | \
((((un32)HL + (un32)(r)) & 0x10000) ? FC : 0); \
HL = W(acc); }

#define ADDSP { b = FETCH; SP += (n8)b; }

#define LDHLSP { b = FETCH; HL = SP + (n8)b; }

#define CP(n) { \
W(acc) = A - (n); \
F = FN | -HB(acc) | ZFLAG(LB(acc)) | ((A ^ (n) ^ LB(acc)) & FH); }

#define SUB(n) { \
W(acc) = A - (n); \
F = FN | -HB(acc) | ZFLAG(LB(acc)) | ((A ^ (n) ^ LB(acc)) & FH); \
A = LB(acc); }

#define SBC(n) { \
W(acc) = A - (n) - (F & FC); \
F = FN | -HB(acc) | ZFLAG(LB(acc)) | ((A ^ (n) ^ LB(acc)) & FH); \
A = LB(acc); }

#define AND(n) { A &= (n); \
F = ZFLAG(A) | FH; }

#define XOR(n) { A ^= (n); \
F = ZFLAG(A); }

#define OR(n) { A |= (n); \
F = ZFLAG(A); }

#define RLCA { \
F = (A >> 7); A = (A << 1) | F; }

#define RRCA { \
F = (A & 0x01); A = (A >> 1) | (A << 7); }

#define RLA { \
b = A >> 7; A = (A << 1) | (F & FC); F = b; }

#define RRA { \
b = A & 0x01; A = (A >> 1) | (F << 7); F = b; }

#define RLC(r) { \
F = (r) >> 7; (r) = ((r) << 1) | F; F |= ZFLAG(r); }

#define RRC(r) { \
F = (r) & 0x01; (r) = ((r) >> 1) | (F << 7); F |= ZFLAG(r); }

#define RL(r) { \
b = F & 0x01; F = ((r) >> 7); (r) = ((r) << 1) | b; F |= ZFLAG((r)); }

#define RR(r) { \
b = F & 0x01; F = ((r) & 0x01); (r) = ((r) >> 1) | (b << 7); F |= ZFLAG((r)); }

#define SLA(r) { \
F = (r) >> 7; (r) <<= 1; F |= ZFLAG(r); }

#define SRA(r) { \
F = (r) & 0x01; (r) = ((r) >> 1) | ((r) & 0x80); F |= ZFLAG(r); }

#define SRL(r) { \
F = (r) & 0x01; (r) >>= 1; F |= ZFLAG(r); }

#define CPL(r) { \
(r) = ~(r); \
F |= (FH | FN); }

#define SCF { F = (F & ~(FN | FH)) | FC; }

#define CCF { F ^= 0x01; F = F & ~(FN | FH); }

#define DAA { \
        HB(acc) = A & 0x0F; \
        if (F & FN) \
                W(acc) = (F & FC) ? (((F & FH) ? 0x9A00 : 0xA000) + FC) : ((F & FH) ? 0xFA00 : 0x0000); \
        else if (F & FC) \
                W(acc) = (((F & FH) ? 0x6600 : ((HB(acc) < 0x0A) ? 0x6000 : 0x6600)) + FC); \
        else if (F & FH) \
                W(acc) = ((A < 0xA0)? 0x0600 : (0x6600+FC)); \
        else if (HB(acc) < 0x0A) \
                W(acc) = ((A < 0xA0) ? 0x0000 : (0x6000 + FC)); \
        else \
                W(acc) = ((A < 0x90) ? 0x0600 : (0x6600+FC)); \
        A += HB(acc); \
        F |= (LB(acc) | (F & FN)) | ZFLAG(A); \
}

#define SWAP(r) { \
(r) = ((r) >> 4) | ((r) << 4); F = ZFLAG(r); }

#define BIT0(r) {\
F = ((F & FC) | FH) | ((((r) << 6) & 0x40) ^ 0x40); }
#define BIT1(r) {\
F = ((F & FC) | FH) | ((((r) << 5) & 0x40) ^ 0x40); }
#define BIT2(r) {\
F = ((F & FC) | FH) | ((((r) << 4) & 0x40) ^ 0x40); }
#define BIT3(r) {\
F = ((F & FC) | FH) | ((((r) << 3) & 0x40) ^ 0x40); }
#define BIT4(r) {\
F = ((F & FC) | FH) | ((((r) << 2) & 0x40) ^ 0x40); }
#define BIT5(r) {\
F = ((F & FC) | FH) | ((((r) << 1) & 0x40) ^ 0x40); }
#define BIT6(r) {\
F = ((F & FC) | FH) | (((r) & 0x40) ^ 0x40); }
#define BIT7(r) {\
F = ((F & FC) | FH) | ((((r) >> 1) & 0x40) ^ 0x40); }

#define RES(n,r) { (r) &= ~(1 << (n)); }
#define SET(n,r) { (r) |= (1 << (n)); }

#define CB_REG_CASES(r, n) \
case 0x00|(n): RLC(r); break; \
case 0x08|(n): RRC(r); break; \
case 0x10|(n): RL(r); break; \
case 0x18|(n): RR(r); break; \
case 0x20|(n): SLA(r); break; \
case 0x28|(n): SRA(r); break; \
case 0x30|(n): SWAP(r); break; \
case 0x38|(n): SRL(r); break; \
case 0x40|(n): BIT0(r); break; \
case 0x48|(n): BIT1(r); break; \
case 0x50|(n): BIT2(r); break; \
case 0x58|(n): BIT3(r); break; \
case 0x60|(n): BIT4(r); break; \
case 0x68|(n): BIT5(r); break; \
case 0x70|(n): BIT6(r); break; \
case 0x78|(n): BIT7(r); break; \
case 0x80|(n): RES(0, r); break; \
case 0x88|(n): RES(1, r); break; \
case 0x90|(n): RES(2, r); break; \
case 0x98|(n): RES(3, r); break; \
case 0xA0|(n): RES(4, r); break; \
case 0xA8|(n): RES(5, r); break; \
case 0xB0|(n): RES(6, r); break; \
case 0xB8|(n): RES(7, r); break; \
case 0xC0|(n): SET(0, r); break; \
case 0xC8|(n): SET(1, r); break; \
case 0xD0|(n): SET(2, r); break; \
case 0xD8|(n): SET(3, r); break; \
case 0xE0|(n): SET(4, r); break; \
case 0xE8|(n): SET(5, r); break; \
case 0xF0|(n): SET(6, r); break; \
case 0xF8|(n): SET(7, r); break;

#define ALU_CASES(base, imm, op) \
case (imm): b = FETCH; op(b); break; \
case (base): op(B); break; \
case (base)+1: op(C); break; \
case (base)+2: op(D); break; \
case (base)+3: op(E); break; \
case (base)+4: op(H); break; \
case (base)+5: op(L); break; \
case (base)+6: op(readb(HL)); break; \
case (base)+7: op(A); break;

#define JR ( PC += 1+(n8)readb(PC) )
#define JP ( PC = readw(PC) )

#define CALL ( W(acc) = PC+2, PUSH(W(acc)), JP )

#define NOJR ( clen-=2, PC++ )
#define NOJP ( clen-=2, PC+=2 )
#define NOCALL ( clen-=6, PC+=2 )
#define NORET ( clen-=6 )

#define RST(n) { PUSH(PC); PC = (n); }
#define RET ( POP(PC) )

#define EI ( IME = 1 )
#define DI ( IME = 0 )

#define PRE_INT ( DI, PUSH(PC) )
#define THROW_INT(n) ( (IF &= ~(1<<(n))), (PC = 0x40+((n)<<3)) )

#endif


