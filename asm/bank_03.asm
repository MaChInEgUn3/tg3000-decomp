; bank $83 (file offset $018000)
org $838000

    incbin !baserom:$18000..$18644
    lda.b $FC                   ; $838644
    bmi CODE_8386B6             ; $838646
    beq CODE_8386B6             ; $838648
    ldx.b $FE                   ; $83864A
    cpx.w $0100                 ; $83864C
    beq CODE_8386B6             ; $83864F
    lda.w #$1801                ; $838651
    sta.w $4370                 ; $838654
CODE_838657:
    lda.l $7EF733,x             ; $838657
    bmi CODE_8386A8             ; $83865B
    lda.b $FC                   ; $83865D
    beq CODE_8386B4             ; $83865F
    bmi CODE_8386B4             ; $838661
    sec                         ; $838663
    sbc.l $7EF433,x             ; $838664
    db $30,$4D ; bmi            ; $838668
    sec                         ; $83866A
    sbc.w #$0064                ; $83866B
    sta.b $FC                   ; $83866E
    lda.l $7EF433,x             ; $838670
    sta.w $4375                 ; $838674
    lda.l $7EF533,x             ; $838677
    sta.w $4372                 ; $83867B
    lda.l $7EF633,x             ; $83867E
    sta.w $2116                 ; $838682
    sep.b #$20                  ; $838685
    lda.l $7EF733,x             ; $838687
    sta.w $4374                 ; $83868B
    lda.b #$80                  ; $83868E
    sta.w $420B                 ; $838690
    rep.b #$20                  ; $838693
    lda.l $7EF833,x             ; $838695
    beq CODE_8386A8             ; $838699
    pea.w $837E                 ; $83869B
    plb                         ; $83869E
    tay                         ; $83869F
    lda.l $7EF933,x             ; $8386A0
    sta.w $0000,y               ; $8386A4
    plb                         ; $8386A7
CODE_8386A8:
    inx                         ; $8386A8
    inx                         ; $8386A9
    txa                         ; $8386AA
    and.w #$00FF                ; $8386AB
    tax                         ; $8386AE
    cpx.w $0100                 ; $8386AF
    bne CODE_838657             ; $8386B2
CODE_8386B4:
    stx.b $FE                   ; $8386B4
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
    lda.w $00D5                 ; $8389C0
    lsr a                       ; $8389C3
    lsr a                       ; $8389C4
    lsr a                       ; $8389C5
    eor.w #$FFFF                ; $8389C6
    sec                         ; $8389C9
    adc.w $00D5                 ; $8389CA
    sta.w $1536,x               ; $8389CD
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
    bne CODE_83E379             ; $83E36E
    lda.l $8103DC               ; $83E370
    bit.w #$0010                ; $83E374
    bne CODE_83E383             ; $83E377
CODE_83E379:
    phd                         ; $83E379
    lda.w #$0200                ; $83E37A
    tcd                         ; $83E37D
    jsr.w $E8AB                 ; $83E37E
    pld                         ; $83E381
    rts                         ; $83E382
CODE_83E383:
    lda.w $000C                 ; $83E383
    bit.w $039D                 ; $83E386
    beq CODE_83E391             ; $83E389
    lda.w #$0001                ; $83E38B
    tsb.w $1E9C                 ; $83E38E
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
    lda.w #$0004                ; $83E3B2
    tsb.w $1E9C                 ; $83E3B5
CODE_83E3B8:
    lda.w $000C                 ; $83E3B8
    bit.w #$0100                ; $83E3BB
    beq CODE_83E3C6             ; $83E3BE
    lda.w #$0008                ; $83E3C0
    tsb.w $1E9C                 ; $83E3C3
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
    incbin !baserom:$1E431..$1E8AB
    pea.w $8330                 ; $83E8AB
    plb                         ; $83E8AE
    lda.w #$000A                ; $83E8AF
    sta.w $8000                 ; $83E8B2
    lda.b $26                   ; $83E8B5
    asl a                       ; $83E8B7
    tax                         ; $83E8B8
    lda.b $24                   ; $83E8B9
    and.w #$FF00                ; $83E8BB
    lsr a                       ; $83E8BE
    sta.w $8000                 ; $83E8BF
    nop                         ; $83E8C2
    lda.l $7ED1C3,x             ; $83E8C3
    sta.w $8000                 ; $83E8C7
    nop                         ; $83E8CA
    nop                         ; $83E8CB
    nop                         ; $83E8CC
    nop                         ; $83E8CD
    nop                         ; $83E8CE
    nop                         ; $83E8CF
    sta.w $8000                 ; $83E8D0
    nop                         ; $83E8D3
    nop                         ; $83E8D4
    nop                         ; $83E8D5
    nop                         ; $83E8D6
    nop                         ; $83E8D7
    lda.w $8000                 ; $83E8D8
    sta.w $0114                 ; $83E8DB
    nop                         ; $83E8DE
    lda.w $8000                 ; $83E8DF
    sta.w $0116                 ; $83E8E2
    nop                         ; $83E8E5
    lda.w $8000                 ; $83E8E6
    sta.w $0118                 ; $83E8E9
    nop                         ; $83E8EC
    lda.w $8000                 ; $83E8ED
    sta.w $011A                 ; $83E8F0
    nop                         ; $83E8F3
    lda.w #$000A                ; $83E8F4
    sta.w $8000                 ; $83E8F7
    lda.b $24                   ; $83E8FA
    and.w #$FF00                ; $83E8FC
    lsr a                       ; $83E8FF
    sta.w $8000                 ; $83E900
    nop                         ; $83E903
    lda.l $7ED1B3,x             ; $83E904
    sta.w $8000                 ; $83E908
    nop                         ; $83E90B
    nop                         ; $83E90C
    nop                         ; $83E90D
    nop                         ; $83E90E
    nop                         ; $83E90F
    nop                         ; $83E910
    sta.w $8000                 ; $83E911
    nop                         ; $83E914
    nop                         ; $83E915
    nop                         ; $83E916
    nop                         ; $83E917
    nop                         ; $83E918
    lda.w $8000                 ; $83E919
    cmp.w $0114                 ; $83E91C
    bpl CODE_83E924             ; $83E91F
    incbin !baserom:$1E921..$1E924
CODE_83E924:
    lda.w $8000                 ; $83E924
    cmp.w $0116                 ; $83E927
    bmi CODE_83E92F             ; $83E92A
    sta.w $0116                 ; $83E92C
CODE_83E92F:
    lda.w $8000                 ; $83E92F
    cmp.w $0118                 ; $83E932
    bpl CODE_83E93A             ; $83E935
    incbin !baserom:$1E937..$1E93A
CODE_83E93A:
    lda.w $8000                 ; $83E93A
    cmp.w $011A                 ; $83E93D
    bmi CODE_83E945             ; $83E940
    sta.w $011A                 ; $83E942
