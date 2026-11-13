; bank $83 (file offset $018000)
org $838000

    incbin !baserom:$18000..$18644
    lda.b $FC                   ; $838644
    bmi CODE_8386B6             ; $838646
    beq CODE_8386B6             ; $838648
    ldx.b $FE                   ; $83864A
    cpx.w $0100                 ; $83864C
    beq CODE_8386B6             ; $83864F
    incbin !baserom:$18651..$186B6
CODE_8386B6:
    rts                         ; $8386B6
    incbin !baserom:$186B7..$186FB
    jsr.w $86FF                 ; $8386FB
    rtl                         ; $8386FE
    lda.w #$0001                ; $8386FF
CODE_838702:
    bit.w $4212                 ; $838702
    bne CODE_838702             ; $838705
    lda.w $4218                 ; $838707
    tax                         ; $83870A
    eor.b $0C                   ; $83870B
    stx.b $0C                   ; $83870D
    ora.b $0E                   ; $83870F
    and.b $0C                   ; $838711
    sta.b $0E                   ; $838713
    sep.b #$20                  ; $838715
    lda.w $4016                 ; $838717
    and.b #$01                  ; $83871A
    eor.b #$01                  ; $83871C
    ora.b $0C                   ; $83871E
    sta.b $0C                   ; $838720
    lda.b #$80                  ; $838722
    sta.w $4201                 ; $838724
    stz.w $4016                 ; $838727
    lda.b #$01                  ; $83872A
    sta.w $4016                 ; $83872C
    stz.w $4016                 ; $83872F
    jsr.w $87C1                 ; $838732
    rep.b #$20                  ; $838735
    lda.b $20                   ; $838737
    tax                         ; $838739
    eor.b $10                   ; $83873A
    stx.b $10                   ; $83873C
    ora.b $12                   ; $83873E
    and.b $10                   ; $838740
    sta.b $12                   ; $838742
    lda.b $22                   ; $838744
    tax                         ; $838746
    eor.b $14                   ; $838747
    stx.b $14                   ; $838749
    ora.b $16                   ; $83874B
    and.b $14                   ; $83874D
    sta.b $16                   ; $83874F
    sep.b #$20                  ; $838751
    stz.w $4201                 ; $838753
    jsr.w $87C1                 ; $838756
    lda.b #$80                  ; $838759
    sta.w $4201                 ; $83875B
    rep.b #$20                  ; $83875E
    lda.b $20                   ; $838760
    tax                         ; $838762
    eor.b $18                   ; $838763
    stx.b $18                   ; $838765
    ora.b $1A                   ; $838767
    and.b $18                   ; $838769
    sta.b $1A                   ; $83876B
    lda.b $22                   ; $83876D
    tax                         ; $83876F
    eor.b $1C                   ; $838770
    stx.b $1C                   ; $838772
    ora.b $1E                   ; $838774
    and.b $1C                   ; $838776
    sta.b $1E                   ; $838778
    lda.b $0C                   ; $83877A
    bit.w #$000F                ; $83877C
    beq CODE_838788             ; $83877F
    lda.w #$0001                ; $838781
    sta.b $0C                   ; $838784
    sta.b $0E                   ; $838786
CODE_838788:
    lda.b $10                   ; $838788
    bit.w #$000F                ; $83878A
    beq CODE_838796             ; $83878D
    incbin !baserom:$1878F..$18796
CODE_838796:
    lda.b $14                   ; $838796
    bit.w #$000F                ; $838798
    beq CODE_8387A4             ; $83879B
    lda.w #$0001                ; $83879D
    sta.b $14                   ; $8387A0
    sta.b $16                   ; $8387A2
CODE_8387A4:
    lda.b $18                   ; $8387A4
    bit.w #$000F                ; $8387A6
    beq CODE_8387B2             ; $8387A9
    lda.w #$0001                ; $8387AB
    sta.b $18                   ; $8387AE
    sta.b $1A                   ; $8387B0
CODE_8387B2:
    lda.b $1C                   ; $8387B2
    bit.w #$000F                ; $8387B4
    beq CODE_8387C0             ; $8387B7
    lda.w #$0001                ; $8387B9
    sta.b $1C                   ; $8387BC
    sta.b $1E                   ; $8387BE
CODE_8387C0:
    rts                         ; $8387C0
    lda.w $4017                 ; $8387C1
    lsr a                       ; $8387C4
    rol.b $21                   ; $8387C5
    lsr a                       ; $8387C7
    rol.b $23                   ; $8387C8
    lda.w $4017                 ; $8387CA
    lsr a                       ; $8387CD
    rol.b $21                   ; $8387CE
    lsr a                       ; $8387D0
    rol.b $23                   ; $8387D1
    lda.w $4017                 ; $8387D3
    lsr a                       ; $8387D6
    rol.b $21                   ; $8387D7
    lsr a                       ; $8387D9
    rol.b $23                   ; $8387DA
    lda.w $4017                 ; $8387DC
    lsr a                       ; $8387DF
    rol.b $21                   ; $8387E0
    lsr a                       ; $8387E2
    rol.b $23                   ; $8387E3
    lda.w $4017                 ; $8387E5
    lsr a                       ; $8387E8
    rol.b $21                   ; $8387E9
    lsr a                       ; $8387EB
    rol.b $23                   ; $8387EC
    lda.w $4017                 ; $8387EE
    lsr a                       ; $8387F1
    rol.b $21                   ; $8387F2
    lsr a                       ; $8387F4
    rol.b $23                   ; $8387F5
    lda.w $4017                 ; $8387F7
    lsr a                       ; $8387FA
    rol.b $21                   ; $8387FB
    lsr a                       ; $8387FD
    rol.b $23                   ; $8387FE
    lda.w $4017                 ; $838800
    lsr a                       ; $838803
    rol.b $21                   ; $838804
    lsr a                       ; $838806
    rol.b $23                   ; $838807
    lda.w $4017                 ; $838809
    lsr a                       ; $83880C
    rol.b $20                   ; $83880D
    lsr a                       ; $83880F
    rol.b $22                   ; $838810
    lda.w $4017                 ; $838812
    lsr a                       ; $838815
    rol.b $20                   ; $838816
    lsr a                       ; $838818
    rol.b $22                   ; $838819
    lda.w $4017                 ; $83881B
    lsr a                       ; $83881E
    rol.b $20                   ; $83881F
    lsr a                       ; $838821
    rol.b $22                   ; $838822
    lda.w $4017                 ; $838824
    lsr a                       ; $838827
    rol.b $20                   ; $838828
    lsr a                       ; $83882A
    rol.b $22                   ; $83882B
    lda.w $4017                 ; $83882D
    lsr a                       ; $838830
    rol.b $20                   ; $838831
    lsr a                       ; $838833
    rol.b $22                   ; $838834
    lda.w $4017                 ; $838836
    lsr a                       ; $838839
    rol.b $20                   ; $83883A
    lsr a                       ; $83883C
    rol.b $22                   ; $83883D
    lda.w $4017                 ; $83883F
    lsr a                       ; $838842
    rol.b $20                   ; $838843
    lsr a                       ; $838845
    rol.b $22                   ; $838846
    lda.w $4017                 ; $838848
    lsr a                       ; $83884B
    rol.b $20                   ; $83884C
    lsr a                       ; $83884E
    rol.b $22                   ; $83884F
    lda.w $4017                 ; $838851
    eor.b #$03                  ; $838854
    lsr a                       ; $838856
    tax                         ; $838857
    lda.b #$00                  ; $838858
    rol a                       ; $83885A
    ora.b $20                   ; $83885B
    sta.b $20                   ; $83885D
    txa                         ; $83885F
    lsr a                       ; $838860
    tax                         ; $838861
    lda.b #$00                  ; $838862
    rol a                       ; $838864
    ora.b $22                   ; $838865
    sta.b $22                   ; $838867
    rts                         ; $838869
    incbin !baserom:$1886A..$18989
    ldx.b $E2                   ; $838989
    lda.w $0026                 ; $83898B
    bit.w #$0010                ; $83898E
    bne CODE_8389AE             ; $838991
    ldy.w #$0000                ; $838993
    lda.w $01CC                 ; $838996
    bpl CODE_83899C             ; $838999
    dey                         ; $83899B
