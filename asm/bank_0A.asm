; bank $8A (file offset $050000)
org $8A8000

    incbin !baserom:$50000..$50001
    ldx.w #$007E                ; $8A8001
CODE_8A8004:
    stz.b $14,x                 ; $8A8004
    dex                         ; $8A8006
    dex                         ; $8A8007
    bpl CODE_8A8004             ; $8A8008
    rtl                         ; $8A800A
    incbin !baserom:$5000B..$50DEC
    sep.b #$20                  ; $8A8DEC
    rep.b #$10                  ; $8A8DEE
    ldx.w #$0000                ; $8A8DF0
    stx.w $2116                 ; $8A8DF3
    lda.b #$09                  ; $8A8DF6
    sta.w $4300                 ; $8A8DF8
    lda.b #$18                  ; $8A8DFB
    sta.w $4301                 ; $8A8DFD
    lda.b #$8A                  ; $8A8E00
    sta.w $4304                 ; $8A8E02
    ldx.w #$8E19                ; $8A8E05
    stx.w $4302                 ; $8A8E08
    ldx.w #$0000                ; $8A8E0B
    stx.w $4305                 ; $8A8E0E
    lda.b #$01                  ; $8A8E11
    sta.w $420B                 ; $8A8E13
    rep.b #$20                  ; $8A8E16
    rtl                         ; $8A8E18
    incbin !baserom:$50E19..$50E1B
    rep.b #$30                  ; $8A8E1B
    stz.w $0B00                 ; $8A8E1D
    stz.w $0B02                 ; $8A8E20
    stz.w $0B04                 ; $8A8E23
    stz.w $0B06                 ; $8A8E26
    stz.w $0B08                 ; $8A8E29
    stz.w $0B0A                 ; $8A8E2C
    stz.w $0B0C                 ; $8A8E2F
    stz.w $0B0E                 ; $8A8E32
    stz.w $0B10                 ; $8A8E35
    stz.w $0B12                 ; $8A8E38
    stz.w $0B14                 ; $8A8E3B
    stz.w $0B16                 ; $8A8E3E
    stz.w $0B18                 ; $8A8E41
    stz.w $0B1A                 ; $8A8E44
    stz.w $0B1C                 ; $8A8E47
    stz.w $0B1E                 ; $8A8E4A
    stz.b $28                   ; $8A8E4D
    stz.b $2A                   ; $8A8E4F
    lda.w #$0001                ; $8A8E51
    sta.b $2C                   ; $8A8E54
    lda.w #$0002                ; $8A8E56
    sta.b $2E                   ; $8A8E59
    ldx.w #$0000                ; $8A8E5B
CODE_8A8E5E:
    lda.w $0500,x               ; $8A8E5E
    bne CODE_8A8E86             ; $8A8E61
CODE_8A8E63:
    inx                         ; $8A8E63
    inx                         ; $8A8E64
    cpx.w #$0100                ; $8A8E65
    bne CODE_8A8E5E             ; $8A8E68
    lda.b $2A                   ; $8A8E6A
    cmp.b $30                   ; $8A8E6C
    beq CODE_8A8E81             ; $8A8E6E
    bpl CODE_8A8E81             ; $8A8E70
    tay                         ; $8A8E72
    lda.w #$E000                ; $8A8E73
CODE_8A8E76:
    sta.w $0900,y               ; $8A8E76
    iny                         ; $8A8E79
    iny                         ; $8A8E7A
    iny                         ; $8A8E7B
    iny                         ; $8A8E7C
    cpy.b $30                   ; $8A8E7D
    bne CODE_8A8E76             ; $8A8E7F
CODE_8A8E81:
    ldy.b $2A                   ; $8A8E81
    sty.b $30                   ; $8A8E83
    rtl                         ; $8A8E85
CODE_8A8E86:
    lda.w $0600,x               ; $8A8E86
    and.w #$01FF                ; $8A8E89
    sta.b $32                   ; $8A8E8C
    lda.w $0700,x               ; $8A8E8E
    bmi CODE_8A8E63             ; $8A8E91
    cmp.w #$00F0                ; $8A8E93
    bpl CODE_8A8E63             ; $8A8E96
    sta.b $26                   ; $8A8E98
    ldy.b $28                   ; $8A8E9A
    lda.b $32                   ; $8A8E9C
    and.w #$FF00                ; $8A8E9E
    beq CODE_8A8EAB             ; $8A8EA1
    incbin !baserom:$50EA3..$50EAB
CODE_8A8EAB:
    lda.w $0500,x               ; $8A8EAB
    bmi CODE_8A8EB8             ; $8A8EAE
    lda.w $0B00,y               ; $8A8EB0
    ora.b $2E                   ; $8A8EB3
    sta.w $0B00,y               ; $8A8EB5
CODE_8A8EB8:
    ldy.b $2A                   ; $8A8EB8
    lda.w $0800,x               ; $8A8EBA
    sta.w $0902,y               ; $8A8EBD
    sep.b #$20                  ; $8A8EC0
    lda.b $32                   ; $8A8EC2
    sta.w $0900,y               ; $8A8EC4
    lda.b $26                   ; $8A8EC7
    sta.w $0901,y               ; $8A8EC9
    rep.b #$20                  ; $8A8ECC
    iny                         ; $8A8ECE
    iny                         ; $8A8ECF
    iny                         ; $8A8ED0
    iny                         ; $8A8ED1
    sty.b $2A                   ; $8A8ED2
    asl.b $2E                   ; $8A8ED4
    rol.b $2E                   ; $8A8ED6
    asl.b $2C                   ; $8A8ED8
    rol.b $2C                   ; $8A8EDA
    bcc CODE_8A8EEC             ; $8A8EDC
    lda.w #$0001                ; $8A8EDE
    sta.b $2C                   ; $8A8EE1
    lda.w #$0002                ; $8A8EE3
    sta.b $2E                   ; $8A8EE6
    inc.b $28                   ; $8A8EE8
    inc.b $28                   ; $8A8EEA
CODE_8A8EEC:
    jmp.w $8E63                 ; $8A8EEC
    rep.b #$20                  ; $8A8EEF
    stz.b $30                   ; $8A8EF1
    ldx.w #$00FE                ; $8A8EF3
CODE_8A8EF6:
    stz.w $0500,x               ; $8A8EF6
    dex                         ; $8A8EF9
    dex                         ; $8A8EFA
    bpl CODE_8A8EF6             ; $8A8EFB
    jsl.l $8A8F02               ; $8A8EFD
    rtl                         ; $8A8F01
    rep.b #$30                  ; $8A8F02
    ldx.w #$0080                ; $8A8F04
    ldy.w #$0000                ; $8A8F07
    lda.w #$E000                ; $8A8F0A
CODE_8A8F0D:
    sta.w $0900,y               ; $8A8F0D
    iny                         ; $8A8F10
    iny                         ; $8A8F11
    iny                         ; $8A8F12
    iny                         ; $8A8F13
    dex                         ; $8A8F14
    bne CODE_8A8F0D             ; $8A8F15
    rtl                         ; $8A8F17
    incbin !baserom:$50F18..$519D0
    lda.w #$0028                ; $8A99D0
    sta.b $66                   ; $8A99D3
    rts                         ; $8A99D5
    incbin !baserom:$519D6..$51BB5
    phy                         ; $8A9BB5
    ldx.w #$0000                ; $8A9BB6
CODE_8A9BB9:
    lda.l $7F7FBC,x             ; $8A9BB9
    bne CODE_8A9BC7             ; $8A9BBD
    lda.b $68                   ; $8A9BBF
    cmp.l $81041C,x             ; $8A9BC1
    beq CODE_8A9BD5             ; $8A9BC5
CODE_8A9BC7:
    inx                         ; $8A9BC7
    inx                         ; $8A9BC8
    cpx.b $66                   ; $8A9BC9
    bne CODE_8A9BB9             ; $8A9BCB
    lda.b $68                   ; $8A9BCD
    beq CODE_8A9BDF             ; $8A9BCF
    incbin !baserom:$51BD1..$51BD5
CODE_8A9BD5:
    lda.w #$FFFF                ; $8A9BD5
    sta.l $7F7FBC,x             ; $8A9BD8
    ply                         ; $8A9BDC
    clc                         ; $8A9BDD
    rts                         ; $8A9BDE
CODE_8A9BDF:
    ply                         ; $8A9BDF
    sec                         ; $8A9BE0
    rts                         ; $8A9BE1
    stz.b $68                   ; $8A9BE2
    ldx.w #$0026                ; $8A9BE4