CODE_83E945:
    lda.w $1ECE                 ; $83E945
    bne CODE_83E950             ; $83E948
    lda.w #$0001                ; $83E94A
    tsb.w $1E9C                 ; $83E94D
CODE_83E950:
    ldx.w $1EBA                 ; $83E950
    lda.l $7EFACD,x             ; $83E953
    tax                         ; $83E957
    lda.b $44                   ; $83E958
    bit.w #$0002                ; $83E95A
    beq CODE_83E992             ; $83E95D
    lda.l $7EFABB,x             ; $83E95F
    sec                         ; $83E963
    sbc.w #$0080                ; $83E964
    cmp.w $0114                 ; $83E967
    bpl CODE_83E985             ; $83E96A
    incbin !baserom:$1E96C..$1E980
CODE_83E980:
    lda.w #$0002                ; $83E980
    tsb.b $44                   ; $83E983
CODE_83E985:
    lda.l $7EFABB,x             ; $83E985
    sec                         ; $83E989
    sbc.w #$0020                ; $83E98A
    sta.w $0116                 ; $83E98D
    bra CODE_83E9C3             ; $83E990
CODE_83E992:
    lda.l $7EFABB,x             ; $83E992
    clc                         ; $83E996
    adc.w #$0080                ; $83E997
    cmp.w $0116                 ; $83E99A
    bmi CODE_83E9B8             ; $83E99D
    lda.l $7EFABB,x             ; $83E99F
    sec                         ; $83E9A3
    sbc.w #$0080                ; $83E9A4
    cmp.w $0114                 ; $83E9A7
    bmi CODE_83E9B8             ; $83E9AA
    cmp.w $0116                 ; $83E9AC
    bpl CODE_83E9C3             ; $83E9AF
    bra CODE_83E980             ; $83E9B1
    incbin !baserom:$1E9B3..$1E9B8
CODE_83E9B8:
    lda.l $7EFABB,x             ; $83E9B8
    clc                         ; $83E9BC
    adc.w #$0020                ; $83E9BD
    sta.w $0114                 ; $83E9C0
CODE_83E9C3:
    stz.w $011E                 ; $83E9C3
    bit.w $1F31                 ; $83E9C6
    bmi CODE_83EA1F             ; $83E9C9
    lda.b $1A                   ; $83E9CB
    and.w #$7FFF                ; $83E9CD
    dec a                       ; $83E9D0
    beq CODE_83E9F6             ; $83E9D1
    cmp.w #$0005                ; $83E9D3
    beq CODE_83E9F6             ; $83E9D6
    lda.b $53                   ; $83E9D8
    and.w #$00FF                ; $83E9DA
    beq CODE_83E9F6             ; $83E9DD
    incbin !baserom:$1E9DF..$1E9F6
CODE_83E9F6:
    stz.w $011E                 ; $83E9F6
    lda.w $1F2C                 ; $83E9F9
    lsr a                       ; $83E9FC
    bcc CODE_83EA1F             ; $83E9FD
    incbin !baserom:$1E9FF..$1EA1F
CODE_83EA1F:
    lda.w $0116                 ; $83EA1F
    sec                         ; $83EA22
    sbc.w $0114                 ; $83EA23
    cmp.w #$8000                ; $83EA26
    ror a                       ; $83EA29
    clc                         ; $83EA2A
    adc.w $0114                 ; $83EA2B
    clc                         ; $83EA2E
    adc.w $011E                 ; $83EA2F
    sta.w $011C                 ; $83EA32
    ldx.w $1EBC                 ; $83EA35
    lda.l $7E3B08,x             ; $83EA38
    beq CODE_83EA55             ; $83EA3C
    incbin !baserom:$1EA3E..$1EA55
CODE_83EA55:
    lda.l $7E3AEC,x             ; $83EA55
    beq CODE_83EA72             ; $83EA59
    incbin !baserom:$1EA5B..$1EA72
CODE_83EA72:
    lda.w $011C                 ; $83EA72
    sec                         ; $83EA75
    sbc.b $06                   ; $83EA76
    asl a                       ; $83EA78
    cmp.w #$FFD0                ; $83EA79
    bmi CODE_83EA86             ; $83EA7C
    cmp.w #$0030                ; $83EA7E
    bpl CODE_83EA86             ; $83EA81
    incbin !baserom:$1EA83..$1EA86
CODE_83EA86:
    tax                         ; $83EA86
    beq CODE_83EA99             ; $83EA87
    bmi CODE_83EA93             ; $83EA89
    lda.w #$0008                ; $83EA8B
    tsb.w $1E9C                 ; $83EA8E
    bra CODE_83EA99             ; $83EA91
CODE_83EA93:
    lda.w #$0004                ; $83EA93
    tsb.w $1E9C                 ; $83EA96
CODE_83EA99:
    plb                         ; $83EA99
    rts                         ; $83EA9A
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
    stz.w $8000                 ; $83EADC
    lda.w #$0060                ; $83EADF
    sta.w $8000                 ; $83EAE2
    lda.w $1E9E                 ; $83EAE5
    xba                         ; $83EAE8
    sta.w $8000                 ; $83EAE9
    lda.b $12                   ; $83EAEC
    clc                         ; $83EAEE
    adc.w $8000                 ; $83EAEF
    sta.b $12                   ; $83EAF2
    lda.b $14                   ; $83EAF4
    adc.w $8000                 ; $83EAF6
    sta.b $14                   ; $83EAF9
    cmp.w #$0009                ; $83EAFB
    bcc CODE_83EB0A             ; $83EAFE
    lda.w #$0009                ; $83EB00
    sta.b $14                   ; $83EB03
    lda.w #$FFFF                ; $83EB05
    sta.b $12                   ; $83EB08
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
    incbin !baserom:$1EB2B..$1EB2F
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
    tax                         ; $83EBF6
    clc                         ; $83EBF7
    adc.w $1E9E                 ; $83EBF8
    sta.b $7E                   ; $83EBFB
    cpx.w #$0054                ; $83EBFD
    bcs CODE_83EC07             ; $83EC00
    cmp.w #$0054                ; $83EC02
    bcs CODE_83EC11             ; $83EC05
CODE_83EC07:
    cpx.w #$0058                ; $83EC07
    bcs CODE_83EC1D             ; $83EC0A
    cmp.w #$0058                ; $83EC0C
    bcc CODE_83EC1D             ; $83EC0F
CODE_83EC11:
    ldx.w $1EBE                 ; $83EC11
    lda.w #$7F21                ; $83EC14
    ldy.w #$0000                ; $83EC17
    jsr.w $FE9A                 ; $83EC1A
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
    inc a                       ; $83ED1A
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
    lda.w #$0000                ; $83ED92
    ldx.b $48                   ; $83ED95
    beq CODE_83ED9D             ; $83ED97
    incbin !baserom:$1ED99..$1ED9D