CODE_83899C:
    clc                         ; $83899C
    adc.w $00D4                 ; $83899D
    sta.w $00D4                 ; $8389A0
    sep.b #$20                  ; $8389A3
    tya                         ; $8389A5
    adc.w $00D6                 ; $8389A6
    sta.w $00D6                 ; $8389A9
    rep.b #$20                  ; $8389AC
CODE_8389AE:
    lda.w $00D5                 ; $8389AE
    clc                         ; $8389B1
    adc.w #$003C                ; $8389B2
    sta.w $1536,x               ; $8389B5
    sta.w $1556,x               ; $8389B8
    lda.w $1F32                 ; $8389BB
    bpl CODE_8389D0             ; $8389BE
    incbin !baserom:$189C0..$189D0
CODE_8389D0:
    rts                         ; $8389D0
    incbin !baserom:$189D1..$196CD
    jsr.w $86FF                 ; $8396CD
    rtl                         ; $8396D0
    jsr.w $8644                 ; $8396D1
    rtl                         ; $8396D4
    jsr.w $8989                 ; $8396D5
    rtl                         ; $8396D8
    incbin !baserom:$196D9..$1E241
    lda.w $1E9E                 ; $83E241
    asl a                       ; $83E244
    tax                         ; $83E245
    lda.l $7E4CDD,x             ; $83E246
    inc a                       ; $83E24A
    sta.l $7E4CDD,x             ; $83E24B
    jsr.w $E2D7                 ; $83E24F
    phd                         ; $83E252
    lda.w #$0200                ; $83E253
    tcd                         ; $83E256
    jsr.w $EAC6                 ; $83E257
    jsr.w $F7F9                 ; $83E25A
    pld                         ; $83E25D
    rtl                         ; $83E25E
    incbin !baserom:$1E25F..$1E2D7
    lda.w #$8000                ; $83E2D7
    sta.w $1EBE                 ; $83E2DA
    lda.w $0395                 ; $83E2DD
    and.w #$00FF                ; $83E2E0
    sta.w $1EB2                 ; $83E2E3
    and.w #$007F                ; $83E2E6
    asl a                       ; $83E2E9
    tax                         ; $83E2EA
    lda.w $021F                 ; $83E2EB
    asl a                       ; $83E2EE
    clc                         ; $83E2EF
    adc.l $86DAD9,x             ; $83E2F0
    tax                         ; $83E2F4
    jsr.w $EA9B                 ; $83E2F5
    sta.w $022B                 ; $83E2F8
    lda.w $0393                 ; $83E2FB
    and.w #$00FF                ; $83E2FE
    asl a                       ; $83E301
    tax                         ; $83E302
    lda.l $86DCE1,x             ; $83E303
    sta.w $1EA8                 ; $83E307
    lda.l $86DCED,x             ; $83E30A
    sta.w $1EC0                 ; $83E30E
    lda.w $0397                 ; $83E311
    and.w #$00FF                ; $83E314
    asl a                       ; $83E317
    tax                         ; $83E318
    lda.l $86DD29,x             ; $83E319
    sta.w $1EB4                 ; $83E31D
    lda.l $86DD35,x             ; $83E320
    sta.w $1EB6                 ; $83E324
    lda.w $0394                 ; $83E327
    and.w #$00FF                ; $83E32A
    asl a                       ; $83E32D
    tax                         ; $83E32E
    lda.l $86DD41,x             ; $83E32F
    sta.w $1EB8                 ; $83E333
    lda.w $0396                 ; $83E336
    and.w #$00FF                ; $83E339
    asl a                       ; $83E33C
    tax                         ; $83E33D
    lda.l $86DD4D,x             ; $83E33E
    sta.w $1F30                 ; $83E342
    stz.w $1ECA                 ; $83E345
    lda.w #$0000                ; $83E348
    sta.w $1EBA                 ; $83E34B
    stz.w $1ECC                 ; $83E34E
    lda.w $1E66                 ; $83E351
    sta.w $1F2C                 ; $83E354
    lda.w $0226                 ; $83E357
    asl a                       ; $83E35A
    and.w #$003E                ; $83E35B
    clc                         ; $83E35E
    adc.w #$0000                ; $83E35F
    sta.w $1EBC                 ; $83E362
    stz.w $1E9C                 ; $83E365
    lda.w $0244                 ; $83E368
    bit.w #$0004                ; $83E36B
    db $D0,$09 ; bne            ; $83E36E
    lda.l $8103DC               ; $83E370
    bit.w #$0010                ; $83E374
    bne CODE_83E383             ; $83E377
    incbin !baserom:$1E379..$1E383