CODE_8A9BE7:
    lda.l $81041C,x             ; $8A9BE7
    cmp.b $68                   ; $8A9BEB
    bmi CODE_8A9BF1             ; $8A9BED
    sta.b $68                   ; $8A9BEF
CODE_8A9BF1:
    dex                         ; $8A9BF1
    dex                         ; $8A9BF2
    bpl CODE_8A9BE7             ; $8A9BF3
    rts                         ; $8A9BF5
    incbin !baserom:$51BF6..$51CB0
    jsr.w $9CF8                 ; $8A9CB0
    lda.l $8103DC               ; $8A9CB3
    bit.w #$000F                ; $8A9CB7
    beq CODE_8A9CC6             ; $8A9CBA
    incbin !baserom:$51CBC..$51CC6
CODE_8A9CC6:
    ldx.w #$0000                ; $8A9CC6
    lda.w $0D00,x               ; $8A9CC9
    beq CODE_8A9CF3             ; $8A9CCC
    incbin !baserom:$51CCE..$51CF3
CODE_8A9CF3:
    jsr.w $9D1E                 ; $8A9CF3
    clc                         ; $8A9CF6
    rtl                         ; $8A9CF7
    ldx.w #$0026                ; $8A9CF8
    lda.w #$0000                ; $8A9CFB
CODE_8A9CFE:
    sta.l $7F7FBC,x             ; $8A9CFE
    dex                         ; $8A9D02
    dex                         ; $8A9D03
    bpl CODE_8A9CFE             ; $8A9D04
    jsr.w $9BE2                 ; $8A9D06
    jsr.w $99D0                 ; $8A9D09
    ldy.w #$0000                ; $8A9D0C
CODE_8A9D0F:
    jsr.w $9BB5                 ; $8A9D0F
    bcs CODE_8A9D1D             ; $8A9D12
    txa                         ; $8A9D14
    lsr a                       ; $8A9D15
    sta.w $0D00,y               ; $8A9D16
    iny                         ; $8A9D19
    iny                         ; $8A9D1A
    bra CODE_8A9D0F             ; $8A9D1B
CODE_8A9D1D:
    rts                         ; $8A9D1D
    ldx.w #$0026                ; $8A9D1E
    lda.w #$0000                ; $8A9D21
CODE_8A9D24:
    sta.l $81041C,x             ; $8A9D24
    dex                         ; $8A9D28
    dex                         ; $8A9D29
    bpl CODE_8A9D24             ; $8A9D2A
    rts                         ; $8A9D2C
    incbin !baserom:$51D2D..$51E5B
    phx                         ; $8A9E5B
    ldx.w #$00FE                ; $8A9E5C
CODE_8A9E5F:
    stz.w $0D00,x               ; $8A9E5F
    dex                         ; $8A9E62
    dex                         ; $8A9E63
    bpl CODE_8A9E5F             ; $8A9E64
    plx                         ; $8A9E66
    cpx.w #$0000                ; $8A9E67
    beq CODE_8A9E82             ; $8A9E6A
    cpx.w #$0002                ; $8A9E6C
    beq CODE_8A9E7A             ; $8A9E6F
    incbin !baserom:$51E71..$51E7A
CODE_8A9E7A:
    jsr.w $C056                 ; $8A9E7A
    rtl                         ; $8A9E7D
    incbin !baserom:$51E7E..$51E82
CODE_8A9E82:
    lda.w $0008                 ; $8A9E82
    pha                         ; $8A9E85
    stz.w $0196                 ; $8A9E86
    stz.w $0198                 ; $8A9E89
    stz.w $019A                 ; $8A9E8C
    jsl.l $8A8001               ; $8A9E8F
    ldx.w #$00FE                ; $8A9E93
CODE_8A9E96:
    stz.w $0D00,x               ; $8A9E96
    dex                         ; $8A9E99
    dex                         ; $8A9E9A
    bpl CODE_8A9E96             ; $8A9E9B
    lda.w $03DC                 ; $8A9E9D
    and.w #$DFFF                ; $8A9EA0
    sta.w $03DC                 ; $8A9EA3
    stz.b $8E                   ; $8A9EA6
    ldx.w #$C4B0                ; $8A9EA8
    lda.w #$008A                ; $8A9EAB
    jsl.l $8081C8               ; $8A9EAE
    jsr.w $A06D                 ; $8A9EB2
    jsl.l $8A8DEC               ; $8A9EB5
    jsr.w $BA55                 ; $8A9EB9
    jsr.w $B994                 ; $8A9EBC
    jsr.w $BACB                 ; $8A9EBF
    jsr.w $B91A                 ; $8A9EC2
    jsl.l $8A8EEF               ; $8A9EC5
    lda.w #$FC5F                ; $8A9EC9
    sta.w $0008                 ; $8A9ECC
    sep.b #$20                  ; $8A9ECF
    lda.b #$82                  ; $8A9ED1
    sta.w $420C                 ; $8A9ED3
    lda.b #$81                  ; $8A9ED6
    sta.w $4200                 ; $8A9ED8
    rep.b #$20                  ; $8A9EDB
    stz.b $3A                   ; $8A9EDD
    stz.b $3C                   ; $8A9EDF
    stz.w $0D88,x               ; $8A9EE1
    lda.w #$0004                ; $8A9EE4
    sta.b $7A                   ; $8A9EE7
    sta.b $7C                   ; $8A9EE9
    lda.w #$0007                ; $8A9EEB
    sta.b $7E                   ; $8A9EEE
    inc.b $8E                   ; $8A9EF0
    lda.w #$0F00                ; $8A9EF2
    sta.w $0198                 ; $8A9EF5
    lda.w #$0080                ; $8A9EF8
    sta.w $0196                 ; $8A9EFB
    ldx.w #$000A                ; $8A9EFE
    ldy.w #$0010                ; $8A9F01
    stx.b $3A                   ; $8A9F04
    sty.b $3C                   ; $8A9F06
    ldx.w #$0000                ; $8A9F08
    ldy.w #$001F                ; $8A9F0B
    stx.b $3E                   ; $8A9F0E
    sty.b $40                   ; $8A9F10
    lda.w #$007C                ; $8A9F12
    sta.w $0D60                 ; $8A9F15
    lda.w #$007C                ; $8A9F18
    sta.w $0D68                 ; $8A9F1B
    lda.w #$0040                ; $8A9F1E
    sta.w $0D78                 ; $8A9F21
    lda.w #$0008                ; $8A9F24
    sta.b $4C                   ; $8A9F27
    lda.w #$CAB0                ; $8A9F29
    jsr.w $A4BE                 ; $8A9F2C
    stz.b $6C                   ; $8A9F2F
    jsr.w $B0E1                 ; $8A9F31
    lda.w #$0A8C                ; $8A9F34
    sta.w $0D7E                 ; $8A9F37
CODE_8A9F3A:
    lda.w #$0001                ; $8A9F3A
    jsl.l $8ABA90               ; $8A9F3D
    lda.w #$CBA7                ; $8A9F41
    jsr.w $A2AF                 ; $8A9F44
    jsr.w $9F76                 ; $8A9F47
    lda.b $6C                   ; $8A9F4A
    beq CODE_8A9F3A             ; $8A9F4C
    jsr.w $9F64                 ; $8A9F4E
    stz.w $0198                 ; $8A9F51
    lda.w #$FFC0                ; $8A9F54
    sta.w $0196                 ; $8A9F57
CODE_8A9F5A:
    lda.w $019A                 ; $8A9F5A
    bne CODE_8A9F5A             ; $8A9F5D
    pla                         ; $8A9F5F
    sta.w $0008                 ; $8A9F60
    rtl                         ; $8A9F63
    lda.b $52                   ; $8A9F64
    beq CODE_8A9F6F             ; $8A9F66
    incbin !baserom:$51F68..$51F6F
CODE_8A9F6F:
    lda.w #$1000                ; $8A9F6F
    trb.w $03DC                 ; $8A9F72
    rts                         ; $8A9F75
    dec.w $0D7E                 ; $8A9F76
    bne CODE_8A9FA0             ; $8A9F79
    incbin !baserom:$51F7B..$51FA0
CODE_8A9FA0:
    rts                         ; $8A9FA0
    phx                         ; $8A9FA1
    phy                         ; $8A9FA2
    ldx.w #$0000                ; $8A9FA3
    lda.b $58                   ; $8A9FA6
    asl a                       ; $8A9FA8
    tay                         ; $8A9FA9
    lda.w $9FF9,y               ; $8A9FAA