CODE_83ED9D:
    ldx.b $31                   ; $83ED9D
    beq CODE_83EDA5             ; $83ED9F
    incbin !baserom:$1EDA1..$1EDA5
CODE_83EDA5:
    stz.w $8000                 ; $83EDA5
    clc                         ; $83EDA8
    adc.w $1EA8                 ; $83EDA9
    tax                         ; $83EDAC
    lda.b $44                   ; $83EDAD
    bit.w #$0010                ; $83EDAF
    bne CODE_83EDB7             ; $83EDB2
    txa                         ; $83EDB4
    lsr a                       ; $83EDB5
    tax                         ; $83EDB6
CODE_83EDB7:
    stx.w $8000                 ; $83EDB7
    lda.w $1EA6                 ; $83EDBA
    sta.w $8000                 ; $83EDBD
    nop                         ; $83EDC0
    lda.w $8000                 ; $83EDC1
    xba                         ; $83EDC4
    and.w #$00FF                ; $83EDC5
    sta.w $0114                 ; $83EDC8
    lda.w $8000                 ; $83EDCB
    bit.w #$FF80                ; $83EDCE
    beq CODE_83EDD6             ; $83EDD1
    incbin !baserom:$1EDD3..$1EDD6
CODE_83EDD6:
    xba                         ; $83EDD6
    and.w #$FF00                ; $83EDD7
    ora.w $0114                 ; $83EDDA
    stz.w $8000                 ; $83EDDD
    nop                         ; $83EDE0
    sta.w $8000                 ; $83EDE1
    lda.w #$0001                ; $83EDE4
    sta.w $8000                 ; $83EDE7
    lda.b $16                   ; $83EDEA
    clc                         ; $83EDEC
    adc.w $8000                 ; $83EDED
    sta.b $16                   ; $83EDF0
    lda.b $18                   ; $83EDF2
    adc.w $8000                 ; $83EDF4
    sta.b $18                   ; $83EDF7
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
    inx                         ; $83EE12
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
    dey                         ; $83EE35
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
    ldx.w #$FFF0                ; $83EEB2
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
    incbin !baserom:$1EED5..$1EEDC
CODE_83EEDC:
    cmp.b $18                   ; $83EEDC
    beq CODE_83EEE2             ; $83EEDE
    bcs CODE_83EEEF             ; $83EEE0
CODE_83EEE2:
    lda.w #$0010                ; $83EEE2
    tsb.b $44                   ; $83EEE5
CODE_83EEE7:
    lda.b $16                   ; $83EEE7
    sta.b $12                   ; $83EEE9
    lda.b $18                   ; $83EEEB
    sta.b $14                   ; $83EEED
CODE_83EEEF:
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
    inc.b $1F                   ; $83EF12
    bra CODE_83EF1F             ; $83EF14
CODE_83EF16:
    lda.b $17                   ; $83EF16
    cmp.w $1EAC                 ; $83EF18
    bcs CODE_83EF6E             ; $83EF1B
    dec.b $1F                   ; $83EF1D
CODE_83EF1F:
    lda.w $1EB2                 ; $83EF1F
    and.w #$007F                ; $83EF22
    asl a                       ; $83EF25
    tax                         ; $83EF26
    lda.b $1F                   ; $83EF27
    asl a                       ; $83EF29
    clc                         ; $83EF2A
    adc.l $86DAD9,x             ; $83EF2B
    tax                         ; $83EF2F
    jsr.w $EA9B                 ; $83EF30
    sta.b $2B                   ; $83EF33
    bit.w $1F31                 ; $83EF35
    bmi CODE_83EF3F             ; $83EF38
    lda.w $1EA2                 ; $83EF3A
    sta.b $95                   ; $83EF3D
CODE_83EF3F:
    lda.b $0E                   ; $83EF3F
    asl a                       ; $83EF41
    sta.w $0114                 ; $83EF42
    lda.b $10                   ; $83EF45
    rol a                       ; $83EF47
    sta.w $0116                 ; $83EF48
    lda.w #$0000                ; $83EF4B
    rol a                       ; $83EF4E
    ldx.w #$0028                ; $83EF4F
CODE_83EF52:
    cmp.w $1EA0                 ; $83EF52
    bcc CODE_83EF5A             ; $83EF55
    sbc.w $1EA0                 ; $83EF57
CODE_83EF5A:
    rol.w $0114                 ; $83EF5A
    rol.w $0116                 ; $83EF5D
    rol a                       ; $83EF60
    dex                         ; $83EF61
    bne CODE_83EF52             ; $83EF62
    lda.w $0114                 ; $83EF64
    sta.b $16                   ; $83EF67
    lda.w $0116                 ; $83EF69
    sta.b $18                   ; $83EF6C
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
    stz.w $8000                 ; $83EFBD
    lda.b $37                   ; $83EFC0
    sta.w $8000                 ; $83EFC2
    lda.w $1E9E                 ; $83EFC5
    sta.w $8000                 ; $83EFC8
    lda.w $013C                 ; $83EFCB
    clc                         ; $83EFCE
    adc.w $8000                 ; $83EFCF
    sta.w $013C                 ; $83EFD2
    lda.w $8000                 ; $83EFD5
CODE_83EFD8:
    bit.w $1F31                 ; $83EFD8
    bmi CODE_83F004             ; $83EFDB
    lda.b $58                   ; $83EFDD
    bne CODE_83F004             ; $83EFDF
    lda.b $10                   ; $83EFE1
    cmp.w #$0010                ; $83EFE3
    bcc CODE_83F004             ; $83EFE6
    stz.w $8000                 ; $83EFE8
    lda.w $1E9E                 ; $83EFEB
    sta.w $8000                 ; $83EFEE
    lda.w #$0080                ; $83EFF1
    sta.w $8000                 ; $83EFF4
    lda.w $013C                 ; $83EFF7
    clc                         ; $83EFFA
    adc.w $8000                 ; $83EFFB
    sta.w $013C                 ; $83EFFE
    lda.w $8000                 ; $83F001
CODE_83F004:
    bit.w $1F31                 ; $83F004
    bmi CODE_83F05B             ; $83F007
    ldx.w $1EBC                 ; $83F009
    lda.l $7E3D6E,x             ; $83F00C
    bit.w #$0008                ; $83F010
    beq CODE_83F01A             ; $83F013
    lda.w #$0000                ; $83F015
    bra CODE_83F04B             ; $83F018
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
CODE_83F04B:
    sta.b $9E                   ; $83F04B
    stz.b $9C                   ; $83F04D
    lda.w $1F2C                 ; $83F04F
    bit.w #$8001                ; $83F052
    beq CODE_83F05B             ; $83F055
    lda.b $9E                   ; $83F057
    sta.b $9C                   ; $83F059
