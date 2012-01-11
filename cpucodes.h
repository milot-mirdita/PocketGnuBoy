case 0x00: /* NOP */
case 0x40: /* LD B,B */
case 0x49: /* LD C,C */
case 0x52: /* LD D,D */
case 0x5B: /* LD E,E */
case 0x64: /* LD H,H */
case 0x6D: /* LD L,L */
case 0x7F: /* LD A,A */
        break;

case 0x41: /* LD B,C */
        B = C; break;
case 0x42: /* LD B,D */
        B = D; break;
case 0x43: /* LD B,E */
        B = E; break;
case 0x44: /* LD B,H */
        B = H; break;
case 0x45: /* LD B,L */
        B = L; break;
case 0x46: /* LD B,(HL) */
        B = readb(xHL); break;
case 0x47: /* LD B,A */
        B = A; break;

case 0x48: /* LD C,B */
        C = B; break;
case 0x4A: /* LD C,D */
        C = D; break;
case 0x4B: /* LD C,E */
        C = E; break;
case 0x4C: /* LD C,H */
        C = H; break;
case 0x4D: /* LD C,L */
        C = L; break;
case 0x4E: /* LD C,(HL) */
        C = readb(xHL); break;
case 0x4F: /* LD C,A */
        C = A; break;

case 0x50: /* LD D,B */
        D = B; break;
case 0x51: /* LD D,C */
        D = C; break;
case 0x53: /* LD D,E */
        D = E; break;
case 0x54: /* LD D,H */
        D = H; break;
case 0x55: /* LD D,L */
        D = L; break;
case 0x56: /* LD D,(HL) */
        D = readb(xHL); break;
case 0x57: /* LD D,A */
        D = A; break;

case 0x58: /* LD E,B */
        E = B; break;
case 0x59: /* LD E,C */
        E = C; break;
case 0x5A: /* LD E,D */
        E = D; break;
case 0x5C: /* LD E,H */
        E = H; break;
case 0x5D: /* LD E,L */
        E = L; break;
case 0x5E: /* LD E,(HL) */
        E = readb(xHL); break;
case 0x5F: /* LD E,A */
        E = A; break;

case 0x60: /* LD H,B */
        H = B; break;
case 0x61: /* LD H,C */
        H = C; break;
case 0x62: /* LD H,D */
        H = D; break;
case 0x63: /* LD H,E */
        H = E; break;
case 0x65: /* LD H,L */
        H = L; break;
case 0x66: /* LD H,(HL) */
        H = readb(xHL); break;
case 0x67: /* LD H,A */
        H = A; break;

case 0x68: /* LD L,B */
        L = B; break;
case 0x69: /* LD L,C */
        L = C; break;
case 0x6A: /* LD L,D */
        L = D; break;
case 0x6B: /* LD L,E */
        L = E; break;
case 0x6C: /* LD L,H */
        L = H; break;
case 0x6E: /* LD L,(HL) */
        L = readb(xHL); break;
case 0x6F: /* LD L,A */
        L = A; break;

case 0x70: /* LD (HL),B */
        writeb(xHL,B); break;
case 0x71: /* LD (HL),C */
        writeb(xHL,C); break;
case 0x72: /* LD (HL),D */
        writeb(xHL,D); break;
case 0x73: /* LD (HL),E */
        writeb(xHL,E); break;
case 0x74: /* LD (HL),H */
        writeb(xHL,H); break;
case 0x75: /* LD (HL),L */
        writeb(xHL,L); break;
case 0x77: /* LD (HL),A */
        writeb(xHL,A); break;

case 0x78: /* LD A,B */
        A = B; break;
case 0x79: /* LD A,C */
        A = C; break;
case 0x7A: /* LD A,D */
        A = D; break;
case 0x7B: /* LD A,E */
        A = E; break;
case 0x7C: /* LD A,H */
        A = H; break;
case 0x7D: /* LD A,L */
        A = L; break;
case 0x7E: /* LD A,(HL) */
        A = readb(xHL); break;

case 0x01: /* LD BC,imm */
        BC = readw(xPC); PC += 2; break;
case 0x11: /* LD DE,imm */
        DE = readw(xPC); PC += 2; break;
case 0x21: /* LD HL,imm */
        HL = readw(xPC); PC += 2; break;
case 0x31: /* LD SP,imm */
        SP = readw(xPC); PC += 2; break;

case 0x02: /* LD (BC),A */
        writeb(xBC, A); break;
case 0x0A: /* LD A,(BC) */
        A = readb(xBC); break;
case 0x12: /* LD (DE),A */
        writeb(xDE, A); break;