CODE_8A9FAD:
    lsr a                       ; $8A9FAD
    bcc CODE_8A9FCA             ; $8A9FAE
    pha                         ; $8A9FB0
    phx                         ; $8A9FB1
    lda.w $9FE9,x               ; $8A9FB2
    cmp.w #$FFFF                ; $8A9FB5
    beq CODE_8A9FE4             ; $8A9FB8
    tax                         ; $8A9FBA
    lda.w $1516,x               ; $8A9FBB
    and.w #$FF7F                ; $8A9FBE
    sta.w $1516,x               ; $8A9FC1
    plx                         ; $8A9FC4
    pla                         ; $8A9FC5
    inx                         ; $8A9FC6
    inx                         ; $8A9FC7
    bra CODE_8A9FAD             ; $8A9FC8
CODE_8A9FCA:
    pha                         ; $8A9FCA
    phx                         ; $8A9FCB
    lda.w $9FE9,x               ; $8A9FCC
    cmp.w #$FFFF                ; $8A9FCF
    beq CODE_8A9FE4             ; $8A9FD2
    tax                         ; $8A9FD4
    lda.w $1516,x               ; $8A9FD5
    ora.w #$0080                ; $8A9FD8
    sta.w $1516,x               ; $8A9FDB
    plx                         ; $8A9FDE
    pla                         ; $8A9FDF
    inx                         ; $8A9FE0
    inx                         ; $8A9FE1
    bra CODE_8A9FAD             ; $8A9FE2
CODE_8A9FE4:
    ply                         ; $8A9FE4
    plx                         ; $8A9FE5
    ply                         ; $8A9FE6
    plx                         ; $8A9FE7
    rts                         ; $8A9FE8
    incbin !baserom:$51FE9..$52005
    stz.b $56                   ; $8AA005
    stz.b $58                   ; $8AA007
    lda.w $0018                 ; $8AA009
    jsr.w $A022                 ; $8AA00C
    lda.w $0014                 ; $8AA00F
    jsr.w $A022                 ; $8AA012
    lda.w $0010                 ; $8AA015
    jsr.w $A022                 ; $8AA018
    lda.w $000C                 ; $8AA01B
    jsr.w $A022                 ; $8AA01E
    rts                         ; $8AA021
    eor.w #$0001                ; $8AA022
    and.w #$0001                ; $8AA025
    bne CODE_8AA033             ; $8AA028
    clc                         ; $8AA02A
    rol.b $56                   ; $8AA02B
    clc                         ; $8AA02D
    adc.b $58                   ; $8AA02E
    sta.b $58                   ; $8AA030
    rts                         ; $8AA032
CODE_8AA033:
    sec                         ; $8AA033
    rol.b $56                   ; $8AA034
    clc                         ; $8AA036
    adc.b $58                   ; $8AA037
    sta.b $58                   ; $8AA039
    rts                         ; $8AA03B
    incbin !baserom:$5203C..$52050
    lda.b $8E                   ; $8AA050
    beq CODE_8AA069             ; $8AA052
    jsr.w $BA7D                 ; $8AA054
    jsl.l $8A8E1B               ; $8AA057
    inc.w $0164                 ; $8AA05B
    lda.w $0164                 ; $8AA05E
    and.w #$0007                ; $8AA061
    bne CODE_8AA069             ; $8AA064
    inc.w $0166                 ; $8AA066
CODE_8AA069:
    jsr.w $A005                 ; $8AA069
    rtl                         ; $8AA06C
    jsr.w $A09F                 ; $8AA06D
    sep.b #$10                  ; $8AA070
    ldx.b #$0F                  ; $8AA072
    stx.w $4371                 ; $8AA074
    ldx.b #$02                  ; $8AA077
    stx.w $4370                 ; $8AA079
    lda.w #$0D00                ; $8AA07C
    sta.w $4372                 ; $8AA07F
    ldx.b #$81                  ; $8AA082
    stx.w $4374                 ; $8AA084
    ldx.b #$32                  ; $8AA087
    stx.w $4311                 ; $8AA089
    ldx.b #$00                  ; $8AA08C
    stx.w $4310                 ; $8AA08E
    lda.w #$0D20                ; $8AA091
    sta.w $4312                 ; $8AA094
    ldx.b #$81                  ; $8AA097
    stx.w $4314                 ; $8AA099
    rep.b #$10                  ; $8AA09C
    rts                         ; $8AA09E
    sep.b #$20                  ; $8AA09F
    lda.b #$4C                  ; $8AA0A1
    sta.w $0D00                 ; $8AA0A3
    lda.b #$18                  ; $8AA0A6
    sta.w $0D03                 ; $8AA0A8
    sta.w $0D06                 ; $8AA0AB
    sta.w $0D09                 ; $8AA0AE
    sta.w $0D0C                 ; $8AA0B1
    sta.w $0D0F                 ; $8AA0B4
    sta.w $0D12                 ; $8AA0B7
    sta.w $0D15                 ; $8AA0BA
    stz.w $0D18                 ; $8AA0BD
    lda.b #$4C                  ; $8AA0C0
    sta.w $0D20                 ; $8AA0C2
    lda.b #$18                  ; $8AA0C5
    sta.w $0D22                 ; $8AA0C7
    sta.w $0D24                 ; $8AA0CA
    sta.w $0D26                 ; $8AA0CD
    sta.w $0D28                 ; $8AA0D0
    sta.w $0D2A                 ; $8AA0D3
    sta.w $0D2C                 ; $8AA0D6
    sta.w $0D2E                 ; $8AA0D9
    stz.w $0D30                 ; $8AA0DC
    lda.b #$E0                  ; $8AA0DF
    sta.w $0D23                 ; $8AA0E1
    sta.w $0D25                 ; $8AA0E4
    sta.w $0D27                 ; $8AA0E7
    sta.w $0D29                 ; $8AA0EA
    sta.w $0D2B                 ; $8AA0ED
    sta.w $0D2D                 ; $8AA0F0
    sta.w $0D2F                 ; $8AA0F3
    rep.b #$20                  ; $8AA0F6
    rts                         ; $8AA0F8
    phx                         ; $8AA0F9
    ldy.b $7E                   ; $8AA0FA
    jsr.w $A27D                 ; $8AA0FC
    ldy.b $7C                   ; $8AA0FF
    jsr.w $A296                 ; $8AA101
    ldy.b $7A                   ; $8AA104
    jsr.w $A264                 ; $8AA106
    lda.w $0D60,x               ; $8AA109
    pha                         ; $8AA10C
    lda.w $0D68,x               ; $8AA10D
    pha                         ; $8AA110
    lda.w $0D70,x               ; $8AA111
    pha                         ; $8AA114
    lda.w $0D70,x               ; $8AA115
    lsr a                       ; $8AA118
    sta.b $48                   ; $8AA119
    lda.w $0D60,x               ; $8AA11B
    sec                         ; $8AA11E
    sbc.b $48                   ; $8AA11F
    clc                         ; $8AA121
    adc.w $0D80,x               ; $8AA122
    sta.w $0D60,x               ; $8AA125
    jsr.w $A25D                 ; $8AA128
    lda.w #$FFFF                ; $8AA12B
    sta.w $0500,y               ; $8AA12E
    lda.w #$3000                ; $8AA131
    sta.w $0800,y               ; $8AA134
    lda.w $0D60,x               ; $8AA137
    sta.w $0600,y               ; $8AA13A
    lda.w $0D68,x               ; $8AA13D
    sta.w $0700,y               ; $8AA140
    jsr.w $A25D                 ; $8AA143
    lda.w #$FFFF                ; $8AA146
    sta.w $0500,y               ; $8AA149
    lda.w #$3001                ; $8AA14C
    sta.w $0800,y               ; $8AA14F
    lda.w $0D60,x               ; $8AA152
    sta.w $0600,y               ; $8AA155
    lda.w $0D68,x               ; $8AA158
    clc                         ; $8AA15B
    adc.b $4C                   ; $8AA15C
    sta.w $0700,y               ; $8AA15E
    jsr.w $A25D                 ; $8AA161
    lda.w #$FFFF                ; $8AA164
    sta.w $0500,y               ; $8AA167
    lda.w #$B000                ; $8AA16A
    sta.w $0800,y               ; $8AA16D
    lda.w $0D60,x               ; $8AA170
    sta.w $0600,y               ; $8AA173
    lda.w $0D68,x               ; $8AA176
    clc                         ; $8AA179
    adc.b $4C                   ; $8AA17A
    clc                         ; $8AA17C
    adc.b $4C                   ; $8AA17D
    sta.w $0700,y               ; $8AA17F
    lda.w $0D60,x               ; $8AA182
    clc                         ; $8AA185
    adc.w #$0008                ; $8AA186
    sta.w $0D60,x               ; $8AA189
    ldy.w #$0000                ; $8AA18C