CODE_83F05B:
    ldx.w $1EBC                 ; $83F05B
    ldy.w #$0000                ; $83F05E
    lda.l $7E3BEC,x             ; $83F061
    beq CODE_83F06A             ; $83F065
    ldy.w #$0002                ; $83F067
CODE_83F06A:
    lda.b $06                   ; $83F06A
    bpl CODE_83F072             ; $83F06C
    eor.w #$FFFF                ; $83F06E
    inc a                       ; $83F071
CODE_83F072:
    sec                         ; $83F072
    sbc.w $1F28,y               ; $83F073
    bcc CODE_83F0C5             ; $83F076
    bit.w $1F31                 ; $83F078
    bmi CODE_83F0A2             ; $83F07B
    ldx.w $1EBC                 ; $83F07D
    ldy.w #$0000                ; $83F080
    lda.l $7E3BEC,x             ; $83F083
    beq CODE_83F08C             ; $83F087
    ldy.w #$0002                ; $83F089
CODE_83F08C:
    lda.b $06                   ; $83F08C
    sec                         ; $83F08E
    sbc.b $9E                   ; $83F08F
    bpl CODE_83F097             ; $83F091
    eor.w #$FFFF                ; $83F093
    inc a                       ; $83F096
CODE_83F097:
    sec                         ; $83F097
    sbc.w $1F28,y               ; $83F098
    bcc CODE_83F0C5             ; $83F09B
    lda.w #$0010                ; $83F09D
    trb.b $7C                   ; $83F0A0
CODE_83F0A2:
    lda.b $10                   ; $83F0A2
    cmp.w #$0010                ; $83F0A4
    bcc CODE_83F0C5             ; $83F0A7
    stz.w $8000                 ; $83F0A9
    lda.w $1E9E                 ; $83F0AC
    sta.w $8000                 ; $83F0AF
    lda.w #$0080                ; $83F0B2
    sta.w $8000                 ; $83F0B5
    lda.w $013C                 ; $83F0B8
    clc                         ; $83F0BB
    adc.w $8000                 ; $83F0BC
    sta.w $013C                 ; $83F0BF
    lda.w $8000                 ; $83F0C2
CODE_83F0C5:
    bit.w $1F31                 ; $83F0C5
    bmi CODE_83F0F1             ; $83F0C8
    lda.b $10                   ; $83F0CA
    cmp.w #$0010                ; $83F0CC
    bcc CODE_83F0F1             ; $83F0CF
    stz.w $8000                 ; $83F0D1
    lda.w $1E9E                 ; $83F0D4
    sta.w $8000                 ; $83F0D7
    lda.b $5B                   ; $83F0DA
    and.w #$00FF                ; $83F0DC
    lsr a                       ; $83F0DF
    lsr a                       ; $83F0E0
    sta.w $8000                 ; $83F0E1
    lda.w $013C                 ; $83F0E4
    clc                         ; $83F0E7
    adc.w $8000                 ; $83F0E8
    sta.w $013C                 ; $83F0EB
    lda.w $8000                 ; $83F0EE
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
    sta.w $0114                 ; $83F142
    bmi CODE_83F14A             ; $83F145
    ldx.w #$FFF4                ; $83F147
CODE_83F14A:
    stz.w $8000                 ; $83F14A
    lda.w $1E9E                 ; $83F14D
    sta.w $8000                 ; $83F150
    stx.w $8000                 ; $83F153
    nop                         ; $83F156
    lda.w $8000                 ; $83F157
    bit.w $8000                 ; $83F15A
    stz.w $8000                 ; $83F15D
    ldx.b $10                   ; $83F160
    cpx.w #$0080                ; $83F162
    bcc CODE_83F16C             ; $83F165
    ldx.w #$7FFF                ; $83F167
    bra CODE_83F16E             ; $83F16A
CODE_83F16C:
    ldx.b $0F                   ; $83F16C
CODE_83F16E:
    stx.w $8000                 ; $83F16E
    sta.w $8000                 ; $83F171
    lda.b $21                   ; $83F174
    clc                         ; $83F176
    bit.w $8000                 ; $83F177
    adc.w $8000                 ; $83F17A
    sta.b $21                   ; $83F17D
    eor.w $0114                 ; $83F17F
    bpl CODE_83F186             ; $83F182
    stz.b $21                   ; $83F184
CODE_83F186:
    stz.w $013C                 ; $83F186
    lda.w $1E9C                 ; $83F189
    bit.w #$0008                ; $83F18C
    beq CODE_83F196             ; $83F18F
    lda.w #$0018                ; $83F191
    bra CODE_83F1A1             ; $83F194
CODE_83F196:
    lda.w $1E9C                 ; $83F196
    bit.w #$0004                ; $83F199
    beq CODE_83F1EC             ; $83F19C
    lda.w #$FFE8                ; $83F19E
CODE_83F1A1:
    stz.w $8000                 ; $83F1A1
    ldx.w $1E9E                 ; $83F1A4
    stx.w $8000                 ; $83F1A7
    sta.w $8000                 ; $83F1AA
    nop                         ; $83F1AD
    lda.w $8000                 ; $83F1AE
    bit.w $8000                 ; $83F1B1
    stz.w $8000                 ; $83F1B4
    ldx.b $10                   ; $83F1B7
    cpx.w #$0080                ; $83F1B9
    bcc CODE_83F1C3             ; $83F1BC
    ldx.w #$7FFF                ; $83F1BE
    bra CODE_83F1C5             ; $83F1C1
CODE_83F1C3:
    ldx.b $0F                   ; $83F1C3
CODE_83F1C5:
    stx.w $8000                 ; $83F1C5
    sta.w $8000                 ; $83F1C8
    lda.b $21                   ; $83F1CB
    clc                         ; $83F1CD
    bit.w $8000                 ; $83F1CE
    adc.w $8000                 ; $83F1D1
    cmp.w #$0080                ; $83F1D4
    bmi CODE_83F1DF             ; $83F1D7
    lda.w #$0080                ; $83F1D9
    sta.w $013C                 ; $83F1DC
CODE_83F1DF:
    cmp.w #$FF80                ; $83F1DF
    bpl CODE_83F1EA             ; $83F1E2
    lda.w #$FF80                ; $83F1E4
    sta.w $013C                 ; $83F1E7
CODE_83F1EA:
    sta.b $21                   ; $83F1EA
CODE_83F1EC:
    lda.b $1C                   ; $83F1EC
    beq CODE_83F23F             ; $83F1EE
    incbin !baserom:$1F1F0..$1F23F
CODE_83F23F:
    lda.b $44                   ; $83F23F
    bit.w #$0020                ; $83F241
    bne CODE_83F24D             ; $83F244
    lda.b $44                   ; $83F246
    bit.w #$0001                ; $83F248
    beq CODE_83F250             ; $83F24B
CODE_83F24D:
    jmp.w $F35C                 ; $83F24D
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
    dex                         ; $83F26F
