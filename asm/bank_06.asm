; bank $86 (file offset $030000)
org $868000

    incbin !baserom:$30000..$34E4A
    phb                         ; $86CE4A
    lda.w #$0037                ; $86CE4B
    mvn $7F,$7F                 ; $86CE4E
    tya                         ; $86CE51
    clc                         ; $86CE52
    adc.w #$0008                ; $86CE53
    tay                         ; $86CE56
    lda.w #$0037                ; $86CE57
    mvn $7F,$7F                 ; $86CE5A
    tya                         ; $86CE5D
    clc                         ; $86CE5E
    adc.w #$0008                ; $86CE5F
    tay                         ; $86CE62
    lda.w #$0037                ; $86CE63
    mvn $7F,$7F                 ; $86CE66
    tya                         ; $86CE69
    clc                         ; $86CE6A
    adc.w #$0008                ; $86CE6B
    tay                         ; $86CE6E
    lda.w #$0037                ; $86CE6F
    mvn $7F,$7F                 ; $86CE72
    tya                         ; $86CE75
    clc                         ; $86CE76
    adc.w #$0008                ; $86CE77
    tay                         ; $86CE7A
    lda.w #$0037                ; $86CE7B
    mvn $7F,$7F                 ; $86CE7E
    tya                         ; $86CE81
    clc                         ; $86CE82
    adc.w #$0008                ; $86CE83
    tay                         ; $86CE86
    lda.w #$0037                ; $86CE87
    mvn $7F,$7F                 ; $86CE8A
    tya                         ; $86CE8D
    clc                         ; $86CE8E
    adc.w #$0008                ; $86CE8F
    tay                         ; $86CE92
    lda.w #$0037                ; $86CE93
    mvn $7F,$7F                 ; $86CE96
    tya                         ; $86CE99
    clc                         ; $86CE9A
    adc.w #$0008                ; $86CE9B
    tay                         ; $86CE9E
    lda.w #$0037                ; $86CE9F
    mvn $7F,$7F                 ; $86CEA2
    plb                         ; $86CEA5
    rtl                         ; $86CEA6
    incbin !baserom:$34EA7..$35D85
    rep.b #$30                  ; $86DD85
    phd                         ; $86DD87
    lda.w #$0000                ; $86DD88
    sta.l $81046D               ; $86DD8B
    sep.b #$30                  ; $86DD8F
    sta.l $81045C               ; $86DD91
    sta.l $81045D               ; $86DD95
    sta.l $81045F               ; $86DD99
    sta.l $810463               ; $86DD9D
    sta.l $810464               ; $86DDA1
    sta.l $810468               ; $86DDA5
    sta.l $810469               ; $86DDA9
    sta.l $81045B               ; $86DDAD
    rep.b #$30                  ; $86DDB1
    phd                         ; $86DDB3
    lda.w #$7F86                ; $86DDB4
    ldx.w #$E478                ; $86DDB7
    ldy.w #$0000                ; $86DDBA
    jsl.l $82FB70               ; $86DDBD
    pld                         ; $86DDC1
    lda.w #$0000                ; $86DDC2
    sta.b $E0                   ; $86DDC5
    lda.w #$007F                ; $86DDC7
    sta.b $E2                   ; $86DDCA
    jsr.w $E096                 ; $86DDCC
    stz.w $2140                 ; $86DDCF
    stz.w $2142                 ; $86DDD2
    ldx.w #$07D0                ; $86DDD5
CODE_86DDD8:
    dex                         ; $86DDD8
    bne CODE_86DDD8             ; $86DDD9
    sep.b #$20                  ; $86DDDB
    lda.b #$77                  ; $86DDDD
CODE_86DDDF:
    cmp.w $2141                 ; $86DDDF
    bne CODE_86DDDF             ; $86DDE2
    rep.b #$20                  ; $86DDE4
    lda.w #$FFFF                ; $86DDE6
    sta.l $81046D               ; $86DDE9
    pld                         ; $86DDED
    rtl                         ; $86DDEE
    rep.b #$30                  ; $86DDEF
    phb                         ; $86DDF1
    phd                         ; $86DDF2
    pea.w $8686                 ; $86DDF3
    plb                         ; $86DDF6
    plb                         ; $86DDF7
    sta.l $810471               ; $86DDF8
    lda.w #$0000                ; $86DDFC
    sta.l $81046D               ; $86DDFF
    sep.b #$20                  ; $86DE03
    lda.b #$01                  ; $86DE05
    sta.l $81045B               ; $86DE07
    rep.b #$20                  ; $86DE0B
    lda.l $810471               ; $86DE0D
    asl a                       ; $86DE11
    tay                         ; $86DE12
    lda.w $EFAB,y               ; $86DE13
    sta.l $810473               ; $86DE16
    tay                         ; $86DE1A
    lda.w #$4000                ; $86DE1B
    sta.w $0461                 ; $86DE1E
    lda.w $0000,y               ; $86DE21
    beq CODE_86DE63             ; $86DE24