CODE_83E383:
    lda.w $000C                 ; $83E383
    bit.w $039D                 ; $83E386
    beq CODE_83E391             ; $83E389
    incbin !baserom:$1E38B..$1E391
CODE_83E391:
    ldx.w #$0058                ; $83E391
    lda.w $000C                 ; $83E394
    bit.w $039F                 ; $83E397
    beq CODE_83E3A5             ; $83E39A
    incbin !baserom:$1E39C..$1E3A5
CODE_83E3A5:
    txa                         ; $83E3A5
    sta.l $7E2142               ; $83E3A6
    lda.w $000C                 ; $83E3AA
    bit.w #$0200                ; $83E3AD
    beq CODE_83E3B8             ; $83E3B0
    incbin !baserom:$1E3B2..$1E3B8
CODE_83E3B8:
    lda.w $000C                 ; $83E3B8
    bit.w #$0100                ; $83E3BB
    beq CODE_83E3C6             ; $83E3BE
    incbin !baserom:$1E3C0..$1E3C6
CODE_83E3C6:
    lda.w $03A1                 ; $83E3C6
    trb.w $000E                 ; $83E3C9
    beq CODE_83E3DA             ; $83E3CC
    incbin !baserom:$1E3CE..$1E3DA
CODE_83E3DA:
    lda.w $03A1                 ; $83E3DA
    bit.w $000C                 ; $83E3DD
    beq CODE_83E3EE             ; $83E3E0
    incbin !baserom:$1E3E2..$1E3EE
CODE_83E3EE:
    lda.w $03A5                 ; $83E3EE
    trb.w $000E                 ; $83E3F1
    beq CODE_83E407             ; $83E3F4
    incbin !baserom:$1E3F6..$1E407
CODE_83E407:
    lda.w $03A3                 ; $83E407
    trb.w $000E                 ; $83E40A
    beq CODE_83E422             ; $83E40D
    incbin !baserom:$1E40F..$1E422
CODE_83E422:
    ldx.b $4C                   ; $83E422
    lda.w $024E,x               ; $83E424
    and.w #$00FF                ; $83E427
    sta.w $025C                 ; $83E42A
    stx.w $1F2E                 ; $83E42D
    rts                         ; $83E430
    incbin !baserom:$1E431..$1EA9B
    lda.l $86DB47,x             ; $83EA9B
    sta.w $1EA2                 ; $83EA9F
    lda.l $86DB99,x             ; $83EAA2
    sta.w $1EA4                 ; $83EAA6
    lda.l $86DBEB,x             ; $83EAA9
    sta.w $1EA6                 ; $83EAAD
    lda.l $86DC3D,x             ; $83EAB0
    sta.w $1EAA                 ; $83EAB4
    lda.l $86DC8F,x             ; $83EAB7
    sta.w $1EAC                 ; $83EABB
    lda.l $86DAF5,x             ; $83EABE
    sta.w $1EA0                 ; $83EAC2
    rts                         ; $83EAC5
    pea.w $8330                 ; $83EAC6
    plb                         ; $83EAC9
    lda.w $1ECE                 ; $83EACA
    beq CODE_83EB31             ; $83EACD
    bpl CODE_83EAD4             ; $83EACF
    jsr.w $F73E                 ; $83EAD1
CODE_83EAD4:
    lda.w $1E9C                 ; $83EAD4
    bit.w #$0001                ; $83EAD7
    beq CODE_83EB0A             ; $83EADA
    incbin !baserom:$1EADC..$1EB0A
CODE_83EB0A:
    stz.w $8000                 ; $83EB0A
    lda.w #$FFE0                ; $83EB0D
    sta.w $8000                 ; $83EB10
    lda.w $1E9E                 ; $83EB13
    xba                         ; $83EB16
    sta.w $8000                 ; $83EB17
    lda.b $12                   ; $83EB1A
    clc                         ; $83EB1C
    adc.w $8000                 ; $83EB1D
    sta.b $12                   ; $83EB20
    lda.b $14                   ; $83EB22
    adc.w $8000                 ; $83EB24
    sta.b $14                   ; $83EB27
    bpl CODE_83EB2F             ; $83EB29
    stz.b $12                   ; $83EB2B
    stz.b $14                   ; $83EB2D
CODE_83EB2F:
    plb                         ; $83EB2F
    rts                         ; $83EB30
CODE_83EB31:
    stz.w $013A                 ; $83EB31
    stz.w $013C                 ; $83EB34
    lda.w #$0010                ; $83EB37
    bit.w $1E9C                 ; $83EB3A
    beq CODE_83EB6B             ; $83EB3D
    incbin !baserom:$1EB3F..$1EB6B
CODE_83EB6B:
    lda.b $31                   ; $83EB6B
    sec                         ; $83EB6D
    sbc.w $1E9E                 ; $83EB6E
    bpl CODE_83EB76             ; $83EB71
    lda.w #$0000                ; $83EB73
CODE_83EB76:
    sta.b $31                   ; $83EB76
    lda.w #$0080                ; $83EB78
    trb.w $1E9C                 ; $83EB7B
    beq CODE_83EBC3             ; $83EB7E
    incbin !baserom:$1EB80..$1EBC3
CODE_83EBC3:
    lda.w #$0020                ; $83EBC3
    trb.w $1E9C                 ; $83EBC6
    beq CODE_83EBED             ; $83EBC9
    incbin !baserom:$1EBCB..$1EBED
CODE_83EBED:
    bit.w $1F31                 ; $83EBED
    bmi CODE_83EC1D             ; $83EBF0
    lda.b $7E                   ; $83EBF2
    bmi CODE_83EC1D             ; $83EBF4
    incbin !baserom:$1EBF6..$1EC1D
CODE_83EC1D:
    lda.w #$0040                ; $83EC1D
    bit.w $1E9C                 ; $83EC20
    beq CODE_83EC30             ; $83EC23
    incbin !baserom:$1EC25..$1EC30