CODE_83F270:
    stx.w $0118                 ; $83F270
    stz.w $8000                 ; $83F273
    lda.b $10                   ; $83F276
    bit.b $0E                   ; $83F278
    bpl CODE_83F27D             ; $83F27A
    inc a                       ; $83F27C
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
    clc                         ; $83F29D
    adc.w $0117                 ; $83F29E
    bmi CODE_83F2B1             ; $83F2A1
    lda.w #$0000                ; $83F2A3
    bra CODE_83F2B1             ; $83F2A6
CODE_83F2A8:
    sec                         ; $83F2A8
    sbc.w $0117                 ; $83F2A9
    bpl CODE_83F2B1             ; $83F2AC
    lda.w #$0000                ; $83F2AE
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
    ora.w #$FF00                ; $83F30A
CODE_83F30D:
    sta.w $011A                 ; $83F30D
    adc.b $06                   ; $83F310
    sta.b $06                   ; $83F312
CODE_83F314:
    lda.b $44                   ; $83F314
    and.w #$0010                ; $83F316
    beq CODE_83F32F             ; $83F319
    lda.b $29                   ; $83F31B
    beq CODE_83F35A             ; $83F31D
    lda.w $013C                 ; $83F31F
    eor.b $29                   ; $83F322
    bpl CODE_83F35A             ; $83F324
    lda.w $0115                 ; $83F326
    clc                         ; $83F329
    adc.w $0119                 ; $83F32A
    bmi CODE_83F35A             ; $83F32D
CODE_83F32F:
    lda.b $1A                   ; $83F32F
    and.w #$7FFF                ; $83F331
    dec a                       ; $83F334
    beq CODE_83F35A             ; $83F335
    cmp.w #$0005                ; $83F337
    beq CODE_83F35A             ; $83F33A
    lda.b $46                   ; $83F33C
    sec                         ; $83F33E
    sbc.w $1E9E                 ; $83F33F
    sta.b $46                   ; $83F342
    bpl CODE_83F35C             ; $83F344
    clc                         ; $83F346
    adc.w #$0004                ; $83F347
    sta.b $46                   ; $83F34A
    ldx.w $1EBE                 ; $83F34C
    lda.w #$4F21                ; $83F34F
    ldy.w #$0000                ; $83F352
    jsr.w $FE9A                 ; $83F355
    bra CODE_83F35C             ; $83F358
CODE_83F35A:
    stz.b $46                   ; $83F35A
CODE_83F35C:
    bit.w $1F31                 ; $83F35C
    bpl CODE_83F364             ; $83F35F
    incbin !baserom:$1F361..$1F364
CODE_83F364:
    lda.b $06                   ; $83F364
    bpl CODE_83F36C             ; $83F366
    eor.w #$FFFF                ; $83F368
    inc a                       ; $83F36B
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
    bmi CODE_83F449             ; $83F433
    ora.w #$8000                ; $83F435
    sta.b $1A                   ; $83F438
    asl a                       ; $83F43A
    tax                         ; $83F43B
    lda.w #$0040                ; $83F43C
    trb.b $44                   ; $83F43F
    lda.l $86DCF7,x             ; $83F441
    sta.b $3A                   ; $83F445
    bra CODE_83F4BB             ; $83F447
CODE_83F449:
    asl a                       ; $83F449
    tax                         ; $83F44A
    lda.b $44                   ; $83F44B
    bit.w #$0040                ; $83F44D
    bne CODE_83F4BB             ; $83F450
    stz.w $8000                 ; $83F452
    lda.w $1E9E                 ; $83F455
    sta.w $8000                 ; $83F458
    lda.l $86DD1B,x             ; $83F45B
    sta.w $8000                 ; $83F45F
    nop                         ; $83F462
    lda.w $8000                 ; $83F463
    bit.w $8000                 ; $83F466
    clc                         ; $83F469
    adc.b $3A                   ; $83F46A
    sta.b $3A                   ; $83F46C
    cmp.l $86DD03,x             ; $83F46E
    bcc CODE_83F48A             ; $83F472
    lda.l $86DD0F,x             ; $83F474
    lsr a                       ; $83F478
    bcc CODE_83F486             ; $83F479
    incbin !baserom:$1F47B..$1F486
CODE_83F486:
    stz.b $1A                   ; $83F486
    bra CODE_83F48E             ; $83F488
CODE_83F48A:
    sta.b $3A                   ; $83F48A
    bra CODE_83F4BB             ; $83F48C
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
    dex                         ; $83F4AA
    cmp.w #$0040                ; $83F4AB
    bmi CODE_83F4B7             ; $83F4AE
    dex                         ; $83F4B0
    cmp.w #$0060                ; $83F4B1
    bmi CODE_83F4B7             ; $83F4B4
    dex                         ; $83F4B6
CODE_83F4B7:
    txa                         ; $83F4B7
    xba                         ; $83F4B8
    sta.b $3A                   ; $83F4B9
CODE_83F4BB:
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
    bne CODE_83F512             ; $83F50D
CODE_83F50F:
    jmp.w $F645                 ; $83F50F
CODE_83F512:
    lda.b $B1                   ; $83F512
    and.w #$00FF                ; $83F514
    asl a                       ; $83F517
    asl a                       ; $83F518
    asl a                       ; $83F519
    asl a                       ; $83F51A
    bit.w #$0800                ; $83F51B
    beq CODE_83F523             ; $83F51E
    ora.w #$F000                ; $83F520
CODE_83F523:
    sta.w $0114                 ; $83F523
    lda.b $B2                   ; $83F526
    and.w #$00FF                ; $83F528
    clc                         ; $83F52B
    adc.w #$0020                ; $83F52C
    sta.w $0116                 ; $83F52F
    lda.b $06                   ; $83F532
    sec                         ; $83F534
    sbc.w $0114                 ; $83F535
    bpl CODE_83F53E             ; $83F538
    eor.w #$FFFF                ; $83F53A
    inc a                       ; $83F53D
CODE_83F53E:
    cmp.w $0116                 ; $83F53E
    bpl CODE_83F50F             ; $83F541
    lda.b $53                   ; $83F543
    and.w #$00FF                ; $83F545
    asl a                       ; $83F548
    tax                         ; $83F549
    jmp.w ($F54B,x)             ; $83F54A
    incbin !baserom:$1F54D..$1F555
    lda.w #$003A                ; $83F555
    jsr.w $FEBD                 ; $83F558
    lda.w #$0000                ; $83F55B
    sta.l $308000               ; $83F55E
    lda.w #$2000                ; $83F562
    sta.l $308000               ; $83F565
    lda.w $1E9E                 ; $83F569
    sta.l $308000               ; $83F56C
    lda.b $56                   ; $83F570
    clc                         ; $83F572
    adc.l $308000               ; $83F573
    sta.b $56                   ; $83F577
    lda.b $58                   ; $83F579
    adc.l $308000               ; $83F57B
    sta.b $58                   ; $83F57F
    cmp.w #$0028                ; $83F581
    bcc CODE_83F590             ; $83F584
    lda.w #$0028                ; $83F586
    sta.b $58                   ; $83F589
    lda.w #$FFFF                ; $83F58B
    sta.b $56                   ; $83F58E