CODE_8AA18F:
    sec                         ; $8AA18F
    lda.w $0D70,x               ; $8AA190
    sbc.w #$0008                ; $8AA193
    bcc CODE_8AA1A0             ; $8AA196
    sta.w $0D70,x               ; $8AA198
    jsr.w $A216                 ; $8AA19B
    bra CODE_8AA18F             ; $8AA19E
CODE_8AA1A0:
    adc.w #$0008                ; $8AA1A0
    clc                         ; $8AA1A3
    adc.w $0D60,x               ; $8AA1A4
    sec                         ; $8AA1A7
    sbc.w #$0008                ; $8AA1A8
    sta.w $0D60,x               ; $8AA1AB
    jsr.w $A25D                 ; $8AA1AE
    lda.w #$FFFF                ; $8AA1B1
    sta.w $0500,y               ; $8AA1B4
    lda.w #$7000                ; $8AA1B7
    sta.w $0800,y               ; $8AA1BA
    lda.w $0D60,x               ; $8AA1BD
    sta.w $0600,y               ; $8AA1C0
    lda.w $0D68,x               ; $8AA1C3
    sta.w $0700,y               ; $8AA1C6
    jsr.w $A25D                 ; $8AA1C9
    lda.w #$FFFF                ; $8AA1CC
    sta.w $0500,y               ; $8AA1CF
    lda.w #$7001                ; $8AA1D2
    sta.w $0800,y               ; $8AA1D5
    lda.w $0D60,x               ; $8AA1D8
    sta.w $0600,y               ; $8AA1DB
    lda.w $0D68,x               ; $8AA1DE
    clc                         ; $8AA1E1
    adc.b $4C                   ; $8AA1E2
    sta.w $0700,y               ; $8AA1E4
    jsr.w $A25D                 ; $8AA1E7
    lda.w #$FFFF                ; $8AA1EA
    sta.w $0500,y               ; $8AA1ED
    lda.w #$F000                ; $8AA1F0
    sta.w $0800,y               ; $8AA1F3
    lda.w $0D60,x               ; $8AA1F6
    sta.w $0600,y               ; $8AA1F9
    lda.w $0D68,x               ; $8AA1FC
    clc                         ; $8AA1FF
    adc.b $4C                   ; $8AA200
    clc                         ; $8AA202
    adc.b $4C                   ; $8AA203
    sta.w $0700,y               ; $8AA205
    pla                         ; $8AA208
    sta.w $0D70,x               ; $8AA209
    pla                         ; $8AA20C
    sta.w $0D68,x               ; $8AA20D
    pla                         ; $8AA210
    sta.w $0D60,x               ; $8AA211
    plx                         ; $8AA214
    rts                         ; $8AA215
    jsr.w $A25D                 ; $8AA216
    lda.w #$FFFF                ; $8AA219
    sta.w $0500,y               ; $8AA21C
    lda.w $0D60,x               ; $8AA21F
    sta.w $0600,y               ; $8AA222
    lda.w $0D68,x               ; $8AA225
    sta.w $0700,y               ; $8AA228
    lda.w #$3002                ; $8AA22B
    sta.w $0800,y               ; $8AA22E
    jsr.w $A25D                 ; $8AA231
    lda.w #$FFFF                ; $8AA234
    sta.w $0500,y               ; $8AA237
    lda.w $0D60,x               ; $8AA23A
    sta.w $0600,y               ; $8AA23D
    lda.w $0D68,x               ; $8AA240
    clc                         ; $8AA243
    adc.b $4C                   ; $8AA244
    clc                         ; $8AA246
    adc.b $4C                   ; $8AA247
    sta.w $0700,y               ; $8AA249
    lda.w #$B002                ; $8AA24C
    sta.w $0800,y               ; $8AA24F
    clc                         ; $8AA252
    lda.w $0D60,x               ; $8AA253
    adc.w #$0008                ; $8AA256
    sta.w $0D60,x               ; $8AA259
    rts                         ; $8AA25C
    ldy.b $8C                   ; $8AA25D
    inc.b $8C                   ; $8AA25F
    inc.b $8C                   ; $8AA261
    rts                         ; $8AA263
    lda.w $0D88,x               ; $8AA264
    sec                         ; $8AA267
    sbc.w $0D80,x               ; $8AA268
    beq CODE_8AA27C             ; $8AA26B
    incbin !baserom:$5226D..$5227C
CODE_8AA27C:
    rts                         ; $8AA27C
CODE_8AA27D:
    lda.w $0D78,x               ; $8AA27D
    sec                         ; $8AA280
    sbc.w $0D70,x               ; $8AA281
    beq CODE_8AA295             ; $8AA284
    bmi CODE_8AA28F             ; $8AA286
    inc.w $0D70,x               ; $8AA288
    dey                         ; $8AA28B
    bne CODE_8AA27D             ; $8AA28C
    rts                         ; $8AA28E
CODE_8AA28F:
    dec.w $0D70,x               ; $8AA28F
    dey                         ; $8AA292
    bne CODE_8AA27D             ; $8AA293
CODE_8AA295:
    rts                         ; $8AA295
CODE_8AA296:
    lda.w $0D90,x               ; $8AA296
    sec                         ; $8AA299
    sbc.w $0D68,x               ; $8AA29A
    beq CODE_8AA2AE             ; $8AA29D
    bmi CODE_8AA2A8             ; $8AA29F
    inc.w $0D68,x               ; $8AA2A1
    dey                         ; $8AA2A4
    bne CODE_8AA296             ; $8AA2A5
    rts                         ; $8AA2A7
CODE_8AA2A8:
    dec.w $0D68,x               ; $8AA2A8
    dey                         ; $8AA2AB
    bne CODE_8AA296             ; $8AA2AC
CODE_8AA2AE:
    rts                         ; $8AA2AE
    sta.b $4A                   ; $8AA2AF
    jsr.w $A47E                 ; $8AA2B1
    ldx.w #$0000                ; $8AA2B4
    jsr.w $A0F9                 ; $8AA2B7
    ldy.w #$0000                ; $8AA2BA
    lda.b ($4A),y               ; $8AA2BD
    sta.b $3C                   ; $8AA2BF
    inc.b $4A                   ; $8AA2C1
    inc.b $4A                   ; $8AA2C3
    inc.b $4A                   ; $8AA2C5
    inc.b $4A                   ; $8AA2C7
    ldy.w #$0004                ; $8AA2C9
    lda.b ($4A),y               ; $8AA2CC
    tax                         ; $8AA2CE
    ldy.w #$0000                ; $8AA2CF
    lda.b ($4A),y               ; $8AA2D2
    beq CODE_8AA32A             ; $8AA2D4
    jsr.w $A32B                 ; $8AA2D6
    lda.w $1516,x               ; $8AA2D9
    and.w #$0002                ; $8AA2DC
    beq CODE_8AA2F7             ; $8AA2DF
    incbin !baserom:$522E1..$522F7
CODE_8AA2F7:
    stz.b $68                   ; $8AA2F7
    lda.b ($4A),y               ; $8AA2F9
    jsr.w $A4CF                 ; $8AA2FB
    jsr.w $B6AA                 ; $8AA2FE
    jsr.w $A37A                 ; $8AA301
    lda.w $1516,x               ; $8AA304
    and.w #$0001                ; $8AA307
    beq CODE_8AA31F             ; $8AA30A
    jsr.w $A469                 ; $8AA30C
    jsr.w $A381                 ; $8AA30F
    lda.w $1516,x               ; $8AA312
    and.w #$0080                ; $8AA315
    bne CODE_8AA31F             ; $8AA318
    jsr.w $A419                 ; $8AA31A
    bcs CODE_8AA32A             ; $8AA31D
CODE_8AA31F:
    clc                         ; $8AA31F
    lda.b $4A                   ; $8AA320
    adc.w #$0008                ; $8AA322
    sta.b $4A                   ; $8AA325
    jmp.w $A2C9                 ; $8AA327