CODE_86DE26:
    tax                         ; $86DE26
    phd                         ; $86DE27
    lda.w $0002,y               ; $86DE28
    and.w #$00FF                ; $86DE2B
    ora.w #$7F00                ; $86DE2E
    ldy.w #$0000                ; $86DE31
    jsl.l $82FB70               ; $86DE34
    pld                         ; $86DE38
    lda.w #$0000                ; $86DE39
    sta.b $E0                   ; $86DE3C
    lda.w #$007F                ; $86DE3E
    sta.b $E2                   ; $86DE41
    lda.w $0024                 ; $86DE43
CODE_86DE46:
    cmp.w $0024                 ; $86DE46
    beq CODE_86DE46             ; $86DE49
    jsr.w $DFFB                 ; $86DE4B
    jsr.w $E0F0                 ; $86DE4E
    lda.l $810473               ; $86DE51
    clc                         ; $86DE55
    adc.w #$0004                ; $86DE56
    tay                         ; $86DE59
    sta.l $810473               ; $86DE5A
    lda.w $0000,y               ; $86DE5E
    bne CODE_86DE26             ; $86DE61
CODE_86DE63:
    iny                         ; $86DE63
    iny                         ; $86DE64
    tya                         ; $86DE65
    sta.l $810473               ; $86DE66
    lda.w $0000,y               ; $86DE6A
    beq CODE_86DE96             ; $86DE6D
    incbin !baserom:$35E6F..$35E96
CODE_86DE96:
    iny                         ; $86DE96
    iny                         ; $86DE97
    lda.w $0000,y               ; $86DE98
    sta.w $0461                 ; $86DE9B
    sta.l $7F0002               ; $86DE9E
    lda.w #$0000                ; $86DEA2
    sta.l $7F0000               ; $86DEA5
    iny                         ; $86DEA9
    iny                         ; $86DEAA
    phy                         ; $86DEAB
    ldx.w #$0000                ; $86DEAC
CODE_86DEAF:
    lda.w $0000,y               ; $86DEAF
    beq CODE_86DEC6             ; $86DEB2
    txa                         ; $86DEB4
    cmp.w $0006,y               ; $86DEB5
    bcs CODE_86DEBE             ; $86DEB8
    lda.w $0006,y               ; $86DEBA
    tax                         ; $86DEBD
CODE_86DEBE:
    tya                         ; $86DEBE
    clc                         ; $86DEBF
    adc.w #$0008                ; $86DEC0
    tay                         ; $86DEC3
    bra CODE_86DEAF             ; $86DEC4
CODE_86DEC6:
    txa                         ; $86DEC6
    inc a                       ; $86DEC7
    asl a                       ; $86DEC8
    asl a                       ; $86DEC9
    clc                         ; $86DECA
    adc.w $0461                 ; $86DECB
    sta.l $810475               ; $86DECE
    ply                         ; $86DED2
CODE_86DED3:
    lda.w $0000,y               ; $86DED3
    beq CODE_86DF41             ; $86DED6
    sta.b $E0                   ; $86DED8
    sec                         ; $86DEDA
    lda.w $0002,y               ; $86DEDB
    sbc.b $E0                   ; $86DEDE
    dec a                       ; $86DEE0
    dec a                       ; $86DEE1
    sta.l $810477               ; $86DEE2
    lda.w $0004,y               ; $86DEE6
    sta.b $E2                   ; $86DEE9
    lda.w $0006,y               ; $86DEEB
    asl a                       ; $86DEEE
    asl a                       ; $86DEEF
    tax                         ; $86DEF0
    lda.l $810475               ; $86DEF1
    sta.l $7F0004,x             ; $86DEF5
    tya                         ; $86DEF9
    clc                         ; $86DEFA
    adc.w #$0008                ; $86DEFB
    sta.l $810473               ; $86DEFE
    ldy.w #$0000                ; $86DF02
    lda.b [$E0],y               ; $86DF05
    clc                         ; $86DF07
    adc.l $7F0004,x             ; $86DF08
    sta.l $7F0006,x             ; $86DF0C
    txa                         ; $86DF10
    cmp.l $7F0000               ; $86DF11
    bcc CODE_86DF1B             ; $86DF15
    sta.l $7F0000               ; $86DF17