CODE_83F590:
    lda.b $4C                   ; $83F590
    sec                         ; $83F592
    sbc.w $1E9E                 ; $83F593
    sta.b $4C                   ; $83F596
    bpl CODE_83F5B5             ; $83F598
    stz.b $4C                   ; $83F59A
    lda.w $0463                 ; $83F59C
    bit.w #$00FF                ; $83F59F
    bne CODE_83F5B5             ; $83F5A2
    lda.w #$0010                ; $83F5A4
    sta.b $4C                   ; $83F5A7
    ldx.w $1EBE                 ; $83F5A9
    lda.w #$FF20                ; $83F5AC
    ldy.w #$0000                ; $83F5AF
    jsr.w $FE9A                 ; $83F5B2
CODE_83F5B5:
    jmp.w $F647                 ; $83F5B5
    lda.w #$003B                ; $83F5B8
    jsr.w $FEBD                 ; $83F5BB
    bit.w $1F31                 ; $83F5BE
    bmi CODE_83F5EA             ; $83F5C1
    lda.w #$0000                ; $83F5C3
    sta.l $308000               ; $83F5C6
    lda.w #$0200                ; $83F5CA
    sta.l $308000               ; $83F5CD
    lda.w $1E9E                 ; $83F5D1
    sta.l $308000               ; $83F5D4
    lda.b $5A                   ; $83F5D8
    sec                         ; $83F5DA
    sbc.l $308000               ; $83F5DB
    bcs CODE_83F5E4             ; $83F5DF
    lda.w #$0000                ; $83F5E1
CODE_83F5E4:
    sta.b $5A                   ; $83F5E4
    lda.l $308000               ; $83F5E6
CODE_83F5EA:
    lda.b $4C                   ; $83F5EA
    sec                         ; $83F5EC
    sbc.w $1E9E                 ; $83F5ED
    sta.b $4C                   ; $83F5F0
    bpl CODE_83F647             ; $83F5F2
    stz.b $4C                   ; $83F5F4
    lda.w $0463                 ; $83F5F6
    bit.w #$00FF                ; $83F5F9
    bne CODE_83F647             ; $83F5FC
    lda.w #$0010                ; $83F5FE
    sta.b $4C                   ; $83F601
    ldx.w $1EBE                 ; $83F603
    lda.w #$FF14                ; $83F606
    ldy.w #$0000                ; $83F609
    jsr.w $FE9A                 ; $83F60C
    bra CODE_83F647             ; $83F60F
    incbin !baserom:$1F611..$1F645
    stz.b $4C                   ; $83F645
CODE_83F647:
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
    lda.b $44                   ; $83F67A
    bit.w #$0004                ; $83F67C
    bne CODE_83F68D             ; $83F67F
    ldx.w $1EBE                 ; $83F681
    lda.w #$FF26                ; $83F684
    ldy.w #$0000                ; $83F687
    jsr.w $FE9A                 ; $83F68A
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
    lda.w #$0080                ; $83F6CF
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
    bne CODE_83F93C             ; $83F937
    jmp.w $F9C2                 ; $83F939
CODE_83F93C:
    lda.w $0114                 ; $83F93C
    asl a                       ; $83F93F
    asl a                       ; $83F940
    sta.w $0114                 ; $83F941
    lda.w $019A                 ; $83F944
    asl a                       ; $83F947
    asl a                       ; $83F948
    sta.w $0118                 ; $83F949
    ldx.w $1EBA                 ; $83F94C
    lda.l $7EFABF,x             ; $83F94F
    sta.l $81012C               ; $83F953
    lda.w $0114                 ; $83F957
    lsr a                       ; $83F95A
    and.w #$003E                ; $83F95B
    tax                         ; $83F95E
    lda.b $06                   ; $83F95F
    sec                         ; $83F961
    sbc.b $9E                   ; $83F962
    lda.w #$0000                ; $83F964
    sec                         ; $83F967
    sbc.l $7E3DEC,x             ; $83F968
    sta.w $011E                 ; $83F96C
CODE_83F96F:
    lda.w $0114                 ; $83F96F
    lsr a                       ; $83F972
    and.w #$003E                ; $83F973
    tax                         ; $83F976
    lda.l $7E3DEE,x             ; $83F977
    sec                         ; $83F97B
    sbc.l $7E3DEC,x             ; $83F97C
    eor.w #$FFFF                ; $83F980
    inc a                       ; $83F983
    sec                         ; $83F984
    sbc.b $0C                   ; $83F985
    clc                         ; $83F987
    adc.w $011E                 ; $83F988
    sta.w $011E                 ; $83F98B
    lda.w $0114                 ; $83F98E
    cmp.w $0118                 ; $83F991
    bcc CODE_83F99C             ; $83F994
    sbc.w $0118                 ; $83F996
    sta.w $0114                 ; $83F999
CODE_83F99C:
    tax                         ; $83F99C
    sta.w $011A                 ; $83F99D
    lda.l $7E8CB3,x             ; $83F9A0
    beq CODE_83F9B3             ; $83F9A4
    xba                         ; $83F9A6
    and.w #$00FF                ; $83F9A7
    asl a                       ; $83F9AA
    tax                         ; $83F9AB
    asl a                       ; $83F9AC
    sta.w $011C                 ; $83F9AD
    jsr.w ($7FFE,x)             ; $83F9B0
CODE_83F9B3:
    lda.w $0114                 ; $83F9B3
    clc                         ; $83F9B6
    adc.w #$0004                ; $83F9B7
    sta.w $0114                 ; $83F9BA
    dec.w $0116                 ; $83F9BD
    bne CODE_83F96F             ; $83F9C0
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
    inc.b $35                   ; $83F9D8
    lda.b $35                   ; $83F9DA
    beq CODE_83FA31             ; $83F9DC
    lda.w $1F06                 ; $83F9DE
    sec                         ; $83F9E1
    sbc.b $35                   ; $83F9E2
    bne CODE_83FA2B             ; $83F9E4
    lda.b $31                   ; $83F9E6
    beq CODE_83F9F1             ; $83F9E8
    incbin !baserom:$1F9EA..$1F9F1