CODE_83EC30:
    jmp.w $ECF6                 ; $83EC30
    incbin !baserom:$1EC33..$1ECF6
    stz.w $8000                 ; $83ECF6
    lda.b $16                   ; $83ECF9
    sta.w $8000                 ; $83ECFB
    lda.w $1E9E                 ; $83ECFE
    sta.w $8000                 ; $83ED01
    nop                         ; $83ED04
    lda.w $8000                 ; $83ED05
    sta.w $0136                 ; $83ED08
    lda.w $8000                 ; $83ED0B
    sta.w $0138                 ; $83ED0E
    stz.w $8000                 ; $83ED11
    lda.b $18                   ; $83ED14
    bit.b $16                   ; $83ED16
    bpl CODE_83ED1B             ; $83ED18
    incbin !baserom:$1ED1A..$1ED1B
CODE_83ED1B:
    sta.w $8000                 ; $83ED1B
    lda.w $1E9E                 ; $83ED1E
    sta.w $8000                 ; $83ED21
    lda.w $0138                 ; $83ED24
    clc                         ; $83ED27
    adc.w $8000                 ; $83ED28
    sta.w $0138                 ; $83ED2B
    lda.w $8000                 ; $83ED2E
    bit.w $1F31                 ; $83ED31
    bmi CODE_83ED84             ; $83ED34
    lda.b $58                   ; $83ED36
    beq CODE_83ED84             ; $83ED38
    lda.w #$0000                ; $83ED3A
    sta.l $308000               ; $83ED3D
    lda.b $17                   ; $83ED41
    sta.l $308000               ; $83ED43
    lda.w #$3000                ; $83ED47
    sta.l $308000               ; $83ED4A
    nop                         ; $83ED4E
    lda.l $308000               ; $83ED4F
    lda.l $308000               ; $83ED53
    tax                         ; $83ED57
    lda.w #$0000                ; $83ED58
    sta.l $308000               ; $83ED5B
    lda.w $1E9E                 ; $83ED5F
    sta.l $308000               ; $83ED62
    txa                         ; $83ED66
    sta.l $308000               ; $83ED67
    lda.b $56                   ; $83ED6B
    sec                         ; $83ED6D
    sbc.l $308000               ; $83ED6E
    sta.l $308000               ; $83ED72
    sta.b $56                   ; $83ED76
    lda.b $58                   ; $83ED78
    sbc.w #$0000                ; $83ED7A
    bpl CODE_83ED82             ; $83ED7D
    incbin !baserom:$1ED7F..$1ED82
CODE_83ED82:
    sta.b $58                   ; $83ED82
CODE_83ED84:
    lda.w $1E9E                 ; $83ED84
    sta.w $0134                 ; $83ED87
    lda.w $1E9C                 ; $83ED8A
    bit.w #$0001                ; $83ED8D
    beq CODE_83EDF9             ; $83ED90
    incbin !baserom:$1ED92..$1EDF9
CODE_83EDF9:
    stz.w $8000                 ; $83EDF9
    lda.b $17                   ; $83EDFC
    sta.w $8000                 ; $83EDFE
    lda.b $17                   ; $83EE01
    sta.w $8000                 ; $83EE03
    nop                         ; $83EE06
    lda.w $8000                 ; $83EE07
    ldx.w $8000                 ; $83EE0A
    and.w #$FFFF                ; $83EE0D
    bpl CODE_83EE13             ; $83EE10
    incbin !baserom:$1EE12..$1EE13
CODE_83EE13:
    stz.w $8000                 ; $83EE13
    nop                         ; $83EE16
    sta.w $8000                 ; $83EE17
    lda.w $1EA4                 ; $83EE1A
    sec                         ; $83EE1D
    sbc.w $1EC0                 ; $83EE1E
    sta.w $8000                 ; $83EE21
    ldy.w #$0000                ; $83EE24
    lda.w $8000                 ; $83EE27
    sta.w $0114                 ; $83EE2A
    lda.w $8000                 ; $83EE2D
    sta.w $0116                 ; $83EE30
    bpl CODE_83EE36             ; $83EE33
    incbin !baserom:$1EE35..$1EE36
CODE_83EE36:
    sty.w $0118                 ; $83EE36
    stz.w $8000                 ; $83EE39
    nop                         ; $83EE3C
    nop                         ; $83EE3D
    stx.w $8000                 ; $83EE3E
    lda.w $1EA4                 ; $83EE41
    sec                         ; $83EE44
    sbc.w $1EC0                 ; $83EE45
    sta.w $8000                 ; $83EE48
    lda.w $0116                 ; $83EE4B
    clc                         ; $83EE4E
    adc.w $8000                 ; $83EE4F
    sta.w $0116                 ; $83EE52
    lda.w $0118                 ; $83EE55
    adc.w $8000                 ; $83EE58
    sta.w $0118                 ; $83EE5B
    stz.w $011A                 ; $83EE5E
    lda.b $16                   ; $83EE61
    sec                         ; $83EE63
    sbc.w $0117                 ; $83EE64
    sta.b $16                   ; $83EE67
    lda.b $18                   ; $83EE69
    sbc.w $0119                 ; $83EE6B
    sta.b $18                   ; $83EE6E
    stz.w $8000                 ; $83EE70
    lda.w $1EA4                 ; $83EE73
    sec                         ; $83EE76
    sbc.w $1EC0                 ; $83EE77
    sta.w $8000                 ; $83EE7A
    lda.w #$0001                ; $83EE7D
    sta.w $8000                 ; $83EE80
    lda.b $16                   ; $83EE83
    sec                         ; $83EE85
    sbc.w $8000                 ; $83EE86
    sta.b $16                   ; $83EE89
    lda.b $18                   ; $83EE8B
    sbc.w $8000                 ; $83EE8D
    sta.b $18                   ; $83EE90
    bpl CODE_83EE98             ; $83EE92
    stz.b $16                   ; $83EE94
    stz.b $18                   ; $83EE96
CODE_83EE98:
    dec.w $0134                 ; $83EE98
    beq CODE_83EEA0             ; $83EE9B
    jmp.w $ED8A                 ; $83EE9D
CODE_83EEA0:
    lda.b $44                   ; $83EEA0
    bit.w #$0010                ; $83EEA2
    bne CODE_83EEE7             ; $83EEA5
    ldx.w #$FFC0                ; $83EEA7
    lda.w $1E9C                 ; $83EEAA
    bit.w #$0001                ; $83EEAD
    beq CODE_83EEB5             ; $83EEB0
    incbin !baserom:$1EEB2..$1EEB5