CODE_8AA32A:
    rts                         ; $8AA32A
    phy                         ; $8AA32B
    lda.w #$0000                ; $8AA32C
    sep.b #$20                  ; $8AA32F
    ldy.w #$0000                ; $8AA331
    lda.b $3C                   ; $8AA334
    sec                         ; $8AA336
    sbc.b #$0A                  ; $8AA337
    tay                         ; $8AA339
    lda.w $C599,y               ; $8AA33A
    tay                         ; $8AA33D
    lda.w $1516,x               ; $8AA33E
    and.b #$80                  ; $8AA341
    beq CODE_8AA34C             ; $8AA343
    incbin !baserom:$52345..$5234C
CODE_8AA34C:
    lda.w $1516,x               ; $8AA34C
    and.b #$40                  ; $8AA34F
    bne CODE_8AA35A             ; $8AA351
    lda.b #$E7                  ; $8AA353
    sta.w $0D23,y               ; $8AA355
    bra CODE_8AA376             ; $8AA358
CODE_8AA35A:
    phy                         ; $8AA35A
    lda.b $6E                   ; $8AA35B
    and.b #$1F                  ; $8AA35D
    tay                         ; $8AA35F
    lda.w $C5B7,y               ; $8AA360
    ply                         ; $8AA363
    pha                         ; $8AA364
    lda.w $0D23,y               ; $8AA365
    and.b #$E0                  ; $8AA368
    sta.w $0D23,y               ; $8AA36A
    pla                         ; $8AA36D
    ora.w $0D23,y               ; $8AA36E
    sta.w $0D23,y               ; $8AA371
    inc.b $6E                   ; $8AA374
CODE_8AA376:
    rep.b #$20                  ; $8AA376
    ply                         ; $8AA378
    rts                         ; $8AA379
    inc.b $3C                   ; $8AA37A
    inc.b $3C                   ; $8AA37C
    inc.b $3C                   ; $8AA37E
    rts                         ; $8AA380
    lda.w #$0800                ; $8AA381
    trb.w $000E                 ; $8AA384
    db $D0,$39 ; bne            ; $8AA387
    lda.w #$0800                ; $8AA389
    trb.w $0012                 ; $8AA38C
    db $D0,$31 ; bne            ; $8AA38F
    lda.w #$0800                ; $8AA391
    trb.w $0016                 ; $8AA394
    db $D0,$29 ; bne            ; $8AA397
    lda.w #$0800                ; $8AA399
    trb.w $001A                 ; $8AA39C
    db $D0,$21 ; bne            ; $8AA39F
    lda.w #$0400                ; $8AA3A1
    trb.w $000E                 ; $8AA3A4
    bne CODE_8AA3F4             ; $8AA3A7
    lda.w #$0400                ; $8AA3A9
    trb.w $0012                 ; $8AA3AC
    bne CODE_8AA3F4             ; $8AA3AF
    lda.w #$0400                ; $8AA3B1
    trb.w $0016                 ; $8AA3B4
    bne CODE_8AA3F4             ; $8AA3B7
    lda.w #$0400                ; $8AA3B9
    trb.w $001A                 ; $8AA3BC
    bne CODE_8AA3F4             ; $8AA3BF
    rts                         ; $8AA3C1
    incbin !baserom:$523C2..$523F4
CODE_8AA3F4:
    phy                         ; $8AA3F4
    ldy.w #$0008                ; $8AA3F5
    lda.b ($4A),y               ; $8AA3F8
    beq CODE_8AA417             ; $8AA3FA
    lda.w $1516,x               ; $8AA3FC
    and.w #$FFBE                ; $8AA3FF
    sta.w $1516,x               ; $8AA402
    lda.w $1518,x               ; $8AA405
    ora.w #$0041                ; $8AA408
    sta.w $1518,x               ; $8AA40B
    stz.b $64                   ; $8AA40E
    stz.b $66                   ; $8AA410
    stz.b $6E                   ; $8AA412
    jsr.w $A48D                 ; $8AA414
CODE_8AA417:
    ply                         ; $8AA417
    rts                         ; $8AA418
    lda.w $0D90                 ; $8AA419
    cmp.w $0D68                 ; $8AA41C
    beq CODE_8AA423             ; $8AA41F
    clc                         ; $8AA421
    rts                         ; $8AA422
CODE_8AA423:
    lda.w #$C0C0                ; $8AA423
    trb.w $000E                 ; $8AA426
    bne CODE_8AA445             ; $8AA429
    lda.w #$C0C0                ; $8AA42B
    trb.w $0012                 ; $8AA42E
    bne CODE_8AA445             ; $8AA431
    lda.w #$C0C0                ; $8AA433
    trb.w $0016                 ; $8AA436
    bne CODE_8AA445             ; $8AA439
    lda.w #$C0C0                ; $8AA43B
    trb.w $001A                 ; $8AA43E
    bne CODE_8AA445             ; $8AA441
    clc                         ; $8AA443
    rts                         ; $8AA444
CODE_8AA445:
    lda.b $62                   ; $8AA445
    pha                         ; $8AA447
    lda.b $4A                   ; $8AA448
    pha                         ; $8AA44A
    phx                         ; $8AA44B
    phy                         ; $8AA44C
    stz.b $5E                   ; $8AA44D
    ldy.w #$0002                ; $8AA44F
    lda.b ($4A),y               ; $8AA452
    sta.b $5C                   ; $8AA454
    jsr.w $A49E                 ; $8AA456
    jsr.w $A466                 ; $8AA459
    ply                         ; $8AA45C
    plx                         ; $8AA45D
    pla                         ; $8AA45E
    sta.b $4A                   ; $8AA45F
    pla                         ; $8AA461
    sta.b $62                   ; $8AA462
    sec                         ; $8AA464
    rts                         ; $8AA465
    jmp.w ($015C)               ; $8AA466
    lda.b $3C                   ; $8AA469
    asl a                       ; $8AA46B
    asl a                       ; $8AA46C
    asl a                       ; $8AA46D
    sec                         ; $8AA46E
    sbc.w #$001C                ; $8AA46F
    sta.w $0D90                 ; $8AA472
    lda.b $5A                   ; $8AA475
    asl a                       ; $8AA477
    asl a                       ; $8AA478
    asl a                       ; $8AA479
    sta.w $0D78                 ; $8AA47A
    rts                         ; $8AA47D
    phx                         ; $8AA47E
    stz.b $8C                   ; $8AA47F
    ldx.w #$00F6                ; $8AA481
CODE_8AA484:
    stz.w $0500,x               ; $8AA484
    dex                         ; $8AA487
    dex                         ; $8AA488
    bpl CODE_8AA484             ; $8AA489
    plx                         ; $8AA48B
    rts                         ; $8AA48C
    pha                         ; $8AA48D
    sep.b #$20                  ; $8AA48E
    lda.b #$11                  ; $8AA490
    sta.b $FB                   ; $8AA492
    rep.b #$20                  ; $8AA494
    lda.w #$0A8C                ; $8AA496
    sta.w $0D7E                 ; $8AA499
    pla                         ; $8AA49C
    rts                         ; $8AA49D
    pha                         ; $8AA49E
    sep.b #$20                  ; $8AA49F
    lda.b #$13                  ; $8AA4A1
    sta.b $FB                   ; $8AA4A3
    rep.b #$20                  ; $8AA4A5
    lda.w #$0A8C                ; $8AA4A7
    sta.w $0D7E                 ; $8AA4AA
    pla                         ; $8AA4AD
    rts                         ; $8AA4AE
    incbin !baserom:$524AF..$524BE
    sta.b $4A                   ; $8AA4BE
    ldy.w #$0000                ; $8AA4C0
CODE_8AA4C3:
    lda.b ($4A),y               ; $8AA4C3
    beq CODE_8AA4CE             ; $8AA4C5
    jsr.w $B680                 ; $8AA4C7
    iny                         ; $8AA4CA
    iny                         ; $8AA4CB
    bra CODE_8AA4C3             ; $8AA4CC
CODE_8AA4CE:
    rts                         ; $8AA4CE
    sta.b $44                   ; $8AA4CF
    phx                         ; $8AA4D1
    phy                         ; $8AA4D2
    lda.w $1516,x               ; $8AA4D3
    and.w #$003C                ; $8AA4D6
    lsr a                       ; $8AA4D9
    lsr a                       ; $8AA4DA
    tax                         ; $8AA4DB
    beq CODE_8AA4F6             ; $8AA4DC
    sep.b #$20                  ; $8AA4DE
    ldy.w #$0000                ; $8AA4E0