CODE_86DF1B:
    inc.b $E0                   ; $86DF1B
    inc.b $E0                   ; $86DF1D
    lda.w $0024                 ; $86DF1F
CODE_86DF22:
    cmp.w $0024                 ; $86DF22
    beq CODE_86DF22             ; $86DF25
    jsr.w $DFFB                 ; $86DF27
    jsr.w $E2B4                 ; $86DF2A
    lda.l $810473               ; $86DF2D
    tay                         ; $86DF31
    lda.l $810477               ; $86DF32
    clc                         ; $86DF36
    adc.l $810475               ; $86DF37
    sta.l $810475               ; $86DF3B
    bra CODE_86DED3             ; $86DF3F
CODE_86DF41:
    lda.l $7F0000               ; $86DF41
    clc                         ; $86DF45
    adc.w #$0004                ; $86DF46
    sta.l $7F0000               ; $86DF49
    tax                         ; $86DF4D
    lda.w #$0000                ; $86DF4E
    sta.l $7F0004,x             ; $86DF51
    lda.w #$0000                ; $86DF55
    sta.b $E0                   ; $86DF58
    lda.w #$007F                ; $86DF5A
    sta.b $E2                   ; $86DF5D
    lda.w $0024                 ; $86DF5F
CODE_86DF62:
    cmp.w $0024                 ; $86DF62
    beq CODE_86DF62             ; $86DF65
    jsr.w $DFFB                 ; $86DF67
    jsr.w $E0F0                 ; $86DF6A
    lda.w #$0077                ; $86DF6D
CODE_86DF70:
    cmp.w $2141                 ; $86DF70
    bne CODE_86DF70             ; $86DF73
    sep.b #$20                  ; $86DF75
    lda.b #$00                  ; $86DF77
    sta.l $81045B               ; $86DF79
    sta.l $810463               ; $86DF7D
    sta.l $810464               ; $86DF81
    sta.l $810468               ; $86DF85
    sta.l $810469               ; $86DF89
    sta.l $81045C               ; $86DF8D
    sta.l $81045F               ; $86DF91
    lda.b #$FF                  ; $86DF95
    sta.l $81045D               ; $86DF97
    rep.b #$20                  ; $86DF9B
    lda.w #$0000                ; $86DF9D
    sta.l $81046F               ; $86DFA0
    lda.w #$FFFF                ; $86DFA4
    sta.l $81046D               ; $86DFA7
    pld                         ; $86DFAB
    plb                         ; $86DFAC
    rtl                         ; $86DFAD
    lda.w $03E0                 ; $86DFAE
    bmi CODE_86DFCA             ; $86DFB1
    asl a                       ; $86DFB3
    sep.b #$20                  ; $86DFB4
    clc                         ; $86DFB6
    adc.w $045C                 ; $86DFB7
    bpl CODE_86DFC4             ; $86DFBA
    stz.w $03E0                 ; $86DFBC
    stz.w $03E1                 ; $86DFBF
    lda.b #$7F                  ; $86DFC2
CODE_86DFC4:
    sta.w $045C                 ; $86DFC4
    rep.b #$20                  ; $86DFC7
    rts                         ; $86DFC9
CODE_86DFCA:
    asl a                       ; $86DFCA
    sep.b #$20                  ; $86DFCB
    clc                         ; $86DFCD
    adc.w $045C                 ; $86DFCE
    bpl CODE_86DFDB             ; $86DFD1
    stz.w $03E0                 ; $86DFD3
    stz.w $03E1                 ; $86DFD6
    lda.b #$00                  ; $86DFD9