case 0x1A: /* LD A,(DE) */
        A = readb(xDE); break;

case 0x22: /* LDI (HL),A */
        writeb(xHL, A); HL++; break;
case 0x2A: /* LDI A,(HL) */
        A = readb(xHL); HL++; break;
case 0x32: /* LDD (HL),A */
        writeb(xHL, A); HL--; break;
case 0x3A: /* LDD A,(HL) */
        A = readb(xHL); HL--; break;

case 0x06: /* LD B,imm */
        B = FETCH; break;
case 0x0E: /* LD C,imm */
        C = FETCH; break;
case 0x16: /* LD D,imm */
        D = FETCH; break;
case 0x1E: /* LD E,imm */
        E = FETCH; break;
case 0x26: /* LD H,imm */
        H = FETCH; break;
case 0x2E: /* LD L,imm */
        L = FETCH; break;
case 0x36: /* LD (HL),imm */
        b = FETCH; writeb(xHL, b); break;
case 0x3E: /* LD A,imm */
        A = FETCH; break;

case 0x08: /* LD (imm),SP */
        writew(readw(xPC), SP); PC += 2; break;
case 0xEA: /* LD (imm),A */
        writeb(readw(xPC), A); PC += 2; break;

case 0xE0: /* LDH (imm),A */
        writehi(FETCH, A); break;
case 0xE2: /* LDH (C),A */
        writehi(C, A); break;
case 0xF0: /* LDH A,(imm) */
        A = readhi(FETCH); break;
case 0xF2: /* LDH A,(C) (undocumented) */
        A = readhi(C); break;


case 0xF8: /* LD HL,SP+imm */
        //b = FETCH; LDHLSP(b); break;
        LDHLSP; break;

case 0xF9: /* LD SP,HL */
        SP = HL; break;
case 0xFA: /* LD A,(imm) */
        A = readb(readw(xPC)); PC += 2; break;

        ALU_CASES(0x80, 0xC6, ADD)
        ALU_CASES(0x88, 0xCE, ADC)
        ALU_CASES(0x90, 0xD6, SUB)
        ALU_CASES(0x98, 0xDE, SBC)
        ALU_CASES(0xA0, 0xE6, AND)
        ALU_CASES(0xA8, 0xEE, XOR)
        ALU_CASES(0xB0, 0xF6, OR)
        ALU_CASES(0xB8, 0xFE, CP)

case 0x09: /* ADD HL,BC */
        ADDW(BC); break;
case 0x19: /* ADD HL,DE */
        ADDW(DE); break;
case 0x39: /* ADD HL,SP */
        ADDW(SP); break;
case 0x29: /* ADD HL,HL */
        ADDW(HL); break;

case 0x04: /* INC B */
        INC(B); break;
case 0x0C: /* INC C */
        INC(C); break;
case 0x14: /* INC D */
        INC(D); break;
case 0x1C: /* INC E */
        INC(E); break;
case 0x24: /* INC H */
        INC(H); break;
case 0x2C: /* INC L */
        INC(L); break;
case 0x34: /* INC (HL) */
        b = readb(xHL);
        INC(b);
        writeb(xHL, b);
        break;
case 0x3C: /* INC A */
        INC(A); break;

case 0x03: /* INC BC */
        INCW(BC); break;
case 0x13: /* INC DE */
        INCW(DE); break;
case 0x23: /* INC HL */
        INCW(HL); break;
case 0x33: /* INC SP */
        INCW(SP); break;

case 0x05: /* DEC B */
        DEC(B); break;
case 0x0D: /* DEC C */
        DEC(C); break;
case 0x15: /* DEC D */
        DEC(D); break;
case 0x1D: /* DEC E */
        DEC(E); break;
case 0x25: /* DEC H */
        DEC(H); break;
case 0x2D: /* DEC L */
        DEC(L); break;
case 0x35: /* DEC (HL) */
        b = readb(xHL);
        DEC(b);
        writeb(xHL, b);
        break;
case 0x3D: /* DEC A */
        DEC(A); break;

case 0x0B: /* DEC BC */
        DECW(BC); break;
case 0x1B: /* DEC DE */
        DECW(DE); break;
case 0x2B: /* DEC HL */
        DECW(HL); break;
case 0x3B: /* DEC SP */
        DECW(SP); break;

case 0x07: /* RLCA */
        RLCA; break;
case 0x0F: /* RRCA */
        RRCA; break;
case 0x17: /* RLA */
        RLA; break;
case 0x1F: /* RRA */
        RRA; break;

case 0x27: /* DAA */
        DAA; break;
case 0x2F: /* CPL */
        CPL(A); break;

case 0x18: /* JR */
        JR; break;