CODE_83F9F1:
    ldx.w $1EBA                 ; $83F9F1
    lda.l $7EFADE,x             ; $83F9F4
    ora.w #$0004                ; $83F9F8
    sta.l $7EFADE,x             ; $83F9FB
    lda.w #$0001                ; $83F9FF
    tsb.b $44                   ; $83FA02
    stz.b $25                   ; $83FA04
    lda.w $019A                 ; $83FA06
    sta.b $26                   ; $83FA09
    ldx.w $1F08                 ; $83FA0B
    lda.w $1ECA                 ; $83FA0E
    sta.w $0446,x               ; $83FA11
    txa                         ; $83FA14
    clc                         ; $83FA15
    adc.w #$0012                ; $83FA16
    sta.b $33                   ; $83FA19
    inx                         ; $83FA1B
    stx.w $1F08                 ; $83FA1C
    lda.w #$0004                ; $83FA1F
    tsb.b $44                   ; $83FA22
    lda.w #$0008                ; $83FA24
    trb.b $44                   ; $83FA27
    bra CODE_83FA31             ; $83FA29
CODE_83FA2B:
    clc                         ; $83FA2B
    adc.w #$0009                ; $83FA2C
    sta.b $33                   ; $83FA2F
CODE_83FA31:
    rts                         ; $83FA31
    clc                         ; $83FA32
    rts                         ; $83FA33
    stz.w $012A                 ; $83FA34
    ldx.w $011A                 ; $83FA37
    lda.l $7E8CB3,x             ; $83FA3A
    and.w #$00FF                ; $83FA3E
    asl a                       ; $83FA41
    asl a                       ; $83FA42
    asl a                       ; $83FA43
    asl a                       ; $83FA44
    bit.w #$0800                ; $83FA45
    beq CODE_83FA4D             ; $83FA48
    ora.w #$F000                ; $83FA4A
CODE_83FA4D:
    sta.w $0128                 ; $83FA4D
    lda.l $7E8CB5,x             ; $83FA50
    and.w #$0030                ; $83FA54
    beq CODE_83FA5C             ; $83FA57
    incbin !baserom:$1FA59..$1FA5C
CODE_83FA5C:
    lda.w $0128                 ; $83FA5C
    jsr.w $FD8A                 ; $83FA5F
    rts                         ; $83FA62
    incbin !baserom:$1FA63..$1FA6A
    stz.w $012A                 ; $83FA6A
    ldx.w $011A                 ; $83FA6D
    lda.l $7E8CB3,x             ; $83FA70
    and.w #$00FF                ; $83FA74
    asl a                       ; $83FA77
    asl a                       ; $83FA78
    asl a                       ; $83FA79
    asl a                       ; $83FA7A
    bit.w #$0800                ; $83FA7B
    beq CODE_83FA83             ; $83FA7E
    ora.w #$F000                ; $83FA80
CODE_83FA83:
    sta.w $0128                 ; $83FA83
    sec                         ; $83FA86
    sbc.b $06                   ; $83FA87
    bpl CODE_83FAB0             ; $83FA89
    ldx.w $011C                 ; $83FA8B
    cmp.l $8380CC,x             ; $83FA8E
    bmi CODE_83FAD5             ; $83FA92
    incbin !baserom:$1FA94..$1FAB0
CODE_83FAB0:
    ldx.w $011C                 ; $83FAB0
    cmp.l $8380CE,x             ; $83FAB3
    bpl CODE_83FAD5             ; $83FAB7
    lda.b $1A                   ; $83FAB9
    bne CODE_83FAD5             ; $83FABB
    lda.w #$0008                ; $83FABD
    sta.b $7E                   ; $83FAC0
    lda.w #$0006                ; $83FAC2
    sta.b $1A                   ; $83FAC5
    lda.w #$FF1D                ; $83FAC7
    ldx.w $1EBE                 ; $83FACA
    ldy.w #$0000                ; $83FACD
    jsr.w $FE9A                 ; $83FAD0
    bra CODE_83FAD5             ; $83FAD3
CODE_83FAD5:
    clc                         ; $83FAD5
    rts                         ; $83FAD6
    jsr.w $FC05                 ; $83FAD7
    bcc CODE_83FB14             ; $83FADA
    ldx.w $011A                 ; $83FADC
    lda.w #$0000                ; $83FADF
    sta.l $7E8CB3,x             ; $83FAE2
    sed                         ; $83FAE6
    ldx.w $1ECC                 ; $83FAE7
    lda.w $038E,x               ; $83FAEA
    clc                         ; $83FAED
    adc.w #$4000                ; $83FAEE
    sta.w $038E,x               ; $83FAF1
    lda.w $0390,x               ; $83FAF4
    adc.w #$0000                ; $83FAF7
    sta.w $0390,x               ; $83FAFA
    cld                         ; $83FAFD
    ldx.w $1EBE                 ; $83FAFE
    lda.w #$FF19                ; $83FB01
    ldy.w #$0000                ; $83FB04
    jsr.w $FE9A                 ; $83FB07
    lda.w #$801E                ; $83FB0A
    sta.b $4A                   ; $83FB0D
    lda.w #$0047                ; $83FB0F
    sta.b $33                   ; $83FB12
CODE_83FB14:
    rts                         ; $83FB14
    jsr.w $FC05                 ; $83FB15
    bcc CODE_83FB49             ; $83FB18
    incbin !baserom:$1FB1A..$1FB49
CODE_83FB49:
    rts                         ; $83FB49
    incbin !baserom:$1FB4A..$1FC05
    ldx.w $011A                 ; $83FC05
    lda.l $7E8CB3,x             ; $83FC08
    and.w #$00FF                ; $83FC0C
    asl a                       ; $83FC0F
    asl a                       ; $83FC10
    asl a                       ; $83FC11
    asl a                       ; $83FC12
    bit.w #$0800                ; $83FC13
    beq CODE_83FC1B             ; $83FC16
    ora.w #$F000                ; $83FC18
CODE_83FC1B:
    sta.w $0128                 ; $83FC1B
    sec                         ; $83FC1E
    sbc.b $06                   ; $83FC1F
    bpl CODE_83FC2E             ; $83FC21
    ldx.w $011C                 ; $83FC23
    cmp.l $8380CC,x             ; $83FC26
    bpl CODE_83FC39             ; $83FC2A
    clc                         ; $83FC2C
    rts                         ; $83FC2D
CODE_83FC2E:
    ldx.w $011C                 ; $83FC2E
    cmp.l $8380CE,x             ; $83FC31
    bmi CODE_83FC39             ; $83FC35
CODE_83FC37:
    clc                         ; $83FC37
    rts                         ; $83FC38
CODE_83FC39:
    ldx.w $011A                 ; $83FC39
    lda.l $7E8CB5,x             ; $83FC3C
    and.w #$00F0                ; $83FC40
    lsr a                       ; $83FC43
    eor.w #$FFFF                ; $83FC44
    inc a                       ; $83FC47
    ldx.w $011C                 ; $83FC48
    sec                         ; $83FC4B
    sbc.l $81012C               ; $83FC4C
    bpl CODE_83FC5A             ; $83FC50
    incbin !baserom:$1FC52..$1FC5A