CODE_86DFDB:
    sta.w $045C                 ; $86DFDB
    rep.b #$20                  ; $86DFDE
    rts                         ; $86DFE0
    php                         ; $86DFE1
    jsr.w $DFAE                 ; $86DFE2
    lda.l $81046D               ; $86DFE5
    bpl CODE_86DFF9             ; $86DFE9
    sep.b #$20                  ; $86DFEB
    lda.b #$77                  ; $86DFED
    cmp.w $2141                 ; $86DFEF
    bne CODE_86DFF9             ; $86DFF2
    rep.b #$20                  ; $86DFF4
    jsr.w $DFFB                 ; $86DFF6
CODE_86DFF9:
    plp                         ; $86DFF9
    rtl                         ; $86DFFA
    php                         ; $86DFFB
    sep.b #$30                  ; $86DFFC
    lda.b #$77                  ; $86DFFE
CODE_86E000:
    cmp.w $2141                 ; $86E000
    bne CODE_86E000             ; $86E003
    lda.w $045C                 ; $86E005
    sta.w $2141                 ; $86E008
    lda.w $045D                 ; $86E00B
    sta.w $2142                 ; $86E00E
    lda.w $045F                 ; $86E011
    sta.w $2143                 ; $86E014
    lda.w $213F                 ; $86E017
    lsr a                       ; $86E01A
    lsr a                       ; $86E01B
    lsr a                       ; $86E01C
    and.b #$02                  ; $86E01D
    ora.w $045B                 ; $86E01F
    ora.b #$80                  ; $86E022
    sta.w $2140                 ; $86E024
    lda.w $0462                 ; $86E027
    and.b #$7F                  ; $86E02A
    ldy.w $0461                 ; $86E02C
CODE_86E02F:
    bit.w $2140                 ; $86E02F
    bpl CODE_86E02F             ; $86E032
    sty.w $2141                 ; $86E034
    ldy.w $045E                 ; $86E037
    sty.w $2142                 ; $86E03A
    ldy.w $0460                 ; $86E03D
    sty.w $2143                 ; $86E040
    sta.w $2140                 ; $86E043
    lda.w $0463                 ; $86E046
    ora.b #$80                  ; $86E049
    ldx.w $0464                 ; $86E04B
    ldy.w $0467                 ; $86E04E
CODE_86E051:
    bit.w $2140                 ; $86E051
    bmi CODE_86E051             ; $86E054
    stx.w $2141                 ; $86E056
    sty.w $2142                 ; $86E059
    ldy.w $0466                 ; $86E05C
    sty.w $2143                 ; $86E05F
    sta.w $2140                 ; $86E062
    lda.w $0468                 ; $86E065
    and.b #$7F                  ; $86E068
    ldx.w $0469                 ; $86E06A
    ldy.w $046C                 ; $86E06D
CODE_86E070:
    bit.w $2140                 ; $86E070
    bpl CODE_86E070             ; $86E073
    stx.w $2141                 ; $86E075
    sty.w $2142                 ; $86E078
    ldy.w $046B                 ; $86E07B
    sty.w $2143                 ; $86E07E
    sta.w $2140                 ; $86E081
CODE_86E084:
    lda.w $2140                 ; $86E084
    bmi CODE_86E084             ; $86E087
    stz.w $0463                 ; $86E089
    stz.w $0468                 ; $86E08C
    lda.b #$FF                  ; $86E08F
    sta.w $045F                 ; $86E091
    plp                         ; $86E094
    rts                         ; $86E095
    rep.b #$30                  ; $86E096
    phd                         ; $86E098
    ldy.w #$0000                ; $86E099
    lda.w #$BBAA                ; $86E09C
CODE_86E09F:
    cmp.w $2140                 ; $86E09F
    bne CODE_86E09F             ; $86E0A2
    sep.b #$20                  ; $86E0A4
    lda.b #$CC                  ; $86E0A6
CODE_86E0A8:
    pha                         ; $86E0A8
    rep.b #$20                  ; $86E0A9
    lda.b [$E0],y               ; $86E0AB
    tax                         ; $86E0AD
    iny                         ; $86E0AE
    iny                         ; $86E0AF
    lda.b [$E0],y               ; $86E0B0
    sta.w $2142                 ; $86E0B2
    iny                         ; $86E0B5
    iny                         ; $86E0B6
    sep.b #$20                  ; $86E0B7
    lda.b #$00                  ; $86E0B9
    cpx.w #$0001                ; $86E0BB
    rol a                       ; $86E0BE
    sta.w $2141                 ; $86E0BF
    adc.b #$7F                  ; $86E0C2
    pla                         ; $86E0C4
    sta.w $2140                 ; $86E0C5