case 0x20: /* JR NZ */
        if (F&FZ) NOJR;
        else JR; break;
case 0x28: /* JR Z */
        if (F&FZ) JR;
        else NOJR; break;
case 0x30: /* JR NC */
        if (F&FC) NOJR;
        else JR; break;
case 0x38: /* JR C */
        if (F&FC) JR;
        else NOJR; break;

case 0xC3: /* JP */
        JP; break;
case 0xC2: /* JP NZ */
        if (F&FZ) NOJP;
        else JP; break;
case 0xCA: /* JP Z */
        if (F&FZ) JP;
        else NOJP; break;
case 0xD2: /* JP NC */
        if (F&FC) NOJP;
        else JP; break;
case 0xDA: /* JP C */
        if (F&FC) JP;
        else NOJP; break;
case 0xE9: /* JP HL */
        PC = HL; break;

case 0xC9: /* RET */
        RET; break;
case 0xC0: /* RET NZ */
        if (F&FZ) NORET;
        else RET; break;
case 0xC8: /* RET Z */
        if (F&FZ) RET;
        else NORET; break;
case 0xD0: /* RET NC */
        if (F&FC) NORET;
        else RET; break;
case 0xD8: /* RET C */
        if (F&FC) RET;
        else NORET; break;
case 0xD9: /* RETI */
        IME = 1;
        RET; break;

case 0xCD: /* CALL */
        CALL; break;
case 0xC4: /* CALL NZ */
        if (F&FZ) NOCALL;
        else CALL; break;
case 0xCC: /* CALL Z */
        if (F&FZ) CALL;
        else NOCALL; break;
case 0xD4: /* CALL NC */
        if (F&FC) NOCALL;
        else CALL; break;
case 0xDC: /* CALL C */
        if (F&FC) CALL;
        else NOCALL; break;

case 0xC7: /* RST 0 */
        RST(0x00); break;
case 0xCF: /* RST 8 */
        RST(0x08); break;
case 0xD7: /* RST 10 */
        RST(0x10); break;
case 0xDF: /* RST 18 */
        RST(0x18); break;
case 0xE7: /* RST 20 */
        RST(0x20); break;
case 0xEF: /* RST 28 */
        RST(0x28); break;
case 0xF7: /* RST 30 */
        RST(0x30); break;
case 0xFF: /* RST 38 */
        RST(0x38); break;

case 0xC1: /* POP BC */
        POP(BC); break;
case 0xC5: /* PUSH BC */
        PUSH(BC); break;
case 0xD1: /* POP DE */
        POP(DE); break;
case 0xD5: /* PUSH DE */
        PUSH(DE); break;
case 0xE1: /* POP HL */
        POP(HL); break;
case 0xE5: /* PUSH HL */
        PUSH(HL); break;
case 0xF1: /* POP AF */
        POP(W(acc));
        A = HB(acc);
        F = GBtoZ80[LB(acc) & 0xF0]; break;
        //POP(AF); break;
case 0xF5: /* PUSH AF */
        HB(acc) = A;
        LB(acc) = Z80toGB[F] | 0x0E;
        PUSH(W(acc)); break;
        //PUSH(AF); break;

case 0xE8: /* ADD SP,imm */
        //b = FETCH; ADDSP(b); break;
        ADDSP; break;

case 0xF3: /* DI */
        DI; break;
case 0xFB: /* EI */
        EI; break;

case 0x37: /* SCF */
        SCF; break;
case 0x3F: /* CCF */
        CCF; break;

case 0x10: /* STOP */
        PC++;
        if (R_KEY1 & 1)
        {
                cpu.speed = cpu.speed ^ 1;
                R_KEY1 = (R_KEY1 & 0x7E) | (cpu.speed << 7);
                break;
        }
        /* NOTE - we do not implement dmg STOP whatsoever */
        break;

case 0x76: /* HALT */
        cpu.halt = 1;
        break;

case 0xCB: /* CB prefix */
        cbop = FETCH;
        clen = cb_cycles_table[cbop];
        switch (cbop)
        {
                CB_REG_CASES(B, 0);
                CB_REG_CASES(C, 1);
                CB_REG_CASES(D, 2);
                CB_REG_CASES(E, 3);
                CB_REG_CASES(H, 4);
                CB_REG_CASES(L, 5);
                CB_REG_CASES(A, 7);
        default:
                {
                        byte c = readb(xHL);
                        switch(cbop)
                        {
                                CB_REG_CASES(c, 6);
                        }
                        if ((cbop & 0xC0) != 0x40) /* exclude BIT */
                                writeb(xHL, c);
                        break;
                }
        }
        break;