CODE_83EEB5:
    stz.w $8000                 ; $83EEB5
    nop                         ; $83EEB8
    nop                         ; $83EEB9
    stx.w $8000                 ; $83EEBA
    lda.w $1E9E                 ; $83EEBD
    xba                         ; $83EEC0
    sta.w $8000                 ; $83EEC1
    lda.b $12                   ; $83EEC4
    clc                         ; $83EEC6
    adc.w $8000                 ; $83EEC7
    sta.b $12                   ; $83EECA
    lda.b $14                   ; $83EECC
    adc.w $8000                 ; $83EECE
    sta.b $14                   ; $83EED1
    bpl CODE_83EEDC             ; $83EED3
    stz.b $14                   ; $83EED5
    stz.b $12                   ; $83EED7
    lda.w #$0000                ; $83EED9
CODE_83EEDC:
    cmp.b $18                   ; $83EEDC
    beq CODE_83EEE2             ; $83EEDE
    incbin !baserom:$1EEE0..$1EEE2
CODE_83EEE2:
    lda.w #$0010                ; $83EEE2
    tsb.b $44                   ; $83EEE5
CODE_83EEE7:
    lda.b $16                   ; $83EEE7
    sta.b $12                   ; $83EEE9
    lda.b $18                   ; $83EEEB
    sta.b $14                   ; $83EEED
    stz.w $8000                 ; $83EEEF
    lda.b $17                   ; $83EEF2
    sta.w $8000                 ; $83EEF4
    lda.w $1EA0                 ; $83EEF7
    lsr a                       ; $83EEFA
    sta.w $8000                 ; $83EEFB
    nop                         ; $83EEFE
    lda.w $8000                 ; $83EEFF
    asl a                       ; $83EF02
    sta.b $0E                   ; $83EF03
    lda.w $8000                 ; $83EF05
    rol a                       ; $83EF08
    sta.b $10                   ; $83EF09
    lda.b $17                   ; $83EF0B
    cmp.w $1EAA                 ; $83EF0D
    bcc CODE_83EF16             ; $83EF10
    incbin !baserom:$1EF12..$1EF16
CODE_83EF16:
    lda.b $17                   ; $83EF16
    cmp.w $1EAC                 ; $83EF18
    bcs CODE_83EF6E             ; $83EF1B
    incbin !baserom:$1EF1D..$1EF6E
CODE_83EF6E:
    stz.w $8000                 ; $83EF6E
    lda.b $10                   ; $83EF71
    sta.w $8000                 ; $83EF73
    sta.w $8000                 ; $83EF76
    nop                         ; $83EF79
    lda.w $8000                 ; $83EF7A
    and.w #$FF00                ; $83EF7D
    xba                         ; $83EF80
    sta.w $0114                 ; $83EF81
    ldx.w #$7FFF                ; $83EF84
    lda.w $8000                 ; $83EF87
    cmp.w #$0100                ; $83EF8A
    bcs CODE_83EF97             ; $83EF8D
    and.w #$00FF                ; $83EF8F
    xba                         ; $83EF92
    ora.w $0114                 ; $83EF93
    tax                         ; $83EF96
CODE_83EF97:
    stz.w $8000                 ; $83EF97
    lda.w #$0000                ; $83EF9A
    sta.w $8000                 ; $83EF9D
    stx.w $8000                 ; $83EFA0
    lda.w $013A                 ; $83EFA3
    clc                         ; $83EFA6
    adc.w $8000                 ; $83EFA7
    sta.w $013A                 ; $83EFAA
    lda.w $013C                 ; $83EFAD
    adc.w $8000                 ; $83EFB0
    sta.w $013C                 ; $83EFB3
    lda.b $10                   ; $83EFB6
    cmp.w #$0010                ; $83EFB8
    bcc CODE_83EFD8             ; $83EFBB
    incbin !baserom:$1EFBD..$1EFD8
CODE_83EFD8:
    bit.w $1F31                 ; $83EFD8
    bmi CODE_83F004             ; $83EFDB
    lda.b $58                   ; $83EFDD
    bne CODE_83F004             ; $83EFDF
    incbin !baserom:$1EFE1..$1F004
CODE_83F004:
    bit.w $1F31                 ; $83F004
    bmi CODE_83F05B             ; $83F007
    ldx.w $1EBC                 ; $83F009
    lda.l $7E3D6E,x             ; $83F00C
    bit.w #$0008                ; $83F010
    beq CODE_83F01A             ; $83F013
    incbin !baserom:$1F015..$1F01A
CODE_83F01A:
    lda.w #$0000                ; $83F01A
    sta.l $308000               ; $83F01D
    lda.l $7E3DEE,x             ; $83F021
    sec                         ; $83F025
    sbc.l $7E3DEC,x             ; $83F026
    asl a                       ; $83F02A
    sta.l $308000               ; $83F02B
    lda.w $0224                 ; $83F02F
    and.w #$FF00                ; $83F032
    lsr a                       ; $83F035
    sta.l $308000               ; $83F036
    lda.l $7E3DEC,x             ; $83F03A
    clc                         ; $83F03E
    sta.l $308000               ; $83F03F
    adc.l $308000               ; $83F043
    eor.w #$FFFF                ; $83F047
    inc a                       ; $83F04A
    sta.b $9E                   ; $83F04B
    stz.b $9C                   ; $83F04D
    lda.w $1F2C                 ; $83F04F
    bit.w #$8001                ; $83F052
    beq CODE_83F05B             ; $83F055
    incbin !baserom:$1F057..$1F05B
CODE_83F05B:
    ldx.w $1EBC                 ; $83F05B
    ldy.w #$0000                ; $83F05E
    lda.l $7E3BEC,x             ; $83F061
    beq CODE_83F06A             ; $83F065
    incbin !baserom:$1F067..$1F06A
CODE_83F06A:
    lda.b $06                   ; $83F06A
    bpl CODE_83F072             ; $83F06C
    incbin !baserom:$1F06E..$1F072
CODE_83F072:
    sec                         ; $83F072
    sbc.w $1F28,y               ; $83F073
    bcc CODE_83F0C5             ; $83F076
    incbin !baserom:$1F078..$1F0C5
CODE_83F0C5:
    bit.w $1F31                 ; $83F0C5
    bmi CODE_83F0F1             ; $83F0C8
    lda.b $10                   ; $83F0CA
    cmp.w #$0010                ; $83F0CC
    bcc CODE_83F0F1             ; $83F0CF
    incbin !baserom:$1F0D1..$1F0F1