CODE_86E0C8:
    cmp.w $2140                 ; $86E0C8
    bne CODE_86E0C8             ; $86E0CB
    bvc CODE_86E0EC             ; $86E0CD
    lda.b #$00                  ; $86E0CF
CODE_86E0D1:
    xba                         ; $86E0D1
    lda.b [$E0],y               ; $86E0D2
    xba                         ; $86E0D4
    iny                         ; $86E0D5
    rep.b #$20                  ; $86E0D6
    sta.w $2140                 ; $86E0D8
    sep.b #$20                  ; $86E0DB
CODE_86E0DD:
    cmp.w $2140                 ; $86E0DD
    bne CODE_86E0DD             ; $86E0E0
    inc a                       ; $86E0E2
    dex                         ; $86E0E3
    bne CODE_86E0D1             ; $86E0E4
    adc.b #$03                  ; $86E0E6
    bne CODE_86E0A8             ; $86E0E8
    incbin !baserom:$360EA..$360EC
CODE_86E0EC:
    rep.b #$20                  ; $86E0EC
    pld                         ; $86E0EE
    rts                         ; $86E0EF
    rep.b #$30                  ; $86E0F0
    phd                         ; $86E0F2
    ldy.w #$0000                ; $86E0F3
    sep.b #$20                  ; $86E0F6
    lda.b #$AB                  ; $86E0F8
    sta.w $2143                 ; $86E0FA
    ldx.w #$0500                ; $86E0FD
    lda.b #$CD                  ; $86E100
CODE_86E102:
    dex                         ; $86E102
    bne CODE_86E106             ; $86E103
    incbin !baserom:$36105..$36106
CODE_86E106:
    cmp.w $2143                 ; $86E106
    bne CODE_86E102             ; $86E109
    lda.b #$00                  ; $86E10B
    sta.l $810479               ; $86E10D
CODE_86E111:
    rep.b #$20                  ; $86E111
    lda.b [$E0],y               ; $86E113
    beq CODE_86E18D             ; $86E115
    iny                         ; $86E117
    iny                         ; $86E118
    tax                         ; $86E119
    sta.w $2140                 ; $86E11A
    sep.b #$20                  ; $86E11D
    lda.l $810479               ; $86E11F
    eor.b #$FF                  ; $86E123
    sta.l $810479               ; $86E125
    sta.w $2143                 ; $86E129
CODE_86E12C:
    cmp.w $2143                 ; $86E12C
    bne CODE_86E12C             ; $86E12F
    rep.b #$20                  ; $86E131
    lda.b [$E0],y               ; $86E133
    iny                         ; $86E135
    iny                         ; $86E136
    sta.w $2140                 ; $86E137
    sep.b #$20                  ; $86E13A
    lda.l $810479               ; $86E13C
    eor.b #$FF                  ; $86E140
    sta.l $810479               ; $86E142
    sta.w $2143                 ; $86E146
    stx.w $4204                 ; $86E149
    lda.b #$03                  ; $86E14C
    sta.w $4206                 ; $86E14E
    nop                         ; $86E151
    nop                         ; $86E152
    nop                         ; $86E153
    nop                         ; $86E154
    nop                         ; $86E155
    nop                         ; $86E156
    nop                         ; $86E157
    lda.b #$01                  ; $86E158
    ldx.w $4214                 ; $86E15A
    bne CODE_86E16E             ; $86E15D
    incbin !baserom:$3615F..$3616E
CODE_86E16E:
    cmp.w $4216                 ; $86E16E
    bcc CODE_86E17F             ; $86E171
    beq CODE_86E17A             ; $86E173
    jsr.w $E1BA                 ; $86E175
    bra CODE_86E182             ; $86E178
CODE_86E17A:
    jsr.w $E1E6                 ; $86E17A
    bra CODE_86E182             ; $86E17D
CODE_86E17F:
    jsr.w $E22A                 ; $86E17F
CODE_86E182:
    lda.l $810479               ; $86E182
CODE_86E186:
    cmp.w $2143                 ; $86E186
    bne CODE_86E186             ; $86E189
    bra CODE_86E111             ; $86E18B
CODE_86E18D:
    lda.w #$0000                ; $86E18D
    sta.w $2140                 ; $86E190
    sep.b #$20                  ; $86E193
    lda.l $810479               ; $86E195
    eor.b #$FF                  ; $86E199
    sta.w $2143                 ; $86E19B
    lda.b #$BC                  ; $86E19E