CODE_8AA4E3:
    lda.b ($44),y               ; $8AA4E3
    beq CODE_8AA4EF             ; $8AA4E5
    rep.b #$20                  ; $8AA4E7
    inc.b $44                   ; $8AA4E9
    sep.b #$20                  ; $8AA4EB
    bra CODE_8AA4E3             ; $8AA4ED
CODE_8AA4EF:
    inc.b $44                   ; $8AA4EF
    dex                         ; $8AA4F1
    bne CODE_8AA4E3             ; $8AA4F2
    rep.b #$20                  ; $8AA4F4
CODE_8AA4F6:
    lda.b $44                   ; $8AA4F6
    ply                         ; $8AA4F8
    plx                         ; $8AA4F9
    rts                         ; $8AA4FA
    lda.w #$CBD2                ; $8AA4FB
    jsr.w $B659                 ; $8AA4FE
    rts                         ; $8AA501
    incbin !baserom:$52502..$5308C
    lda.w $1526                 ; $8AB08C
    lsr a                       ; $8AB08F
    lsr a                       ; $8AB090
    and.w #$000F                ; $8AB091
    cmp.w #$0002                ; $8AB094
    bne CODE_8AB09C             ; $8AB097
    lda.w #$FFFF                ; $8AB099
CODE_8AB09C:
    inc a                       ; $8AB09C
    asl a                       ; $8AB09D
    asl a                       ; $8AB09E
    sta.b $6A                   ; $8AB09F
    lda.w $1526                 ; $8AB0A1
    and.w #$FFC3                ; $8AB0A4
    ora.b $6A                   ; $8AB0A7
    sta.w $1526                 ; $8AB0A9
    rts                         ; $8AB0AC
    incbin !baserom:$530AD..$530D3
    lda.w #$CC83                ; $8AB0D3
    jsr.w $B659                 ; $8AB0D6
    incbin !baserom:$530D9..$530E1
    lda.w $1528                 ; $8AB0E1
    and.w #$FFC3                ; $8AB0E4
    sta.w $1528                 ; $8AB0E7
    lda.l $8103DC               ; $8AB0EA
    and.w #$0400                ; $8AB0EE
    ldy.w #$0008                ; $8AB0F1
    jsr.w $B15E                 ; $8AB0F4
    ora.w $1528                 ; $8AB0F7
    sta.w $1528                 ; $8AB0FA
    lda.w $1526                 ; $8AB0FD
    and.w #$FFC3                ; $8AB100
    sta.w $1526                 ; $8AB103
    lda.l $8103DC               ; $8AB106
    and.w #$0300                ; $8AB10A
    ldy.w #$0006                ; $8AB10D
    jsr.w $B15E                 ; $8AB110
    ora.w $1526                 ; $8AB113
    sta.w $1526                 ; $8AB116
    rts                         ; $8AB119
    incbin !baserom:$5311A..$5315E
    cpy.w #$0000                ; $8AB15E
    beq CODE_8AB167             ; $8AB161
CODE_8AB163:
    lsr a                       ; $8AB163
    dey                         ; $8AB164
    bne CODE_8AB163             ; $8AB165
CODE_8AB167:
    rts                         ; $8AB167
    incbin !baserom:$53168..$53174
    sep.b #$20                  ; $8AB174
    lda.b #$10                  ; $8AB176
    sta.w $03DC                 ; $8AB178
    rep.b #$20                  ; $8AB17B
    inc.b $6C                   ; $8AB17D
    rts                         ; $8AB17F
    incbin !baserom:$53180..$53659
    sta.b $62                   ; $8AB659
    jsr.w $B91A                 ; $8AB65B
CODE_8AB65E:
    jsr.w $9F76                 ; $8AB65E
    lda.b $6C                   ; $8AB661
    bne CODE_8AB67F             ; $8AB663
    lda.w #$0001                ; $8AB665
    jsl.l $8ABA90               ; $8AB668
    jsr.w $9FA1                 ; $8AB66C
    lda.b $62                   ; $8AB66F
    jsr.w $A2AF                 ; $8AB671
    lda.b $5E                   ; $8AB674
    beq CODE_8AB65E             ; $8AB676
    incbin !baserom:$53678..$5367F
CODE_8AB67F:
    rts                         ; $8AB67F
    phx                         ; $8AB680
    phy                         ; $8AB681
    inc a                       ; $8AB682
    inc a                       ; $8AB683
    inc a                       ; $8AB684
    inc a                       ; $8AB685
    sta.b $60                   ; $8AB686
CODE_8AB688:
    ldy.w #$0000                ; $8AB688
    lda.b ($60),y               ; $8AB68B
    beq CODE_8AB6A7             ; $8AB68D
    ldy.w #$0004                ; $8AB68F
    lda.b ($60),y               ; $8AB692
    tax                         ; $8AB694
    ldy.w #$0006                ; $8AB695
    lda.b ($60),y               ; $8AB698
    sta.w $1516,x               ; $8AB69A
    clc                         ; $8AB69D
    lda.b $60                   ; $8AB69E
    adc.w #$0008                ; $8AB6A0
    sta.b $60                   ; $8AB6A3
    bra CODE_8AB688             ; $8AB6A5
CODE_8AB6A7:
    ply                         ; $8AB6A7
    plx                         ; $8AB6A8
    rts                         ; $8AB6A9
    phx                         ; $8AB6AA
    pha                         ; $8AB6AB
    sec                         ; $8AB6AC
    lda.b $40                   ; $8AB6AD
    sbc.b $3E                   ; $8AB6AF
    lsr a                       ; $8AB6B1
    sta.b $42                   ; $8AB6B2
    pla                         ; $8AB6B4
    sta.b $44                   ; $8AB6B5
    sep.b #$20                  ; $8AB6B7
    ldy.w #$0000                ; $8AB6B9
CODE_8AB6BC:
    lda.b ($44),y               ; $8AB6BC
    beq CODE_8AB6C3             ; $8AB6BE
    iny                         ; $8AB6C0
    bra CODE_8AB6BC             ; $8AB6C1
CODE_8AB6C3:
    rep.b #$20                  ; $8AB6C3
    sty.b $5A                   ; $8AB6C5
    tya                         ; $8AB6C7
    lsr a                       ; $8AB6C8
    sta.b $46                   ; $8AB6C9
    sec                         ; $8AB6CB
    lda.b $42                   ; $8AB6CC
    sbc.b $46                   ; $8AB6CE
    clc                         ; $8AB6D0
    adc.b $3E                   ; $8AB6D1
    inc a                       ; $8AB6D3
    sta.b $3A                   ; $8AB6D4
    sep.b #$20                  ; $8AB6D6
    ldy.w #$0000                ; $8AB6D8
CODE_8AB6DB:
    lda.b $68                   ; $8AB6DB
    beq CODE_8AB6EB             ; $8AB6DD
    incbin !baserom:$536DF..$536EB
CODE_8AB6EB:
    lda.b ($44),y               ; $8AB6EB
    beq CODE_8AB6F5             ; $8AB6ED
    jsr.w $B83C                 ; $8AB6EF
    iny                         ; $8AB6F2
    bra CODE_8AB6DB             ; $8AB6F3
CODE_8AB6F5:
    rep.b #$20                  ; $8AB6F5
    lda.b $86                   ; $8AB6F7
    bne CODE_8AB6FE             ; $8AB6F9
    jsr.w $B700                 ; $8AB6FB
CODE_8AB6FE:
    plx                         ; $8AB6FE
    rts                         ; $8AB6FF
    phy                         ; $8AB700
    lda.b $3C                   ; $8AB701
    sec                         ; $8AB703
    sbc.w #$000A                ; $8AB704
    bmi CODE_8AB718             ; $8AB707
    tay                         ; $8AB709
    lda.b $5A                   ; $8AB70A
    and.w #$0001                ; $8AB70C
    beq CODE_8AB719             ; $8AB70F
    lda.w #$0004                ; $8AB711
    sta.w $0D04,y               ; $8AB714
    ply                         ; $8AB717
CODE_8AB718:
    rts                         ; $8AB718