CODE_83F0F1:
    lda.w $1E9C                 ; $83F0F1
    bit.w #$0002                ; $83F0F4
    beq CODE_83F115             ; $83F0F7
    incbin !baserom:$1F0F9..$1F115
CODE_83F115:
    stz.w $8000                 ; $83F115
    lda.w $013C                 ; $83F118
    sta.w $8000                 ; $83F11B
    lda.w $1EA2                 ; $83F11E
    sta.w $8000                 ; $83F121
    nop                         ; $83F124
    lda.b $16                   ; $83F125
    sec                         ; $83F127
    sbc.w $8000                 ; $83F128
    sta.b $16                   ; $83F12B
    lda.b $18                   ; $83F12D
    sbc.w $8000                 ; $83F12F
    bpl CODE_83F139             ; $83F132
    incbin !baserom:$1F134..$1F139
CODE_83F139:
    sta.b $18                   ; $83F139
    ldx.w #$000C                ; $83F13B
    lda.b $21                   ; $83F13E
    beq CODE_83F186             ; $83F140
    incbin !baserom:$1F142..$1F186
CODE_83F186:
    stz.w $013C                 ; $83F186
    lda.w $1E9C                 ; $83F189
    bit.w #$0008                ; $83F18C
    beq CODE_83F196             ; $83F18F
    incbin !baserom:$1F191..$1F196
CODE_83F196:
    lda.w $1E9C                 ; $83F196
    bit.w #$0004                ; $83F199
    beq CODE_83F1EC             ; $83F19C
    incbin !baserom:$1F19E..$1F1EC
CODE_83F1EC:
    lda.b $1C                   ; $83F1EC
    beq CODE_83F23F             ; $83F1EE
    incbin !baserom:$1F1F0..$1F23F
CODE_83F23F:
    lda.b $44                   ; $83F23F
    bit.w #$0020                ; $83F241
    db $D0,$07 ; bne            ; $83F244
    lda.b $44                   ; $83F246
    bit.w #$0001                ; $83F248
    beq CODE_83F250             ; $83F24B
    incbin !baserom:$1F24D..$1F250
CODE_83F250:
    stz.w $8000                 ; $83F250
    lda.b $0E                   ; $83F253
    sta.w $8000                 ; $83F255
    lda.w $1EB8                 ; $83F258
    sta.w $8000                 ; $83F25B
    ldx.w #$0000                ; $83F25E
    lda.w $8000                 ; $83F261
    sta.w $0114                 ; $83F264
    lda.w $8000                 ; $83F267
    sta.w $0116                 ; $83F26A
    bpl CODE_83F270             ; $83F26D
    incbin !baserom:$1F26F..$1F270
CODE_83F270:
    stx.w $0118                 ; $83F270
    stz.w $8000                 ; $83F273
    lda.b $10                   ; $83F276
    bit.b $0E                   ; $83F278
    bpl CODE_83F27D             ; $83F27A
    incbin !baserom:$1F27C..$1F27D
CODE_83F27D:
    sta.w $8000                 ; $83F27D
    lda.w $1EB8                 ; $83F280
    sta.w $8000                 ; $83F283
    lda.w $0116                 ; $83F286
    clc                         ; $83F289
    adc.w $8000                 ; $83F28A
    sta.w $0116                 ; $83F28D
    lda.w $0118                 ; $83F290
    adc.w $8000                 ; $83F293
    sta.w $0118                 ; $83F296
    lda.b $21                   ; $83F299
    bpl CODE_83F2A8             ; $83F29B
    incbin !baserom:$1F29D..$1F2A8
CODE_83F2A8:
    sec                         ; $83F2A8
    sbc.w $0117                 ; $83F2A9
    bpl CODE_83F2B1             ; $83F2AC
    incbin !baserom:$1F2AE..$1F2B1
CODE_83F2B1:
    stz.w $8000                 ; $83F2B1
    nop                         ; $83F2B4
    nop                         ; $83F2B5
    sta.w $8000                 ; $83F2B6
    lda.w $1E9E                 ; $83F2B9
    sta.w $8000                 ; $83F2BC
    nop                         ; $83F2BF
    ldx.w $8000                 ; $83F2C0
    bit.w $8000                 ; $83F2C3
    stz.w $8000                 ; $83F2C6
    lda.b $0F                   ; $83F2C9
    lsr a                       ; $83F2CB
    lsr a                       ; $83F2CC
    sta.w $8000                 ; $83F2CD
    stx.w $8000                 ; $83F2D0
    nop                         ; $83F2D3
    lda.w $8000                 ; $83F2D4
    sta.w $0114                 ; $83F2D7
    lda.w $8000                 ; $83F2DA
    sta.w $0116                 ; $83F2DD
    lda.b $04                   ; $83F2E0
    clc                         ; $83F2E2
    adc.w $0114                 ; $83F2E3
    sta.b $04                   ; $83F2E6
    lda.b $06                   ; $83F2E8
    adc.w $0116                 ; $83F2EA
    sta.b $06                   ; $83F2ED
    lda.b $1C                   ; $83F2EF
    bne CODE_83F314             ; $83F2F1
    lda.b $28                   ; $83F2F3
    and.w #$FF00                ; $83F2F5
    sta.w $0118                 ; $83F2F8
    clc                         ; $83F2FB
    adc.b $04                   ; $83F2FC
    sta.b $04                   ; $83F2FE
    lda.b $2A                   ; $83F300
    and.w #$00FF                ; $83F302
    bit.w #$0080                ; $83F305
    beq CODE_83F30D             ; $83F308
    incbin !baserom:$1F30A..$1F30D
CODE_83F30D:
    sta.w $011A                 ; $83F30D
    adc.b $06                   ; $83F310
    sta.b $06                   ; $83F312
CODE_83F314:
    lda.b $44                   ; $83F314
    and.w #$0010                ; $83F316
    db $F0,$14 ; beq            ; $83F319
    lda.b $29                   ; $83F31B
    beq CODE_83F35A             ; $83F31D
    incbin !baserom:$1F31F..$1F35A
CODE_83F35A:
    stz.b $46                   ; $83F35A
    bit.w $1F31                 ; $83F35C
    bpl CODE_83F364             ; $83F35F
    incbin !baserom:$1F361..$1F364