CODE_86E1A0:
    cmp.w $2143                 ; $86E1A0
    bne CODE_86E1A0             ; $86E1A3
    sta.w $2143                 ; $86E1A5
CODE_86E1A8:
    lda.w $2143                 ; $86E1A8
    bne CODE_86E1A8             ; $86E1AB
    rep.b #$20                  ; $86E1AD
    lda.w #$0000                ; $86E1AF
    sta.w $2140                 ; $86E1B2
    sta.w $2142                 ; $86E1B5
    pld                         ; $86E1B8
    rts                         ; $86E1B9
    lda.l $810479               ; $86E1BA
CODE_86E1BE:
    cmp.w $2143                 ; $86E1BE
    bne CODE_86E1BE             ; $86E1C1
    lda.b [$E0],y               ; $86E1C3
    iny                         ; $86E1C5
    sta.w $2140                 ; $86E1C6
    lda.b [$E0],y               ; $86E1C9
    iny                         ; $86E1CB
    sta.w $2141                 ; $86E1CC
    lda.b [$E0],y               ; $86E1CF
    iny                         ; $86E1D1
    sta.w $2142                 ; $86E1D2
    lda.l $810479               ; $86E1D5
    eor.b #$FF                  ; $86E1D9
    sta.l $810479               ; $86E1DB
    sta.w $2143                 ; $86E1DF
    dex                         ; $86E1E2
    bne CODE_86E1BE             ; $86E1E3
    rts                         ; $86E1E5
    lda.l $810479               ; $86E1E6
CODE_86E1EA:
    cmp.w $2143                 ; $86E1EA
    bne CODE_86E1EA             ; $86E1ED
    lda.b [$E0],y               ; $86E1EF
    iny                         ; $86E1F1
    sta.w $2140                 ; $86E1F2
    lda.b [$E0],y               ; $86E1F5
    iny                         ; $86E1F7
    sta.w $2141                 ; $86E1F8
    lda.b [$E0],y               ; $86E1FB
    iny                         ; $86E1FD
    sta.w $2142                 ; $86E1FE
    lda.l $810479               ; $86E201
    eor.b #$FF                  ; $86E205
    sta.l $810479               ; $86E207
    sta.w $2143                 ; $86E20B
    dex                         ; $86E20E
    bne CODE_86E1EA             ; $86E20F
CODE_86E211:
    cmp.w $2143                 ; $86E211
    bne CODE_86E211             ; $86E214
    lda.b [$E0],y               ; $86E216
    iny                         ; $86E218
    sta.w $2140                 ; $86E219
    lda.l $810479               ; $86E21C
    eor.b #$FF                  ; $86E220
    sta.l $810479               ; $86E222
    sta.w $2143                 ; $86E226
    rts                         ; $86E229
    lda.l $810479               ; $86E22A
CODE_86E22E:
    cmp.w $2143                 ; $86E22E
    bne CODE_86E22E             ; $86E231
    lda.b [$E0],y               ; $86E233
    iny                         ; $86E235
    sta.w $2140                 ; $86E236
    lda.b [$E0],y               ; $86E239
    iny                         ; $86E23B
    sta.w $2141                 ; $86E23C
    lda.b [$E0],y               ; $86E23F
    iny                         ; $86E241
    sta.w $2142                 ; $86E242
    lda.l $810479               ; $86E245
    eor.b #$FF                  ; $86E249
    sta.l $810479               ; $86E24B
    sta.w $2143                 ; $86E24F
    dex                         ; $86E252
    bne CODE_86E22E             ; $86E253
CODE_86E255:
    cmp.w $2143                 ; $86E255
    bne CODE_86E255             ; $86E258
    lda.b [$E0],y               ; $86E25A
    iny                         ; $86E25C
    sta.w $2140                 ; $86E25D
    lda.b [$E0],y               ; $86E260
    iny                         ; $86E262
    sta.w $2141                 ; $86E263
    lda.l $810479               ; $86E266
    eor.b #$FF                  ; $86E26A
    sta.l $810479               ; $86E26C
    sta.w $2143                 ; $86E270
    rts                         ; $86E273
    incbin !baserom:$36274..$362B4
    rep.b #$30                  ; $86E2B4
    phd                         ; $86E2B6
    ldy.w #$0000                ; $86E2B7
    sep.b #$20                  ; $86E2BA
    lda.b #$AB                  ; $86E2BC
    sta.w $2143                 ; $86E2BE
    ldx.w #$0500                ; $86E2C1
    lda.b #$CD                  ; $86E2C4