CODE_83FC5A:
    ldx.w $011C                 ; $83FC5A
    cmp.l $83826E,x             ; $83FC5D
    bpl CODE_83FC37             ; $83FC61
    sec                         ; $83FC63
    rts                         ; $83FC64
    stz.w $012A                 ; $83FC65
    ldx.w $011A                 ; $83FC68
    lda.l $7E8CB3,x             ; $83FC6B
    and.w #$00FF                ; $83FC6F
    asl a                       ; $83FC72
    asl a                       ; $83FC73
    asl a                       ; $83FC74
    asl a                       ; $83FC75
    bit.w #$0800                ; $83FC76
    beq CODE_83FC7E             ; $83FC79
    ora.w #$F000                ; $83FC7B
CODE_83FC7E:
    sta.w $0128                 ; $83FC7E
    lda.l $7E8CB5,x             ; $83FC81
    and.w #$0030                ; $83FC85
    lsr a                       ; $83FC88
    lsr a                       ; $83FC89
    lsr a                       ; $83FC8A
    txy                         ; $83FC8B
    tax                         ; $83FC8C
    jmp.w ($FC90,x)             ; $83FC8D
    incbin !baserom:$1FC90..$1FC98
    lda.w $0128                 ; $83FC98
    jsr.w $FD8A                 ; $83FC9B
    bcs CODE_83FCCB             ; $83FC9E
    lda.w $0128                 ; $83FCA0
    eor.w #$FFFF                ; $83FCA3
    inc a                       ; $83FCA6
    sta.w $0128                 ; $83FCA7
    jsr.w $FD8A                 ; $83FCAA
    bcs CODE_83FCCB             ; $83FCAD
    lda.w $0128                 ; $83FCAF
    sec                         ; $83FCB2
    sbc.w #$0080                ; $83FCB3
    sta.w $0128                 ; $83FCB6
    jsr.w $FD8A                 ; $83FCB9
    bcs CODE_83FCCB             ; $83FCBC
    lda.w $0128                 ; $83FCBE
    eor.w #$FFFF                ; $83FCC1
    inc a                       ; $83FCC4
    sta.w $0128                 ; $83FCC5
    jsr.w $FD8A                 ; $83FCC8
CODE_83FCCB:
    rts                         ; $83FCCB
    lda.w $0128                 ; $83FCCC
    jsr.w $FD8A                 ; $83FCCF
    bcs CODE_83FCF1             ; $83FCD2
    lda.w $0128                 ; $83FCD4
    jsr.w $FDCD                 ; $83FCD7
    bcs CODE_83FCF1             ; $83FCDA
    lda.w $0128                 ; $83FCDC
    eor.w #$FFFF                ; $83FCDF
    inc a                       ; $83FCE2
    sta.w $0128                 ; $83FCE3
    jsr.w $FD8A                 ; $83FCE6
    bcs CODE_83FCF1             ; $83FCE9
    lda.w $0128                 ; $83FCEB
    jsr.w $FDCD                 ; $83FCEE
CODE_83FCF1:
    rts                         ; $83FCF1
    incbin !baserom:$1FCF2..$1FD26
    sep.b #$20                  ; $83FD26
    lda.b #$01                  ; $83FD28
    sta.b $53                   ; $83FD2A
    rep.b #$20                  ; $83FD2C
    jmp.w $FD45                 ; $83FD2E
    sep.b #$20                  ; $83FD31
    lda.b #$02                  ; $83FD33
    sta.b $53                   ; $83FD35
    rep.b #$20                  ; $83FD37
    jmp.w $FD45                 ; $83FD39
    sep.b #$20                  ; $83FD3C
    lda.b #$00                  ; $83FD3E
    sta.b $53                   ; $83FD40
    rep.b #$20                  ; $83FD42
    rts                         ; $83FD44
    ldx.w $011A                 ; $83FD45
    lda.l $7E8CB3,x             ; $83FD48
    and.w #$00FF                ; $83FD4C
    sta.b $B1                   ; $83FD4F
    lda.l $7E8CB4,x             ; $83FD51
    and.w #$FF00                ; $83FD55
    ora.b $B1                   ; $83FD58
    sta.b $B1                   ; $83FD5A
    rts                         ; $83FD5C
    incbin !baserom:$1FD5D..$1FD8A
    bit.w $0120                 ; $83FD8A
    bmi CODE_83FDAC             ; $83FD8D
    sec                         ; $83FD8F
    sbc.b $06                   ; $83FD90
    bpl CODE_83FDA0             ; $83FD92
    ldx.w $011C                 ; $83FD94
    cmp.l $8380CC,x             ; $83FD97
    bmi CODE_83FDAC             ; $83FD9B
    incbin !baserom:$1FD9D..$1FDA0
CODE_83FDA0:
    ldx.w $011C                 ; $83FDA0
    cmp.l $8380CE,x             ; $83FDA3
    bpl CODE_83FDAC             ; $83FDA7
    incbin !baserom:$1FDA9..$1FDAC
CODE_83FDAC:
    clc                         ; $83FDAC
    rts                         ; $83FDAD
    incbin !baserom:$1FDAE..$1FDCD
    bit.w $0120                 ; $83FDCD
    bmi CODE_83FDF5             ; $83FDD0
    bra CODE_83FDF5             ; $83FDD2
    incbin !baserom:$1FDD4..$1FDF5
CODE_83FDF5:
    clc                         ; $83FDF5
    rts                         ; $83FDF6
    incbin !baserom:$1FDF7..$1FE9A
    bit.w $047C                 ; $83FE9A
    bmi CODE_83FEB8             ; $83FE9D
    pha                         ; $83FE9F
    lda.b $23                   ; $83FEA0
    beq CODE_83FEAE             ; $83FEA2
    incbin !baserom:$1FEA4..$1FEAE
CODE_83FEAE:
    pla                         ; $83FEAE
    stz.w $0466                 ; $83FEAF
    stx.w $0465                 ; $83FEB2
    sta.w $0463                 ; $83FEB5
CODE_83FEB8:
    clc                         ; $83FEB8
    rts                         ; $83FEB9
    incbin !baserom:$1FEBA..$1FEBD
    pha                         ; $83FEBD
    asl a                       ; $83FEBE
    pha                         ; $83FEBF
    lda.b $33                   ; $83FEC0
    asl a                       ; $83FEC2
    tax                         ; $83FEC3
    lda.l $83A0A4,x             ; $83FEC4
    plx                         ; $83FEC8
    cmp.l $83A0A4,x             ; $83FEC9
    beq CODE_83FED1             ; $83FECD
    bcs CODE_83FED5             ; $83FECF
CODE_83FED1:
    pla                         ; $83FED1
    sta.b $33                   ; $83FED2
    rts                         ; $83FED4
CODE_83FED5:
    pla                         ; $83FED5
    rts                         ; $83FED6
    incbin !baserom:$1FED7..$20000