CODE_83F364:
    lda.b $06                   ; $83F364
    bpl CODE_83F36C             ; $83F366
    incbin !baserom:$1F368..$1F36C
CODE_83F36C:
    sec                         ; $83F36C
    sbc.w #$0280                ; $83F36D
    bcc CODE_83F3B0             ; $83F370
    incbin !baserom:$1F372..$1F3B0
CODE_83F3B0:
    lda.b $60                   ; $83F3B0
    and.w #$00FF                ; $83F3B2
    tax                         ; $83F3B5
    jmp.w ($F3B9,x)             ; $83F3B6
    incbin !baserom:$1F3B9..$1F423
    lda.l $8103DC               ; $83F423
    and.w #$0003                ; $83F427
    cmp.w #$0002                ; $83F42A
    beq CODE_83F48E             ; $83F42D
    lda.b $1A                   ; $83F42F
    beq CODE_83F48E             ; $83F431
    incbin !baserom:$1F433..$1F48E
CODE_83F48E:
    lda.b $21                   ; $83F48E
    ldx.w #$0006                ; $83F490
    cmp.w #$FFA0                ; $83F493
    bmi CODE_83F4B7             ; $83F496
    dex                         ; $83F498
    cmp.w #$FFC0                ; $83F499
    bmi CODE_83F4B7             ; $83F49C
    dex                         ; $83F49E
    cmp.w #$FFE8                ; $83F49F
    bmi CODE_83F4B7             ; $83F4A2
    dex                         ; $83F4A4
    cmp.w #$0018                ; $83F4A5
    bmi CODE_83F4B7             ; $83F4A8
    incbin !baserom:$1F4AA..$1F4B7
CODE_83F4B7:
    txa                         ; $83F4B7
    xba                         ; $83F4B8
    sta.b $3A                   ; $83F4B9
    lda.b $3B                   ; $83F4BB
    and.w #$00FF                ; $83F4BD
    asl a                       ; $83F4C0
    tax                         ; $83F4C1
    lda.l $86CB54,x             ; $83F4C2
    ldx.w $1EBA                 ; $83F4C6
    sta.l $7EFABF,x             ; $83F4C9
    lda.w #$0000                ; $83F4CD
    sta.l $308000               ; $83F4D0
    lda.w $1E9E                 ; $83F4D4
    sta.l $308000               ; $83F4D7
    lda.w #$001E                ; $83F4DB
    sta.l $308000               ; $83F4DE
    nop                         ; $83F4E2
    nop                         ; $83F4E3
    lda.b $48                   ; $83F4E4
    sec                         ; $83F4E6
    sbc.l $308000               ; $83F4E7
    sta.l $308000               ; $83F4EB
    bpl CODE_83F4F4             ; $83F4EF
    lda.w #$0000                ; $83F4F1
CODE_83F4F4:
    sta.b $48                   ; $83F4F4
    bit.w $1F31                 ; $83F4F6
    bmi CODE_83F50F             ; $83F4F9
    lda.b $1A                   ; $83F4FB
    and.w #$7FFF                ; $83F4FD
    dec a                       ; $83F500
    beq CODE_83F50F             ; $83F501
    cmp.w #$0005                ; $83F503
    beq CODE_83F50F             ; $83F506
    lda.b $53                   ; $83F508
    and.w #$00FF                ; $83F50A
    db $D0,$03 ; bne            ; $83F50D
CODE_83F50F:
    jmp.w $F645                 ; $83F50F
    incbin !baserom:$1F512..$1F645
    stz.b $4C                   ; $83F645
    bit.w $1F31                 ; $83F647
    bmi CODE_83F68D             ; $83F64A
    lda.w #$0000                ; $83F64C
    sta.l $308000               ; $83F64F
    lda.w $1E9E                 ; $83F653
    sta.l $308000               ; $83F656
    lda.w #$0800                ; $83F65A
    sta.l $308000               ; $83F65D
    clc                         ; $83F661
    lda.b $97                   ; $83F662
    clc                         ; $83F664
    adc.l $308000               ; $83F665
    sta.b $97                   ; $83F669
    lda.l $308000               ; $83F66B
    bcc CODE_83F68D             ; $83F66F
    lda.b $58                   ; $83F671
    beq CODE_83F68D             ; $83F673
    cmp.w #$0009                ; $83F675
    bcs CODE_83F68D             ; $83F678
    incbin !baserom:$1F67A..$1F68D
CODE_83F68D:
    lda.w #$0000                ; $83F68D
    sta.l $308000               ; $83F690
    lda.b $10                   ; $83F694
    sta.l $308000               ; $83F696
    lda.w #$0100                ; $83F69A
    sta.l $308000               ; $83F69D
    nop                         ; $83F6A1
    lda.l $308000               ; $83F6A2
    sta.w $0114                 ; $83F6A6
    lda.l $308000               ; $83F6A9
    sta.w $0116                 ; $83F6AD
    lda.w #$0000                ; $83F6B0
    sta.l $308000               ; $83F6B3
    lda.w $0115                 ; $83F6B7
    sta.l $308000               ; $83F6BA
    lda.w $1E9E                 ; $83F6BE
    sta.l $308000               ; $83F6C1
    nop                         ; $83F6C5
    lda.l $308000               ; $83F6C6
    cmp.w #$0080                ; $83F6CA
    bcc CODE_83F6D2             ; $83F6CD
    incbin !baserom:$1F6CF..$1F6D2
CODE_83F6D2:
    sep.b #$20                  ; $83F6D2
    clc                         ; $83F6D4
    adc.b $1E                   ; $83F6D5
    sta.b $1E                   ; $83F6D7
    rep.b #$20                  ; $83F6D9
    lda.l $308000               ; $83F6DB
    plb                         ; $83F6DF
    rts                         ; $83F6E0
    incbin !baserom:$1F6E1..$1F73E
    lda.w #$0001                ; $83F73E
    bit.b $44                   ; $83F741
    bne CODE_83F748             ; $83F743
    incbin !baserom:$1F745..$1F748