CODE_86E2C6:
    dex                         ; $86E2C6
    bne CODE_86E2CA             ; $86E2C7
    incbin !baserom:$362C9..$362CA
CODE_86E2CA:
    cmp.w $2143                 ; $86E2CA
    bne CODE_86E2C6             ; $86E2CD
    lda.b #$00                  ; $86E2CF
    sta.l $810479               ; $86E2D1
    rep.b #$20                  ; $86E2D5
    lda.l $810477               ; $86E2D7
    beq CODE_86E351             ; $86E2DB
    tax                         ; $86E2DD
    sta.w $2140                 ; $86E2DE
    sep.b #$20                  ; $86E2E1
    lda.l $810479               ; $86E2E3
    eor.b #$FF                  ; $86E2E7
    sta.l $810479               ; $86E2E9
    sta.w $2143                 ; $86E2ED
CODE_86E2F0:
    cmp.w $2143                 ; $86E2F0
    bne CODE_86E2F0             ; $86E2F3
    rep.b #$20                  ; $86E2F5
    lda.l $810475               ; $86E2F7
    sta.w $2140                 ; $86E2FB
    sep.b #$20                  ; $86E2FE
    lda.l $810479               ; $86E300
    eor.b #$FF                  ; $86E304
    sta.l $810479               ; $86E306
    sta.w $2143                 ; $86E30A
    stx.w $4204                 ; $86E30D
    lda.b #$03                  ; $86E310
    sta.w $4206                 ; $86E312
    nop                         ; $86E315
    nop                         ; $86E316
    nop                         ; $86E317
    nop                         ; $86E318
    nop                         ; $86E319
    nop                         ; $86E31A
    nop                         ; $86E31B
    lda.b #$01                  ; $86E31C
    ldx.w $4214                 ; $86E31E
    bne CODE_86E332             ; $86E321
    incbin !baserom:$36323..$36332
CODE_86E332:
    cmp.w $4216                 ; $86E332
    db $90,$0C ; bcc            ; $86E335
    db $F0,$05 ; beq            ; $86E337
    jsr.w $E37E                 ; $86E339
    bra CODE_86E346             ; $86E33C
    incbin !baserom:$3633E..$36346
CODE_86E346:
    lda.l $810479               ; $86E346
CODE_86E34A:
    cmp.w $2143                 ; $86E34A
    bne CODE_86E34A             ; $86E34D
    rep.b #$20                  ; $86E34F
CODE_86E351:
    lda.w #$0000                ; $86E351
    sta.w $2140                 ; $86E354
    sep.b #$20                  ; $86E357
    lda.l $810479               ; $86E359
    eor.b #$FF                  ; $86E35D
    sta.w $2143                 ; $86E35F
    lda.b #$BC                  ; $86E362
CODE_86E364:
    cmp.w $2143                 ; $86E364
    bne CODE_86E364             ; $86E367
    sta.w $2143                 ; $86E369
CODE_86E36C:
    lda.w $2143                 ; $86E36C
    bne CODE_86E36C             ; $86E36F
    rep.b #$20                  ; $86E371
    lda.w #$0000                ; $86E373
    sta.w $2140                 ; $86E376
    sta.w $2142                 ; $86E379
    pld                         ; $86E37C
    rts                         ; $86E37D
    lda.l $810479               ; $86E37E
CODE_86E382:
    cmp.w $2143                 ; $86E382
    bne CODE_86E382             ; $86E385
    lda.b [$E0],y               ; $86E387
    iny                         ; $86E389
    sta.w $2140                 ; $86E38A
    lda.b [$E0],y               ; $86E38D
    iny                         ; $86E38F
    sta.w $2141                 ; $86E390
    lda.b [$E0],y               ; $86E393
    iny                         ; $86E395
    sta.w $2142                 ; $86E396
    lda.l $810479               ; $86E399
    eor.b #$FF                  ; $86E39D
    sta.l $810479               ; $86E39F
    sta.w $2143                 ; $86E3A3
    dex                         ; $86E3A6
    bne CODE_86E382             ; $86E3A7
    rts                         ; $86E3A9
    incbin !baserom:$363AA..$38000