CODE_8AB719:
    lda.w #$0000                ; $8AB719
    sta.w $0D04,y               ; $8AB71C
    ply                         ; $8AB71F
    rts                         ; $8AB720
    incbin !baserom:$53721..$5383C
    ldx.w $0D7C                 ; $8AB83C
    db $D0,$03 ; bne            ; $8AB83F
    jmp.w $B8C9                 ; $8AB841
    incbin !baserom:$53844..$538C9
    php                         ; $8AB8C9
    phy                         ; $8AB8CA
    rep.b #$20                  ; $8AB8CB
    and.w #$00FF                ; $8AB8CD
    ldy.b $3A                   ; $8AB8D0
    sty.b $36                   ; $8AB8D2
    ldy.b $3C                   ; $8AB8D4
    sty.b $38                   ; $8AB8D6
    jsr.w $B909                 ; $8AB8D8
    sec                         ; $8AB8DB
    sbc.w #$0020                ; $8AB8DC
    asl a                       ; $8AB8DF
    tay                         ; $8AB8E0
    lda.w $C9C4,y               ; $8AB8E1
    ora.w #$3C00                ; $8AB8E4
    sta.l $7F0000,x             ; $8AB8E7
    lda.w $CA3A,y               ; $8AB8EB
    ora.w #$3C00                ; $8AB8EE
    sta.l $7F0040,x             ; $8AB8F1
    inc.b $3A                   ; $8AB8F5
    ply                         ; $8AB8F7
    plp                         ; $8AB8F8
    rts                         ; $8AB8F9
    incbin !baserom:$538FA..$53909
    pha                         ; $8AB909
    lda.b $38                   ; $8AB90A
    asl a                       ; $8AB90C
    asl a                       ; $8AB90D
    asl a                       ; $8AB90E
    asl a                       ; $8AB90F
    asl a                       ; $8AB910
    asl a                       ; $8AB911
    asl.b $36                   ; $8AB912
    clc                         ; $8AB914
    adc.b $36                   ; $8AB915
    tax                         ; $8AB917
    pla                         ; $8AB918
    rts                         ; $8AB919
    lda.w #$0000                ; $8AB91A
    sta.b $70                   ; $8AB91D
    lda.w #$007F                ; $8AB91F
    sta.b $72                   ; $8AB922
    stz.b $76                   ; $8AB924
    lda.w #$003E                ; $8AB926
    sta.b $74                   ; $8AB929
    ldx.w #$0010                ; $8AB92B
CODE_8AB92E:
    stz.b $78                   ; $8AB92E
    jsr.w $B942                 ; $8AB930
    lda.b $78                   ; $8AB933
    beq CODE_8AB93E             ; $8AB935
    lda.w #$0001                ; $8AB937
    jsl.l $8ABA90               ; $8AB93A
CODE_8AB93E:
    dex                         ; $8AB93E
    bne CODE_8AB92E             ; $8AB93F
    rts                         ; $8AB941
    lda.b $70                   ; $8AB942
    pha                         ; $8AB944
    phx                         ; $8AB945
    ldx.w #$001C                ; $8AB946
CODE_8AB949:
    ldy.b $76                   ; $8AB949
    lda.b [$70],y               ; $8AB94B
    cmp.w #$003D                ; $8AB94D
    beq CODE_8AB954             ; $8AB950
    inc.b $78                   ; $8AB952
CODE_8AB954:
    lda.w #$003D                ; $8AB954
    sta.b [$70],y               ; $8AB957
    ldy.b $74                   ; $8AB959
    lda.b [$70],y               ; $8AB95B
    cmp.w #$003D                ; $8AB95D
    beq CODE_8AB964             ; $8AB960
    inc.b $78                   ; $8AB962
CODE_8AB964:
    lda.w #$003D                ; $8AB964
    sta.b [$70],y               ; $8AB967
    lda.b $70                   ; $8AB969
    clc                         ; $8AB96B
    adc.w #$0040                ; $8AB96C
    sta.b $70                   ; $8AB96F
    dex                         ; $8AB971
    bne CODE_8AB949             ; $8AB972
    plx                         ; $8AB974
    pla                         ; $8AB975
    sta.b $70                   ; $8AB976
    inc.b $76                   ; $8AB978
    inc.b $76                   ; $8AB97A
    dec.b $74                   ; $8AB97C
    dec.b $74                   ; $8AB97E
    rts                         ; $8AB980
    ldy.w #$0800                ; $8AB981
    ldx.w #$0000                ; $8AB984
    lda.w #$003D                ; $8AB987
CODE_8AB98A:
    sta.l $7F0000,x             ; $8AB98A
    inx                         ; $8AB98E
    inx                         ; $8AB98F
    dey                         ; $8AB990
    bne CODE_8AB98A             ; $8AB991
    rts                         ; $8AB993
    ldy.w #$0800                ; $8AB994
    ldx.w #$FEC3                ; $8AB997
    lda.w #$0088                ; $8AB99A
    jsl.l $80837D               ; $8AB99D
    ldy.w #$1000                ; $8AB9A1
    ldx.w #$F87F                ; $8AB9A4
    lda.w #$0088                ; $8AB9A7
    jsl.l $80837D               ; $8AB9AA
    rts                         ; $8AB9AE
    incbin !baserom:$539AF..$53A55
    ldx.w #$EE52                ; $8ABA55
    ldy.w #$4000                ; $8ABA58
    lda.w #$0096                ; $8ABA5B
    jsl.l $80837D               ; $8ABA5E
    ldx.w #$FA8B                ; $8ABA62
    ldy.w #$7000                ; $8ABA65
    lda.w #$0085                ; $8ABA68
    jsl.l $80837D               ; $8ABA6B
    lda.w #$8A00                ; $8ABA6F
    ldx.w #$C658                ; $8ABA72
    ldy.w #$0200                ; $8ABA75
    jsl.l $8083CB               ; $8ABA78
    rts                         ; $8ABA7C
    lda.w #$007F                ; $8ABA7D
    sta.b $14                   ; $8ABA80
    ldx.w #$0000                ; $8ABA82
    ldy.w #$3000                ; $8ABA85
    lda.w #$0800                ; $8ABA88
    jsl.l $8083AD               ; $8ABA8B
    rts                         ; $8ABA8F
    clc                         ; $8ABA90
    adc.l $810024               ; $8ABA91
CODE_8ABA95:
    cmp.l $810024               ; $8ABA95
    bne CODE_8ABA95             ; $8ABA99
    rtl                         ; $8ABA9B
    incbin !baserom:$53A9C..$53ACB
    lda.w #$0000                ; $8ABACB
    sta.w $2116                 ; $8ABACE
    ldx.w #$0060                ; $8ABAD1
    ldy.w #$0000                ; $8ABAD4
CODE_8ABAD7:
    lda.w $C8E4,y               ; $8ABAD7
    sta.w $2118                 ; $8ABADA
    iny                         ; $8ABADD
    iny                         ; $8ABADE
    dex                         ; $8ABADF
    bpl CODE_8ABAD7             ; $8ABAE0
    rts                         ; $8ABAE2
    incbin !baserom:$53AE3..$54056
    lda.w $0008                 ; $8AC056
    pha                         ; $8AC059
    stz.w $0196                 ; $8AC05A
    stz.w $0198                 ; $8AC05D
    stz.w $019A                 ; $8AC060
    jsl.l $8A8001               ; $8AC063
    stz.b $8E                   ; $8AC067
    ldx.w #$C4B0                ; $8AC069
    lda.w #$008A                ; $8AC06C
    jsl.l $8081C8               ; $8AC06F
    jsl.l $8A8DEC               ; $8AC073
    jsr.w $BA55                 ; $8AC077
    ldy.w #$0800                ; $8AC07A
    ldx.w #$F861                ; $8AC07D
    lda.w #$0087                ; $8AC080
    jsl.l $80837D               ; $8AC083
    jsr.w $C24B                 ; $8AC087
    ldy.w #$0000                ; $8AC08A
    ldx.w #$F5BC                ; $8AC08D
    lda.w #$0087                ; $8AC090
    jsl.l $80837D               ; $8AC093
    jsl.l $8A8EEF               ; $8AC097
    jsr.w $B981                 ; $8AC09B
    lda.w #$FC5F                ; $8AC09E
    sta.w $0008                 ; $8AC0A1
    inc.b $8E                   ; $8AC0A4
    sep.b #$20                  ; $8AC0A6
    lda.b #$BF                  ; $8AC0A8
    sta.w $210E                 ; $8AC0AA
    lda.b #$FF                  ; $8AC0AD
    sta.w $210E                 ; $8AC0AF
    rep.b #$20                  ; $8AC0B2
    lda.w #$0058                ; $8AC0B4
    sta.b $42                   ; $8AC0B7
    lda.w #$0014                ; $8AC0B9
    sta.b $44                   ; $8AC0BC
    jsr.w $C19C                 ; $8AC0BE
    jsr.w $C0F2                 ; $8AC0C1
    lda.w #$0F00                ; $8AC0C4
    sta.w $0198                 ; $8AC0C7
    lda.w #$0080                ; $8AC0CA
    sta.w $0196                 ; $8AC0CD
    ldx.w #$00B4                ; $8AC0D0