CODE_83F748:
    ldx.w $1EBA                 ; $83F748
    lda.l $7EFAB5,x             ; $83F74B
    sec                         ; $83F74F
    sbc.b $25                   ; $83F750
    sta.w $0114                 ; $83F752
    lda.l $7EFAB7,x             ; $83F755
    sbc.b $27                   ; $83F759
    sta.w $0116                 ; $83F75B
    lda.w $0114                 ; $83F75E
    sec                         ; $83F761
    sbc.w #$0040                ; $83F762
    sta.w $0114                 ; $83F765
    lda.w $0116                 ; $83F768
    sbc.w #$0000                ; $83F76B
    sta.w $0116                 ; $83F76E
    lda.w $0114                 ; $83F771
    lsr a                       ; $83F774
    lsr a                       ; $83F775
    lsr a                       ; $83F776
    lsr a                       ; $83F777
    clc                         ; $83F778
    adc.w #$0008                ; $83F779
    cmp.w #$0080                ; $83F77C
    bcc CODE_83F784             ; $83F77F
    lda.w #$0080                ; $83F781
CODE_83F784:
    stz.w $8000                 ; $83F784
    ldy.w $1E9E                 ; $83F787
    sty.w $8000                 ; $83F78A
    sta.w $8000                 ; $83F78D
    lda.b $25                   ; $83F790
    clc                         ; $83F792
    adc.w $8000                 ; $83F793
    sta.b $25                   ; $83F796
    lda.b $27                   ; $83F798
    adc.w $8000                 ; $83F79A
    sta.b $27                   ; $83F79D
    ldx.w $1EBA                 ; $83F79F
    lda.l $7EFAB5,x             ; $83F7A2
    sec                         ; $83F7A6
    sbc.b $25                   ; $83F7A7
    sta.w $0114                 ; $83F7A9
    lda.l $7EFAB7,x             ; $83F7AC
    sbc.b $27                   ; $83F7B0
    sta.w $0116                 ; $83F7B2
    lda.w $0114                 ; $83F7B5
    sec                         ; $83F7B8
    sbc.w #$0040                ; $83F7B9
    sta.w $0114                 ; $83F7BC
    lda.w $0116                 ; $83F7BF
    sbc.w #$0000                ; $83F7C2
    sta.w $0116                 ; $83F7C5
    ora.w $0114                 ; $83F7C8
    beq CODE_83F7CF             ; $83F7CB
    bcs CODE_83F7F8             ; $83F7CD
CODE_83F7CF:
    lda.l $7EFAB5,x             ; $83F7CF
    sec                         ; $83F7D3
    sbc.w #$0040                ; $83F7D4
    sta.b $25                   ; $83F7D7
    lda.l $7EFAB7,x             ; $83F7D9
    sbc.w #$0000                ; $83F7DD
    sta.b $27                   ; $83F7E0
    lda.w #$0001                ; $83F7E2
    trb.b $44                   ; $83F7E5
    dec.w $1ECE                 ; $83F7E7
    lda.w $1ECE                 ; $83F7EA
    cmp.w #$8000                ; $83F7ED
    bne CODE_83F7F8             ; $83F7F0
    lda.w #$0118                ; $83F7F2
    sta.w $1ECE                 ; $83F7F5
CODE_83F7F8:
    rts                         ; $83F7F8
    pea.w $8330                 ; $83F7F9
    plb                         ; $83F7FC
    lda.b $25                   ; $83F7FD
    sta.b $2D                   ; $83F7FF
    lda.b $27                   ; $83F801
    sta.b $2F                   ; $83F803
    lda.b $25                   ; $83F805
    clc                         ; $83F807
    adc.w #$0100                ; $83F808
    xba                         ; $83F80B
    and.w #$00FF                ; $83F80C
    sta.w $0114                 ; $83F80F
    lda.b $27                   ; $83F812
    adc.w #$0000                ; $83F814
    xba                         ; $83F817
    and.w #$FF00                ; $83F818
    ora.w $0114                 ; $83F81B
    sta.w $0114                 ; $83F81E
    stz.w $0120                 ; $83F821
    lda.w #$0080                ; $83F824
    trb.b $44                   ; $83F827
    beq CODE_83F839             ; $83F829
    incbin !baserom:$1F82B..$1F839
CODE_83F839:
    lda.b $44                   ; $83F839
    and.w #$0020                ; $83F83B
    cmp.w #$0020                ; $83F83E
    bne CODE_83F892             ; $83F841
    incbin !baserom:$1F843..$1F892
CODE_83F892:
    jmp.w $F8F0                 ; $83F892
    incbin !baserom:$1F895..$1F8F0
    lda.b $44                   ; $83F8F0
    bit.w #$0001                ; $83F8F2
    bne CODE_83F914             ; $83F8F5
    stz.w $8000                 ; $83F8F7
    lda.w $1E9E                 ; $83F8FA
    sta.w $8000                 ; $83F8FD
    lda.b $10                   ; $83F900
    sta.w $8000                 ; $83F902
    lda.b $25                   ; $83F905
    clc                         ; $83F907
    adc.w $8000                 ; $83F908
    sta.b $25                   ; $83F90B
    lda.b $27                   ; $83F90D
    adc.w $8000                 ; $83F90F
    sta.b $27                   ; $83F912
CODE_83F914:
    jsr.w $F9D1                 ; $83F914
    lda.b $25                   ; $83F917
    clc                         ; $83F919
    adc.w #$0100                ; $83F91A
    xba                         ; $83F91D
    and.w #$00FF                ; $83F91E
    sta.w $0116                 ; $83F921
    lda.b $27                   ; $83F924
    adc.w #$0000                ; $83F926
    xba                         ; $83F929
    and.w #$FF00                ; $83F92A
    ora.w $0116                 ; $83F92D
    sec                         ; $83F930
    sbc.w $0114                 ; $83F931
    sta.w $0116                 ; $83F934
    db $D0,$03 ; bne            ; $83F937
    jmp.w $F9C2                 ; $83F939
    incbin !baserom:$1F93C..$1F9C2
    lda.b $25                   ; $83F9C2
    sec                         ; $83F9C4
    sbc.b $2D                   ; $83F9C5
    sta.b $2D                   ; $83F9C7
    lda.b $27                   ; $83F9C9
    sbc.b $2F                   ; $83F9CB
    sta.b $2F                   ; $83F9CD
    plb                         ; $83F9CF
    rts                         ; $83F9D0
    lda.b $26                   ; $83F9D1
    cmp.w $019A                 ; $83F9D3
    bcc CODE_83FA31             ; $83F9D6
    incbin !baserom:$1F9D8..$1FA31
CODE_83FA31:
    rts                         ; $83FA31
    incbin !baserom:$1FA32..$20000