CODE_8AC0D3:
    phx                         ; $8AC0D3
    lda.w #$0001                ; $8AC0D4
    jsl.l $8ABA90               ; $8AC0D7
    plx                         ; $8AC0DB
    dex                         ; $8AC0DC
    bne CODE_8AC0D3             ; $8AC0DD
    stz.w $0198                 ; $8AC0DF
    lda.w #$FFC0                ; $8AC0E2
    sta.w $0196                 ; $8AC0E5
CODE_8AC0E8:
    lda.w $019A                 ; $8AC0E8
    bne CODE_8AC0E8             ; $8AC0EB
    pla                         ; $8AC0ED
    sta.w $0008                 ; $8AC0EE
    rts                         ; $8AC0F1
    stz.b $90                   ; $8AC0F2
    inc.b $90                   ; $8AC0F4
    ldx.w #$000C                ; $8AC0F6
    ldy.w #$0006                ; $8AC0F9
    lda.w #$C44F                ; $8AC0FC
    jsr.w $C137                 ; $8AC0FF
    dec.b $90                   ; $8AC102
    ldx.w #$0006                ; $8AC104
    ldy.w #$0016                ; $8AC107
    lda.w #$C458                ; $8AC10A
    jsr.w $C137                 ; $8AC10D
    ldx.w #$0005                ; $8AC110
    ldy.w #$0017                ; $8AC113
    lda.w #$C46D                ; $8AC116
    jsr.w $C137                 ; $8AC119
    ldx.w #$0005                ; $8AC11C
    ldy.w #$0018                ; $8AC11F
    lda.w #$C484                ; $8AC122
    jsr.w $C137                 ; $8AC125
    inc.b $90                   ; $8AC128
    ldx.w #$0006                ; $8AC12A
    ldy.w #$001A                ; $8AC12D
    lda.w #$C49B                ; $8AC130
    jsr.w $C137                 ; $8AC133
    rts                         ; $8AC136
    stx.b $3A                   ; $8AC137
    sty.b $3C                   ; $8AC139
    jsr.w $C13F                 ; $8AC13B
    rts                         ; $8AC13E
    sta.b $48                   ; $8AC13F
    ldy.w #$0000                ; $8AC141
CODE_8AC144:
    lda.b ($48),y               ; $8AC144
    and.w #$00FF                ; $8AC146
    beq CODE_8AC156             ; $8AC149
    cmp.w #$0080                ; $8AC14B
    db $F0,$07 ; beq            ; $8AC14E
    jsr.w $C278                 ; $8AC150
    iny                         ; $8AC153
    bra CODE_8AC144             ; $8AC154
CODE_8AC156:
    rts                         ; $8AC156
    incbin !baserom:$54157..$5419C
    ldx.w #$0000                ; $8AC19C
CODE_8AC19F:
    lda.w $C443,x               ; $8AC19F
    ora.w #$2000                ; $8AC1A2
    sta.w $0800,x               ; $8AC1A5
    lda.b $42                   ; $8AC1A8
    sta.w $0600,x               ; $8AC1AA
    clc                         ; $8AC1AD
    adc.w #$0010                ; $8AC1AE
    sta.b $42                   ; $8AC1B1
    lda.b $44                   ; $8AC1B3
    sta.w $0700,x               ; $8AC1B5
    lda.w #$0001                ; $8AC1B8
    sta.w $0500,x               ; $8AC1BB
    inx                         ; $8AC1BE
    inx                         ; $8AC1BF
    cpx.w #$000C                ; $8AC1C0
    bne CODE_8AC19F             ; $8AC1C3
    rts                         ; $8AC1C5
    incbin !baserom:$541C6..$5424B
    sep.b #$20                  ; $8AC24B
    lda.b #$20                  ; $8AC24D
    sta.w $2121                 ; $8AC24F
    ldy.w #$0060                ; $8AC252
CODE_8AC255:
    stz.w $2122                 ; $8AC255
    dey                         ; $8AC258
    bne CODE_8AC255             ; $8AC259
    rep.b #$20                  ; $8AC25B
    lda.w #$8A80                ; $8AC25D
    ldx.w #$C964                ; $8AC260
    ldy.w #$0020                ; $8AC263
    jsl.l $8083CB               ; $8AC266
    lda.w #$8A60                ; $8AC26A
    ldx.w #$C984                ; $8AC26D
    ldy.w #$0040                ; $8AC270
    jsl.l $8083CB               ; $8AC273
    rts                         ; $8AC277
    php                         ; $8AC278
    phy                         ; $8AC279
    phx                         ; $8AC27A
    rep.b #$20                  ; $8AC27B
    and.w #$00FF                ; $8AC27D
    ldy.b $3A                   ; $8AC280
    sty.b $36                   ; $8AC282
    ldy.b $3C                   ; $8AC284
    sty.b $38                   ; $8AC286
    jsr.w $C2CC                 ; $8AC288
    jsr.w $C2A2                 ; $8AC28B
    ldy.b $90                   ; $8AC28E
    beq CODE_8AC295             ; $8AC290
    ora.w #$0400                ; $8AC292
CODE_8AC295:
    ora.w #$3880                ; $8AC295
    sta.l $7F0000,x             ; $8AC298
    inc.b $3A                   ; $8AC29C
    plx                         ; $8AC29E
    ply                         ; $8AC29F
    plp                         ; $8AC2A0
    rts                         ; $8AC2A1
    cmp.w #$0020                ; $8AC2A2
    beq CODE_8AC2BB             ; $8AC2A5
    cmp.w #$002E                ; $8AC2A7
    beq CODE_8AC2BF             ; $8AC2AA
    cmp.w #$0021                ; $8AC2AC
    db $F0,$12 ; beq            ; $8AC2AF
    cmp.w #$0041                ; $8AC2B1
    bmi CODE_8AC2C7             ; $8AC2B4
    sec                         ; $8AC2B6
    sbc.w #$0041                ; $8AC2B7
    rts                         ; $8AC2BA
CODE_8AC2BB:
    lda.w #$0025                ; $8AC2BB
    rts                         ; $8AC2BE
CODE_8AC2BF:
    lda.w #$0024                ; $8AC2BF
    rts                         ; $8AC2C2
    incbin !baserom:$542C3..$542C7
CODE_8AC2C7:
    sec                         ; $8AC2C7
    sbc.w #$0016                ; $8AC2C8
    rts                         ; $8AC2CB
    pha                         ; $8AC2CC
    lda.b $38                   ; $8AC2CD
    asl a                       ; $8AC2CF
    asl a                       ; $8AC2D0
    asl a                       ; $8AC2D1
    asl a                       ; $8AC2D2
    asl a                       ; $8AC2D3
    asl a                       ; $8AC2D4
    asl.b $36                   ; $8AC2D5
    clc                         ; $8AC2D7
    adc.b $36                   ; $8AC2D8
    tax                         ; $8AC2DA
    pla                         ; $8AC2DB
    rts                         ; $8AC2DC
    lda.w $03DC                 ; $8AC2DD
    and.w #$0300                ; $8AC2E0
    db $F0,$0A ; beq            ; $8AC2E3
    cmp.w #$0100                ; $8AC2E5
    beq CODE_8AC311             ; $8AC2E8
    incbin !baserom:$542EA..$54311
CODE_8AC311:
    ldx.w #$0000                ; $8AC311
    lda.l $8ACADE,x             ; $8AC314
    and.w #$00FF                ; $8AC318
    cmp.w $0444                 ; $8AC31B
    beq CODE_8AC328             ; $8AC31E
    incbin !baserom:$54320..$54328
CODE_8AC328:
    lda.l $8ACADE,x             ; $8AC328
    and.w #$00FF                ; $8AC32C
    sta.w $0444                 ; $8AC32F
    rtl                         ; $8AC332
    incbin !baserom:$54333..$58000
