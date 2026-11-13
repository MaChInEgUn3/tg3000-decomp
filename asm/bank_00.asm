; bank $80 (file offset $000000)
org $808000

    incbin !baserom:$0..$2
    clc                         ; $808002
    xce                         ; $808003
    cld                         ; $808004
    sep.b #$34                  ; $808005
    lda.b #$81                  ; $808007
    pha                         ; $808009
    plb                         ; $80800A
    lda.b #$00                  ; $80800B
    sta.w $4200                 ; $80800D
CODE_808010:
    lda.w $4212                 ; $808010
    bmi CODE_808010             ; $808013
CODE_808015:
    lda.w $4212                 ; $808015
    bpl CODE_808015             ; $808018
    lda.b #$00                  ; $80801A
    sta.w $420C                 ; $80801C
    lda.b #$80                  ; $80801F
    sta.w $2100                 ; $808021
CODE_808024:
    lda.w $4212                 ; $808024
    bmi CODE_808024             ; $808027
CODE_808029:
    lda.w $4212                 ; $808029
    bpl CODE_808029             ; $80802C
    rep.b #$30                  ; $80802E
    lda.w #$0100                ; $808030
    tcd                         ; $808033
    lda.w #$1FFF                ; $808034
    tcs                         ; $808037
    jml.l $80803C               ; $808038
    ldx.w #$0000                ; $80803C
    phb                         ; $80803F
    pea.w $7E7F                 ; $808040
    plb                         ; $808043
CODE_808044:
    stz.w $0000,x               ; $808044
    inx                         ; $808047
    inx                         ; $808048
    bne CODE_808044             ; $808049
    plb                         ; $80804B
CODE_80804C:
    stz.w $0000,x               ; $80804C
    inx                         ; $80804F
    cpx.w #$0365                ; $808050
    bne CODE_80804C             ; $808053
    ldx.w #$03E0                ; $808055
CODE_808058:
    stz.w $0000,x               ; $808058
    inx                         ; $80805B
    inx                         ; $80805C
    cpx.w #$FA39                ; $80805D
    bcc CODE_808058             ; $808060
    plb                         ; $808062
    ldx.w #$80A0                ; $808063
    lda.w #$0080                ; $808066
    jsr.w $81CC                 ; $808069
    jsl.l $86DD85               ; $80806C
    jsr.w $80E4                 ; $808070
    sep.b #$20                  ; $808073
    lda.b #$81                  ; $808075
    sta.w $4200                 ; $808077
    rep.b #$20                  ; $80807A
    jsr.w $8119                 ; $80807C
    phb                         ; $80807F
    pea.w $3030                 ; $808080
    plb                         ; $808083
    plb                         ; $808084
    lda.w #$FFFF                ; $808085
CODE_808088:
    cmp.l $308000               ; $808088
    bne CODE_808088             ; $80808C
    ldx.w #$0100                ; $80808E
CODE_808091:
    cmp.l $308000               ; $808091
    bne CODE_808088             ; $808095
    dex                         ; $808097
    bne CODE_808091             ; $808098
    plb                         ; $80809A
    jsr.w $89DB                 ; $80809B
    incbin !baserom:$9E..$E4
    sep.b #$20                  ; $8080E4
    lda.b #$5C                  ; $8080E6
    sta.w $0000                 ; $8080E8
    sta.w $0004                 ; $8080EB
    lda.b #$80                  ; $8080EE
    sta.w $0003                 ; $8080F0
    lda.b #$80                  ; $8080F3
    sta.w $0007                 ; $8080F5
    rep.b #$20                  ; $8080F8
    lda.w #$8807                ; $8080FA
    sta.w $0001                 ; $8080FD
    lda.w #$89B2                ; $808100
    sta.w $0005                 ; $808103
    lda.w #$8934                ; $808106
    sta.w $0008                 ; $808109
    stz.w $0026                 ; $80810C
    stz.w $019A                 ; $80810F
    stz.w $002A                 ; $808112
    stz.w $0028                 ; $808115
    rts                         ; $808118
    stz.w $03E2                 ; $808119
    ldx.w #$0078                ; $80811C
    stz.w $0114                 ; $80811F
CODE_808122:
    lda.l $7EFA3C,x             ; $808122
    and.w #$00FF                ; $808126
    clc                         ; $808129
    adc.w $0114                 ; $80812A
    sta.w $0114                 ; $80812D
    dex                         ; $808130
    bne CODE_808122             ; $808131
    lda.w $0114                 ; $808133
    eor.w #$7645                ; $808136
    cmp.l $7EFA3B               ; $808139
    bne CODE_808155             ; $80813D
    incbin !baserom:$13F..$155
CODE_808155:
    ldx.w #$0078                ; $808155
    stz.w $0114                 ; $808158
CODE_80815B:
    lda.l $810367,x             ; $80815B
    and.w #$00FF                ; $80815F
    clc                         ; $808162
    adc.w $0114                 ; $808163
    sta.w $0114                 ; $808166
    dex                         ; $808169
    bne CODE_80815B             ; $80816A
    lda.w $0114                 ; $80816C
    eor.w #$7645                ; $80816F
    cmp.w $0366                 ; $808172
    bne CODE_80817F             ; $808175
    incbin !baserom:$177..$17F
CODE_80817F:
    ldx.w #$85A9                ; $80817F
    ldy.w #$0366                ; $808182
    lda.w #$0079                ; $808185
    phb                         ; $808188
    mvn $81,$83                 ; $808189
    plb                         ; $80818C
    jsl.l $808199               ; $80818D
    lda.w #$00FF                ; $808191
    tsb.w $045A                 ; $808194
    sec                         ; $808197
    rts                         ; $808198
    ldx.w #$0078                ; $808199
    stz.w $0114                 ; $80819C
CODE_80819F:
    lda.l $810367,x             ; $80819F
    and.w #$00FF                ; $8081A3
    clc                         ; $8081A6
    adc.w $0114                 ; $8081A7
    sta.w $0114                 ; $8081AA
    dex                         ; $8081AD
    bne CODE_80819F             ; $8081AE
    lda.w $0114                 ; $8081B0
    eor.w #$7645                ; $8081B3
    sta.w $0366                 ; $8081B6
    phb                         ; $8081B9
    ldx.w #$0366                ; $8081BA
    ldy.w #$FA3B                ; $8081BD
    lda.w #$0079                ; $8081C0
    mvn $7E,$81                 ; $8081C3
    plb                         ; $8081C6
    rtl                         ; $8081C7
    jsr.w $81CC                 ; $8081C8
    rtl                         ; $8081CB
    sep.b #$20                  ; $8081CC
    phb                         ; $8081CE
    pha                         ; $8081CF
    plb                         ; $8081D0
    stz.w $420B                 ; $8081D1
    stz.w $420C                 ; $8081D4
    lda.w $0000,x               ; $8081D7
    sta.w $2100                 ; $8081DA
    lda.w $0001,x               ; $8081DD
    sta.w $2101                 ; $8081E0
    lda.w $0002,x               ; $8081E3
    sta.w $2102                 ; $8081E6
    lda.w $0003,x               ; $8081E9
    sta.w $2103                 ; $8081EC
    lda.w $0004,x               ; $8081EF
    sta.w $2105                 ; $8081F2
    lda.w $0005,x               ; $8081F5
    sta.w $2106                 ; $8081F8
    lda.w $0006,x               ; $8081FB
    sta.w $2107                 ; $8081FE
    lda.w $0007,x               ; $808201
    sta.w $2108                 ; $808204
    lda.w $0008,x               ; $808207
    sta.w $2109                 ; $80820A
    lda.w $0009,x               ; $80820D
    sta.w $210A                 ; $808210
    lda.w $000A,x               ; $808213
    sta.w $210B                 ; $808216
    lda.w $000B,x               ; $808219
    sta.w $210C                 ; $80821C
    lda.w $000C,x               ; $80821F
    sta.w $210D                 ; $808222
    lda.w $000D,x               ; $808225
    sta.w $210D                 ; $808228
    lda.w $000E,x               ; $80822B
    sta.w $210E                 ; $80822E
    lda.w $000F,x               ; $808231
    sta.w $210E                 ; $808234
    lda.w $0010,x               ; $808237
    sta.w $210F                 ; $80823A
    lda.w $0011,x               ; $80823D
    sta.w $210F                 ; $808240
    lda.w $0012,x               ; $808243
    sta.w $2110                 ; $808246
    lda.w $0013,x               ; $808249
    sta.w $2110                 ; $80824C
    lda.w $0014,x               ; $80824F
    sta.w $2111                 ; $808252
    lda.w $0015,x               ; $808255
    sta.w $2111                 ; $808258
    lda.w $0016,x               ; $80825B
    sta.w $2112                 ; $80825E
    lda.w $0017,x               ; $808261
    sta.w $2112                 ; $808264
    lda.w $0018,x               ; $808267
    sta.w $2113                 ; $80826A
    lda.w $0019,x               ; $80826D
    sta.w $2113                 ; $808270
    lda.w $001A,x               ; $808273
    sta.w $2114                 ; $808276
    lda.w $001B,x               ; $808279
    sta.w $2114                 ; $80827C
    lda.w $001C,x               ; $80827F
    sta.w $2115                 ; $808282
    lda.w $001D,x               ; $808285
    sta.w $2116                 ; $808288
    lda.w $001E,x               ; $80828B
    sta.w $2117                 ; $80828E
    lda.w $001F,x               ; $808291
    sta.w $211A                 ; $808294
    lda.w $0020,x               ; $808297
    sta.w $211B                 ; $80829A
    lda.w $0021,x               ; $80829D
    sta.w $211B                 ; $8082A0
    lda.w $0022,x               ; $8082A3
    sta.w $211C                 ; $8082A6
    lda.w $0023,x               ; $8082A9
    sta.w $211C                 ; $8082AC
    lda.w $0024,x               ; $8082AF
    sta.w $211D                 ; $8082B2
    lda.w $0025,x               ; $8082B5
    sta.w $211D                 ; $8082B8
    lda.w $0026,x               ; $8082BB
    sta.w $211E                 ; $8082BE
    lda.w $0027,x               ; $8082C1
    sta.w $211E                 ; $8082C4
    lda.w $0028,x               ; $8082C7
    sta.w $211F                 ; $8082CA
    lda.w $0029,x               ; $8082CD
    sta.w $211F                 ; $8082D0
    lda.w $002A,x               ; $8082D3
    sta.w $2120                 ; $8082D6
    lda.w $002B,x               ; $8082D9
    sta.w $2120                 ; $8082DC
    lda.w $002C,x               ; $8082DF
    sta.w $2121                 ; $8082E2
    lda.w $002D,x               ; $8082E5
    sta.w $2123                 ; $8082E8
    lda.w $002E,x               ; $8082EB
    sta.w $2124                 ; $8082EE
    lda.w $002F,x               ; $8082F1
    sta.w $2125                 ; $8082F4
    lda.w $0030,x               ; $8082F7
    sta.w $2126                 ; $8082FA
    lda.w $0031,x               ; $8082FD
    sta.w $2127                 ; $808300
    lda.w $0032,x               ; $808303
    sta.w $2128                 ; $808306
    lda.w $0033,x               ; $808309
    sta.w $2129                 ; $80830C
    lda.w $0034,x               ; $80830F
    sta.w $212A                 ; $808312
    lda.w $0035,x               ; $808315
    sta.w $212B                 ; $808318
    lda.w $0036,x               ; $80831B
    sta.w $212C                 ; $80831E
    lda.w $0037,x               ; $808321
    sta.w $212D                 ; $808324
    lda.w $0038,x               ; $808327
    sta.w $212E                 ; $80832A
    lda.w $0039,x               ; $80832D
    sta.w $212F                 ; $808330
    lda.w $003A,x               ; $808333
    sta.w $2130                 ; $808336
    lda.w $003B,x               ; $808339
    sta.w $2131                 ; $80833C
    lda.w $003C,x               ; $80833F
    sta.w $2132                 ; $808342
    lda.w $003D,x               ; $808345
    sta.w $2133                 ; $808348
    lda.w $003E,x               ; $80834B
    sta.w $4200                 ; $80834E
    lda.w $003F,x               ; $808351
    sta.w $4207                 ; $808354
    lda.w $0040,x               ; $808357
    sta.w $4208                 ; $80835A
    lda.w $0041,x               ; $80835D
    sta.w $4209                 ; $808360
    lda.w $0042,x               ; $808363
    sta.w $420A                 ; $808366
    lda.w $0043,x               ; $808369
    sta.w $420D                 ; $80836C
    rep.b #$20                  ; $80836F
    plb                         ; $808371
    rts                         ; $808372
    clc                         ; $808373
    adc.w $0024                 ; $808374
CODE_808377:
    cmp.w $0024                 ; $808377
    bne CODE_808377             ; $80837A
    rts                         ; $80837C
    jsr.w $8381                 ; $80837D
    rtl                         ; $808380
    ora.w #$7F00                ; $808381
    phy                         ; $808384
    ldy.w #$0000                ; $808385
    jsr.w $FBA0                 ; $808388
    ldy.w #$1801                ; $80838B
    sty.w $4300                 ; $80838E
    ldy.w #$0000                ; $808391
    sty.w $4302                 ; $808394
    sta.w $4305                 ; $808397
    pla                         ; $80839A
    sta.w $2116                 ; $80839B
    sep.b #$20                  ; $80839E
    lda.b #$7F                  ; $8083A0
    sta.w $4304                 ; $8083A2
    lda.b #$01                  ; $8083A5
    sta.w $420B                 ; $8083A7
    rep.b #$20                  ; $8083AA
    rts                         ; $8083AC
    sta.w $4305                 ; $8083AD
    lda.w #$1801                ; $8083B0
    sta.w $4300                 ; $8083B3
    stx.w $4302                 ; $8083B6
    sty.w $2116                 ; $8083B9
    sep.b #$20                  ; $8083BC
    lda.b $14                   ; $8083BE
    sta.w $4304                 ; $8083C0
    lda.b #$01                  ; $8083C3
    sta.w $420B                 ; $8083C5
    rep.b #$20                  ; $8083C8
    rtl                         ; $8083CA
    stx.w $4302                 ; $8083CB
    sty.w $4305                 ; $8083CE
    ldx.w #$2202                ; $8083D1
    stx.w $4300                 ; $8083D4
    sep.b #$20                  ; $8083D7
    sta.w $2121                 ; $8083D9
    xba                         ; $8083DC
    sta.w $4304                 ; $8083DD
    lda.b #$01                  ; $8083E0
    sta.w $420B                 ; $8083E2
    rep.b #$20                  ; $8083E5
    rtl                         ; $8083E7
    ldx.w #$0200                ; $8083E8
    lda.w #$E0E0                ; $8083EB
CODE_8083EE:
    sta.w $08FE,x               ; $8083EE
    dex                         ; $8083F1
    dex                         ; $8083F2
    bne CODE_8083EE             ; $8083F3
    stz.w $0172                 ; $8083F5
    stz.w $0174                 ; $8083F8
    rts                         ; $8083FB
    phx                         ; $8083FC
    ldx.w $0028                 ; $8083FD
    sta.w $1900,x               ; $808400
    tya                         ; $808403
    sta.w $19C0,x               ; $808404
    pla                         ; $808407
    sta.w $1940,x               ; $808408
    lda.b $14                   ; $80840B
    sta.w $1980,x               ; $80840D
    inx                         ; $808410
    inx                         ; $808411
    txa                         ; $808412
    and.w #$003F                ; $808413
    sta.w $0028                 ; $808416
    rts                         ; $808419
    phb                         ; $80841A
    sep.b #$20                  ; $80841B
    pha                         ; $80841D
    plb                         ; $80841E
    sty.b $16                   ; $80841F
    ldy.b $74                   ; $808421
    lda.w $0000,x               ; $808423
    beq CODE_80842D             ; $808426
    bpl CODE_808435             ; $808428
    incbin !baserom:$42A..$42D
CODE_80842D:
    plb                         ; $80842D
    rep.b #$20                  ; $80842E
    sty.b $74                   ; $808430
    ldy.b $16                   ; $808432
    rts                         ; $808434
CODE_808435:
    phx                         ; $808435
    cmp.b #$20                  ; $808436
    bne CODE_808445             ; $808438
    lda.b $16                   ; $80843A
    clc                         ; $80843C
    adc.b #$06                  ; $80843D
    sta.b $16                   ; $80843F
    plx                         ; $808441
    jmp.w $84AC                 ; $808442
CODE_808445:
    cmp.b #$30                  ; $808445
    db $90,$23 ; bcc            ; $808447
    cmp.b #$3A                  ; $808449
    bcs CODE_808452             ; $80844B
    incbin !baserom:$44D..$452
CODE_808452:
    cmp.b #$41                  ; $808452
    db $90,$16 ; bcc            ; $808454
    cmp.b #$5B                  ; $808456
    bcs CODE_80845F             ; $808458
    sbc.b #$26                  ; $80845A
    jmp.w $847B                 ; $80845C
CODE_80845F:
    cmp.b #$61                  ; $80845F
    db $90,$09 ; bcc            ; $808461
    cmp.b #$7B                  ; $808463
    db $B0,$05 ; bcs            ; $808465
    sbc.b #$60                  ; $808467
    jmp.w $847B                 ; $808469
    incbin !baserom:$46C..$47B
    rep.b #$20                  ; $80847B
    and.w #$00FF                ; $80847D
    tax                         ; $808480
    bra CODE_808486             ; $808481
    incbin !baserom:$483..$486
CODE_808486:
    rep.b #$20                  ; $808486
    and.w #$00FF                ; $808488
    clc                         ; $80848B
    adc.b $14                   ; $80848C
    sta.w $0902,y               ; $80848E
    lda.b $16                   ; $808491
    sta.w $0900,y               ; $808493
    lda.w #$0000                ; $808496
    sta.w $0B01,y               ; $808499
    sep.b #$20                  ; $80849C
    lda.l $8085E5,x             ; $80849E
    clc                         ; $8084A2
    adc.b $16                   ; $8084A3
    sta.b $16                   ; $8084A5
    iny                         ; $8084A7
    iny                         ; $8084A8
    iny                         ; $8084A9
    iny                         ; $8084AA
    plx                         ; $8084AB
    inx                         ; $8084AC
    jmp.w $8423                 ; $8084AD
    incbin !baserom:$4B0..$4BE
    sta.b $16                   ; $8084BE
    lda.b $74                   ; $8084C0
    pha                         ; $8084C2
    phy                         ; $8084C3
    lda.b $16                   ; $8084C4
    jsr.w $841A                 ; $8084C6
    tya                         ; $8084C9
    and.w #$00FF                ; $8084CA
    sta.b $14                   ; $8084CD
    pla                         ; $8084CF
    and.w #$00FF                ; $8084D0
    sec                         ; $8084D3
    sbc.b $14                   ; $8084D4
    lsr a                       ; $8084D6
    sta.b $14                   ; $8084D7
    ply                         ; $8084D9
    sep.b #$20                  ; $8084DA
CODE_8084DC:
    lda.w $0900,y               ; $8084DC
    clc                         ; $8084DF
    adc.b $14                   ; $8084E0
    sta.w $0900,y               ; $8084E2
    iny                         ; $8084E5
    iny                         ; $8084E6
    iny                         ; $8084E7
    iny                         ; $8084E8
    cpy.b $74                   ; $8084E9
    bne CODE_8084DC             ; $8084EB
    rep.b #$20                  ; $8084ED
    rts                         ; $8084EF
    sty.b $16                   ; $8084F0
    stx.b $18                   ; $8084F2
    sta.b $1A                   ; $8084F4
    ldy.b $74                   ; $8084F6
    ldx.w #$0007                ; $8084F8
CODE_8084FB:
    jsr.w $8542                 ; $8084FB
    bne CODE_808503             ; $8084FE
    dex                         ; $808500
    bpl CODE_8084FB             ; $808501
CODE_808503:
    phx                         ; $808503
    clc                         ; $808504
    adc.w #$0034                ; $808505
    tax                         ; $808508
    clc                         ; $808509
    adc.b $14                   ; $80850A
    sta.w $0902,y               ; $80850C
    lda.w #$0000                ; $80850F
    sta.w $0B01,y               ; $808512
    lda.b $16                   ; $808515
    sta.w $0900,y               ; $808517
    lda.l $8085E5,x             ; $80851A
    and.w #$00FF                ; $80851E
    clc                         ; $808521
    adc.b $16                   ; $808522
    sta.b $16                   ; $808524
    iny                         ; $808526
    iny                         ; $808527
    iny                         ; $808528
    iny                         ; $808529
    jsr.w $8542                 ; $80852A
    plx                         ; $80852D
    cpx.w #$0006                ; $80852E
    beq CODE_808538             ; $808531
    cpx.w #$0003                ; $808533
    bne CODE_80853A             ; $808536
CODE_808538:
    inc.b $16                   ; $808538
CODE_80853A:
    dex                         ; $80853A
    bpl CODE_808503             ; $80853B
    sty.b $74                   ; $80853D
    ldy.b $16                   ; $80853F
    rts                         ; $808541
    lda.w #$0000                ; $808542
    asl.b $1A                   ; $808545
    rol.b $18                   ; $808547
    rol a                       ; $808549
    asl.b $1A                   ; $80854A
    rol.b $18                   ; $80854C
    rol a                       ; $80854E
    asl.b $1A                   ; $80854F
    rol.b $18                   ; $808551
    rol a                       ; $808553
    asl.b $1A                   ; $808554
    rol.b $18                   ; $808556
    rol a                       ; $808558
    rts                         ; $808559
    sty.b $16                   ; $80855A
    sta.b $1A                   ; $80855C
    ldy.b $74                   ; $80855E
    ldx.w #$0004                ; $808560
CODE_808563:
    jsr.w $85CA                 ; $808563
    bne CODE_808570             ; $808566
    cpx.w #$0003                ; $808568
    beq CODE_808570             ; $80856B
    dex                         ; $80856D
    bra CODE_808563             ; $80856E
CODE_808570:
    phx                         ; $808570
    clc                         ; $808571
    adc.w #$0034                ; $808572
    tax                         ; $808575
    clc                         ; $808576
    adc.b $14                   ; $808577
    sta.w $0902,y               ; $808579
    lda.w #$0000                ; $80857C
    sta.w $0B01,y               ; $80857F
    lda.l $8085A6,x             ; $808582
    and.w #$00FF                ; $808586
    clc                         ; $808589
    adc.b $16                   ; $80858A
    sta.w $0900,y               ; $80858C
    lda.w #$0008                ; $80858F
    clc                         ; $808592
    adc.b $16                   ; $808593
    sta.b $16                   ; $808595
    iny                         ; $808597
    iny                         ; $808598
    iny                         ; $808599
    iny                         ; $80859A
    plx                         ; $80859B
    cpx.w #$0003                ; $80859C
    bne CODE_8085BF             ; $80859F
    lda.w #$0041                ; $8085A1
    clc                         ; $8085A4
    adc.b $14                   ; $8085A5
    sta.w $0902,y               ; $8085A7
    lda.w #$0000                ; $8085AA
    sta.w $0B01,y               ; $8085AD
    lda.b $16                   ; $8085B0
    sta.w $0900,y               ; $8085B2
    clc                         ; $8085B5
    adc.w #$0003                ; $8085B6
    sta.b $16                   ; $8085B9
    iny                         ; $8085BB
    iny                         ; $8085BC
    iny                         ; $8085BD
    iny                         ; $8085BE
CODE_8085BF:
    jsr.w $85CA                 ; $8085BF
    dex                         ; $8085C2
    bne CODE_808570             ; $8085C3
    sty.b $74                   ; $8085C5
    ldy.b $16                   ; $8085C7
    rts                         ; $8085C9
    lda.w #$0000                ; $8085CA
    asl.b $1A                   ; $8085CD
    rol a                       ; $8085CF
    asl.b $1A                   ; $8085D0
    rol a                       ; $8085D2
    asl.b $1A                   ; $8085D3
    rol a                       ; $8085D5
    asl.b $1A                   ; $8085D6
    rol a                       ; $8085D8
    rts                         ; $8085D9
    incbin !baserom:$5DA..$667
    sta.b $16                   ; $808667
    lda.b $74                   ; $808669
    pha                         ; $80866B
    phy                         ; $80866C
    lda.b $16                   ; $80866D
    jsr.w $855A                 ; $80866F
    tya                         ; $808672
    and.w #$00FF                ; $808673
    sta.b $14                   ; $808676
    pla                         ; $808678
    and.w #$00FF                ; $808679
    sec                         ; $80867C
    sbc.b $14                   ; $80867D
    sta.b $14                   ; $80867F
    ply                         ; $808681
    sep.b #$20                  ; $808682
CODE_808684:
    lda.w $0900,y               ; $808684
    clc                         ; $808687
    adc.b $14                   ; $808688
    sta.w $0900,y               ; $80868A
    iny                         ; $80868D
    iny                         ; $80868E
    iny                         ; $80868F
    iny                         ; $808690
    cpy.b $74                   ; $808691
    bne CODE_808684             ; $808693
    rep.b #$20                  ; $808695
    rts                         ; $808697
    sta.b $16                   ; $808698
    lda.b $74                   ; $80869A
    pha                         ; $80869C
    phy                         ; $80869D
    lda.b $16                   ; $80869E
    jsr.w $84F0                 ; $8086A0
    tya                         ; $8086A3
    and.w #$00FF                ; $8086A4
    sta.b $14                   ; $8086A7
    pla                         ; $8086A9
    and.w #$00FF                ; $8086AA
    sec                         ; $8086AD
    sbc.b $14                   ; $8086AE
    sta.b $14                   ; $8086B0
    ply                         ; $8086B2
    sep.b #$20                  ; $8086B3
CODE_8086B5:
    lda.w $0900,y               ; $8086B5
    clc                         ; $8086B8
    adc.b $14                   ; $8086B9
    sta.w $0900,y               ; $8086BB
    iny                         ; $8086BE
    iny                         ; $8086BF
    iny                         ; $8086C0
    iny                         ; $8086C1
    cpy.b $74                   ; $8086C2
    bne CODE_8086B5             ; $8086C4
    rep.b #$20                  ; $8086C6
    rts                         ; $8086C8
    stz.b $16                   ; $8086C9
    stz.b $18                   ; $8086CB
    ldx.w #$0008                ; $8086CD
CODE_8086D0:
    sta.w $4204                 ; $8086D0
    lda.w #$000A                ; $8086D3
    sep.b #$20                  ; $8086D6
    sta.w $4206                 ; $8086D8
    nop                         ; $8086DB
    nop                         ; $8086DC
    xba                         ; $8086DD
    xba                         ; $8086DE
    xba                         ; $8086DF
    rep.b #$20                  ; $8086E0
    lda.w $4216                 ; $8086E2
    lsr a                       ; $8086E5
    ror.b $18                   ; $8086E6
    ror.b $16                   ; $8086E8
    lsr a                       ; $8086EA
    ror.b $18                   ; $8086EB
    ror.b $16                   ; $8086ED
    lsr a                       ; $8086EF
    ror.b $18                   ; $8086F0
    ror.b $16                   ; $8086F2
    lsr a                       ; $8086F4
    ror.b $18                   ; $8086F5
    ror.b $16                   ; $8086F7
    lda.w $4214                 ; $8086F9
    dex                         ; $8086FC
    bne CODE_8086D0             ; $8086FD
    ldx.b $18                   ; $8086FF
    lda.b $16                   ; $808701
    rts                         ; $808703
    incbin !baserom:$704..$7DF
    phd                         ; $8087DF
    lda.w #$0100                ; $8087E0
    tcd                         ; $8087E3
    jsr.w $87E9                 ; $8087E4
    pld                         ; $8087E7
    rtl                         ; $8087E8
    lda.b $12                   ; $8087E9
    bmi CODE_808800             ; $8087EB
    lda.b $10                   ; $8087ED
    asl a                       ; $8087EF
    eor.w #$2B45                ; $8087F0
    sta.b $10                   ; $8087F3
    lda.b $12                   ; $8087F5
    rol a                       ; $8087F7
    eor.w #$1D87                ; $8087F8
    sta.b $12                   ; $8087FB
    lda.b $10                   ; $8087FD
    rts                         ; $8087FF
CODE_808800:
    asl.b $10                   ; $808800
    rol.b $12                   ; $808802
    lda.b $10                   ; $808804
    rts                         ; $808806
    rep.b #$30                  ; $808807
    phb                         ; $808809
    pha                         ; $80880A
    phx                         ; $80880B
    phy                         ; $80880C
    phd                         ; $80880D
    phk                         ; $80880E
    plb                         ; $80880F
    sep.b #$20                  ; $808810
    lda.w $4210                 ; $808812
    lda.b #$81                  ; $808815
    pha                         ; $808817
    plb                         ; $808818
    rep.b #$20                  ; $808819
    lda.w #$0000                ; $80881B
    tcd                         ; $80881E
    lda.w #$1000                ; $80881F
    sta.b $2C                   ; $808822
    lda.w $0196                 ; $808824
    beq CODE_808844             ; $808827
    clc                         ; $808829
    adc.w $019A                 ; $80882A
    sta.w $019A                 ; $80882D
    sec                         ; $808830
    sbc.w $0198                 ; $808831
    beq CODE_80883B             ; $808834
    eor.w $0196                 ; $808836
    bmi CODE_808844             ; $808839
CODE_80883B:
    lda.w $0198                 ; $80883B
    sta.w $019A                 ; $80883E
    stz.w $0196                 ; $808841
CODE_808844:
    sep.b #$20                  ; $808844
    lda.w $019B                 ; $808846
    bne CODE_80884D             ; $808849
    ora.b #$80                  ; $80884B
CODE_80884D:
    sta.w $2100                 ; $80884D
    rep.b #$20                  ; $808850
    lda.w #$0001                ; $808852
    trb.b $26                   ; $808855
    beq CODE_808884             ; $808857
    lda.w #$2202                ; $808859
    sta.w $4300                 ; $80885C
    lda.w #$0500                ; $80885F
    sta.w $4302                 ; $808862
    lda.w #$0200                ; $808865
    sta.w $4305                 ; $808868
    sep.b #$20                  ; $80886B
    stz.w $2121                 ; $80886D
    lda.b #$81                  ; $808870
    sta.w $4304                 ; $808872
    lda.b #$01                  ; $808875
    sta.w $420B                 ; $808877
    rep.b #$20                  ; $80887A
    lda.b $2C                   ; $80887C
    sec                         ; $80887E
    sbc.w #$0280                ; $80887F
    sta.b $2C                   ; $808882
CODE_808884:
    lda.w #$0002                ; $808884
    trb.b $26                   ; $808887
    beq CODE_8088B9             ; $808889
    stz.w $2102                 ; $80888B
    lda.w #$0402                ; $80888E
    sta.w $4300                 ; $808891
    lda.w #$0900                ; $808894
    sta.w $4302                 ; $808897
    lda.w #$0220                ; $80889A
    sta.w $4305                 ; $80889D
    sep.b #$20                  ; $8088A0
    lda.b #$81                  ; $8088A2
    sta.w $4304                 ; $8088A4
    lda.b #$01                  ; $8088A7
    sta.w $420B                 ; $8088A9
    rep.b #$20                  ; $8088AC
    stz.w $2102                 ; $8088AE
    lda.b $2C                   ; $8088B1
    sec                         ; $8088B3
    sbc.w #$0280                ; $8088B4
    sta.b $2C                   ; $8088B7
CODE_8088B9:
    ldx.b $2A                   ; $8088B9
    cpx.b $28                   ; $8088BB
    beq CODE_808901             ; $8088BD
    lda.w #$1801                ; $8088BF
    sta.w $4300                 ; $8088C2
CODE_8088C5:
    lda.w $19C0,x               ; $8088C5
    sta.w $2116                 ; $8088C8
    lda.w $1940,x               ; $8088CB
    sta.w $4302                 ; $8088CE
    lda.w $1900,x               ; $8088D1
    cmp.b $2C                   ; $8088D4
    beq CODE_8088DA             ; $8088D6
    bcs CODE_808903             ; $8088D8
CODE_8088DA:
    sta.w $4305                 ; $8088DA
    eor.w #$FFFF                ; $8088DD
    sec                         ; $8088E0
    adc.b $2C                   ; $8088E1
    sta.b $2C                   ; $8088E3
    sep.b #$20                  ; $8088E5
    lda.w $1980,x               ; $8088E7
    sta.w $4304                 ; $8088EA
    lda.b #$01                  ; $8088ED
    sta.w $420B                 ; $8088EF
    rep.b #$20                  ; $8088F2
    inx                         ; $8088F4
    inx                         ; $8088F5
    txa                         ; $8088F6
    and.w #$003F                ; $8088F7
    sta.b $2A                   ; $8088FA
    tax                         ; $8088FC
    cmp.b $28                   ; $8088FD
    bne CODE_8088C5             ; $8088FF
CODE_808901:
    bra CODE_808931             ; $808901
CODE_808903:
    sbc.b $2C                   ; $808903
    sta.w $1900,x               ; $808905
    lda.w $1940,x               ; $808908
    clc                         ; $80890B
    adc.b $2C                   ; $80890C
    sta.w $1940,x               ; $80890E
    lda.b $2C                   ; $808911
    lsr a                       ; $808913
    clc                         ; $808914
    adc.w $19C0,x               ; $808915
    sta.w $19C0,x               ; $808918
    lda.b $2C                   ; $80891B
    sta.w $4305                 ; $80891D
    sep.b #$20                  ; $808920
    lda.w $1980,x               ; $808922
    sta.w $4304                 ; $808925
    lda.b #$01                  ; $808928
    sta.w $420B                 ; $80892A
    rep.b #$20                  ; $80892D
    stz.b $2C                   ; $80892F
CODE_808931:
    jmp.w ($0008)               ; $808931
    lda.b $26                   ; $808934
    bit.w #$0004                ; $808936
    bne CODE_80893F             ; $808939
    jsl.l $8386FB               ; $80893B
CODE_80893F:
    jsr.w $894E                 ; $80893F
    jsl.l $86DFE1               ; $808942
    inc.b $24                   ; $808946
    pld                         ; $808948
    ply                         ; $808949
    plx                         ; $80894A
    pla                         ; $80894B
    plb                         ; $80894C
    rti                         ; $80894D
    ldx.w $1B24                 ; $80894E
    lda.w $01FB                 ; $808951
    jsr.w $899F                 ; $808954
    lda.w $01FC                 ; $808957
    jsr.w $899F                 ; $80895A
    lda.w $02FB                 ; $80895D
    jsr.w $899F                 ; $808960
    lda.w $02FC                 ; $808963
    jsr.w $899F                 ; $808966
    stx.w $1B24                 ; $808969
    lda.w #$00FF                ; $80896C
    trb.w $01FB                 ; $80896F
    trb.w $01FC                 ; $808972
    trb.w $02FB                 ; $808975
    trb.w $02FC                 ; $808978
    cpx.w $1B26                 ; $80897B
    beq CODE_80899E             ; $80897E
    ldx.w $1B26                 ; $808980
    stz.w $0466                 ; $808983
    lda.w #$FF00                ; $808986
    sta.w $0465                 ; $808989
    lda.w $1B30,x               ; $80898C
    ora.w #$FF00                ; $80898F
    sta.w $0463                 ; $808992
    inx                         ; $808995
    txa                         ; $808996
    and.w #$0007                ; $808997
    tax                         ; $80899A
    stx.w $1B26                 ; $80899B
CODE_80899E:
    rts                         ; $80899E
    and.w #$00FF                ; $80899F
    beq CODE_8089B1             ; $8089A2
    sep.b #$20                  ; $8089A4
    sta.w $1B30,x               ; $8089A6
    rep.b #$20                  ; $8089A9
    inx                         ; $8089AB
    txa                         ; $8089AC
    and.w #$0007                ; $8089AD
    tax                         ; $8089B0
CODE_8089B1:
    rts                         ; $8089B1
    incbin !baserom:$9B2..$9D1
    jsl.l $8180D0               ; $8089D1
    rts                         ; $8089D5
    jsl.l $8180DA               ; $8089D6
    rts                         ; $8089DA
    lda.w $0459                 ; $8089DB
    db $10,$24 ; bpl            ; $8089DE
    jsl.l $80FC20               ; $8089E0
    lda.w #$0000                ; $8089E4
    jsl.l $86DDEF               ; $8089E7
    sep.b #$20                  ; $8089EB
    lda.b #$01                  ; $8089ED
    sta.w $045F                 ; $8089EF
    lda.b #$7F                  ; $8089F2
    sta.w $045D                 ; $8089F4
    sta.w $045C                 ; $8089F7
    sta.w $045E                 ; $8089FA
    rep.b #$20                  ; $8089FD
    jsr.w $EE56                 ; $8089FF
    bra CODE_808A1F             ; $808A02
    incbin !baserom:$A04..$A1F
CODE_808A1F:
    jsl.l $84A4ED               ; $808A1F
    jsl.l $80FC2A               ; $808A23
    jsl.l $808199               ; $808A27
    lda.w $03DC                 ; $808A2B
    bit.w #$5000                ; $808A2E
    bne CODE_808A37             ; $808A31
    jsl.l $84A4ED               ; $808A33
CODE_808A37:
    lda.w #$8000                ; $808A37
    trb.w $03DC                 ; $808A3A
    beq CODE_808A53             ; $808A3D
    incbin !baserom:$A3F..$A53
CODE_808A53:
    lda.w #$4000                ; $808A53
    trb.w $03DC                 ; $808A56
    lda.w $03DC                 ; $808A59
    bit.w #$2000                ; $808A5C
    beq CODE_808A71             ; $808A5F
    incbin !baserom:$A61..$A71
CODE_808A71:
    lda.w #$00FF                ; $808A71
    trb.w $047D                 ; $808A74
    lda.w $03DC                 ; $808A77
    and.w #$000F                ; $808A7A
    cmp.w #$0002                ; $808A7D
    db $F0,$05 ; beq            ; $808A80
    jsr.w $8A8C                 ; $808A82
    incbin !baserom:$A85..$A8C
    jsr.w $BEC6                 ; $808A8C
    jsl.l $808199               ; $808A8F
    jsl.l $8A9CB0               ; $808A93
    jsl.l $8AC2DD               ; $808A97
    lda.w $0444                 ; $808A9B
    beq CODE_808AB8             ; $808A9E
    incbin !baserom:$AA0..$AB8
CODE_808AB8:
    jsl.l $84A34B               ; $808AB8
    jsr.w $8BC5                 ; $808ABC
    jsl.l $84A31B               ; $808ABF
    lda.w #$FFFF                ; $808AC3
    sta.l $8103E0               ; $808AC6
    lda.w $0444                 ; $808ACA
    sta.w $047B                 ; $808ACD
    jsl.l $828342               ; $808AD0
    jsl.l $84A4CD               ; $808AD4
    lda.w #$0001                ; $808AD8
    jsl.l $84A4B7               ; $808ADB
    lda.w #$0001                ; $808ADF
    sta.l $8103E0               ; $808AE2
    lda.w #$FFFF                ; $808AE6
    cmp.w $0446                 ; $808AE9
    db $F0,$23 ; beq            ; $808AEC
    jsr.w $E4E6                 ; $808AEE
    lda.w $03E2                 ; $808AF1
    bit.w #$0001                ; $808AF4
    bne CODE_808B12             ; $808AF7
    lda.w $03DC                 ; $808AF9
    and.w #$00F0                ; $808AFC
    cmp.w #$0030                ; $808AFF
    db $F0,$05 ; beq            ; $808B02
    lda.w $03A8                 ; $808B04
    bra CODE_808B0F             ; $808B07
    incbin !baserom:$B09..$B0F
CODE_808B0F:
    bpl CODE_808B12             ; $808B0F
    incbin !baserom:$B11..$B12
CODE_808B12:
    jsl.l $84A388               ; $808B12
    jsl.l $80FBDC               ; $808B16
    lda.w $047B                 ; $808B1A
    jsl.l $80FBA5               ; $808B1D
    jsl.l $84A41B               ; $808B21
    lda.w $0444                 ; $808B25
    cmp.w #$00FF                ; $808B28
    db $F0,$32 ; beq            ; $808B2B
    lda.w $0444                 ; $808B2D
    sta.w $047B                 ; $808B30
    ldx.w $0444                 ; $808B33
    lda.l $84A240,x             ; $808B36
    and.w #$00FF                ; $808B3A
    cmp.w $0368                 ; $808B3D
    beq CODE_808B59             ; $808B40
    lda.w $03E2                 ; $808B42
    bit.w #$0001                ; $808B45
    bne CODE_808B55             ; $808B48
    jsl.l $8A9CB0               ; $808B4A
    bcc CODE_808B55             ; $808B4E
    incbin !baserom:$B50..$B55
CODE_808B55:
    jsl.l $80FC25               ; $808B55
CODE_808B59:
    jsr.w $BE32                 ; $808B59
    jmp.w $8AB8                 ; $808B5C
    incbin !baserom:$B5F..$BC5
    stz.w $019A                 ; $808BC5
    ldx.w #$A26F                ; $808BC8
    lda.w #$0084                ; $808BCB
    jsr.w $81CC                 ; $808BCE
    lda.w #$0001                ; $808BD1
    jsr.w $8373                 ; $808BD4
    ldx.w #$8B62                ; $808BD7
    ldy.w #$2000                ; $808BDA
    lda.w #$7E81                ; $808BDD
    jsr.w $FBA0                 ; $808BE0
    ldy.w #$0010                ; $808BE3
    ldx.w #$8000                ; $808BE6
    lda.w #$008D                ; $808BE9
    jsr.w $8381                 ; $808BEC
    ldy.w #$1310                ; $808BEF
    ldx.w #$8000                ; $808BF2
    lda.w #$0088                ; $808BF5
    jsr.w $8381                 ; $808BF8
    ldy.w #$2000                ; $808BFB
    ldx.w #$8190                ; $808BFE
    lda.w #$0087                ; $808C01
    jsr.w $8381                 ; $808C04
    ldy.w #$2500                ; $808C07
    ldx.w #$895F                ; $808C0A
    lda.w #$008D                ; $808C0D
    jsr.w $8381                 ; $808C10
    stz.w $2116                 ; $808C13
    ldx.w #$0010                ; $808C16
CODE_808C19:
    stz.w $2118                 ; $808C19
    dex                         ; $808C1C
    bne CODE_808C19             ; $808C1D
    jsr.w $B7E7                 ; $808C1F
    ldx.b $92                   ; $808C22
    lda.w $8B35,x               ; $808C24
    sta.b $B8                   ; $808C27
    lda.w $8B38,x               ; $808C29
    clc                         ; $808C2C
    adc.w #$0004                ; $808C2D
    sta.b $B4                   ; $808C30
    lda.w $8B3A,x               ; $808C32
    sta.b $5C                   ; $808C35
    sep.b #$20                  ; $808C37
    lda.w $8B37,x               ; $808C39
    sta.b $AD                   ; $808C3C
    rep.b #$20                  ; $808C3E
    lda.w $8B3C,x               ; $808C40
    sta.b $60                   ; $808C43
    lda.w $8B3E,x               ; $808C45
    sta.b $62                   ; $808C48
    phb                         ; $808C4A
    ldx.w #$8000                ; $808C4B
    ldy.b $C4                   ; $808C4E
    lda.w #$003F                ; $808C50
    mvn $81,$9C                 ; $808C53
    ldx.w #$9ECA                ; $808C56
    lda.w #$003F                ; $808C59
    mvn $81,$9D                 ; $808C5C
    lda.b $C4                   ; $808C5F
    clc                         ; $808C61
    adc.w #$0180                ; $808C62
    tay                         ; $808C65
    lda.w #$001F                ; $808C66
    ldx.w #$8000                ; $808C69
    mvn $81,$94                 ; $808C6C
    plb                         ; $808C6F
    ldy.b $C4                   ; $808C70
    lda.w #$0C70                ; $808C72
    sta.w $0122,y               ; $808C75
    lda.w #$0C69                ; $808C78
    sta.w $0124,y               ; $808C7B
    lda.w #$109F                ; $808C7E
    sta.w $0142,y               ; $808C81
    lda.w #$108C                ; $808C84
    sta.w $0144,y               ; $808C87
    lda.w #$13FF                ; $808C8A
    sta.w $01C2,y               ; $808C8D
    lda.w #$1210                ; $808C90
    sta.w $01C4,y               ; $808C93
    lda.w #$7FFF                ; $808C96
    sta.w $01A2,y               ; $808C99
    lda.w #$1210                ; $808C9C
    sta.w $01A4,y               ; $808C9F
    lda.w #$7F9C                ; $808CA2
    sta.w $01E2,y               ; $808CA5
    lda.w #$4210                ; $808CA8
    sta.w $01E4,y               ; $808CAB
    ldy.b $5C                   ; $808CAE
    ldx.w #$8000                ; $808CB0
    lda.w #$0087                ; $808CB3
    jsr.w $B95D                 ; $808CB6
    lda.w #$554D                ; $808CB9
    sta.b $AE                   ; $808CBC
    lda.w #$0F00                ; $808CBE
    sta.b $B0                   ; $808CC1
    lda.w #$000A                ; $808CC3
    sta.b $BA                   ; $808CC6
    sep.b #$20                  ; $808CC8
    lda.b #$08                  ; $808CCA
    sta.b $A9                   ; $808CCC
    rep.b #$20                  ; $808CCE
    lda.w #$1011                ; $808CD0
    sta.b $BE                   ; $808CD3
    stz.b $B2                   ; $808CD5
    lda.w #$1013                ; $808CD7
    sta.b $BC                   ; $808CDA
    lda.w #$0002                ; $808CDC
    sta.b $C0                   ; $808CDF
CODE_808CE1:
    lda.b $64                   ; $808CE1
    bne CODE_808CE1             ; $808CE3
    lda.w #$0001                ; $808CE5
    jsr.w $8373                 ; $808CE8
CODE_808CEB:
    lda.b $64                   ; $808CEB
    bne CODE_808CEB             ; $808CED
    ldx.b $5E                   ; $808CEF
    ldy.w #$01E0                ; $808CF1
    lda.w #$0000                ; $808CF4
CODE_808CF7:
    sta.l $7F0000,x             ; $808CF7
    inx                         ; $808CFB
    inx                         ; $808CFC
    dey                         ; $808CFD
    bne CODE_808CF7             ; $808CFE
    lda.w #$0004                ; $808D00
    sta.b $9C                   ; $808D03
    lda.w #$0005                ; $808D05
    sta.b $9E                   ; $808D08
    lda.w #$0001                ; $808D0A
    jsr.w $8373                 ; $808D0D
    lda.b $76                   ; $808D10
    bit.w #$0020                ; $808D12
    beq CODE_808D1A             ; $808D15
    jmp.w $8EA3                 ; $808D17
CODE_808D1A:
    lda.w #$0F00                ; $808D1A
    sta.b $98                   ; $808D1D
    lda.w #$0100                ; $808D1F
    sta.b $96                   ; $808D22
    lda.w #$0001                ; $808D24
    sta.l $8103E0               ; $808D27
    lda.w #$D60E                ; $808D2B
    sta.b $50                   ; $808D2E
    ldx.w $0444                 ; $808D30
    lda.w $03DC                 ; $808D33
    and.w #$0300                ; $808D36
    db $F0,$0E ; beq            ; $808D39
    cmp.w #$0100                ; $808D3B
    beq CODE_808D52             ; $808D3E
    incbin !baserom:$D40..$D52
CODE_808D52:
    lda.l $85D530,x             ; $808D52
    ldy.w #$001C                ; $808D56
    and.w #$00FF                ; $808D59
    beq CODE_808D6A             ; $808D5C
    tya                         ; $808D5E
    clc                         ; $808D5F
    adc.w $036C                 ; $808D60
    tax                         ; $808D63
    lda.l $85D58E,x             ; $808D64
    sta.b $50                   ; $808D68
CODE_808D6A:
    lda.b $5E                   ; $808D6A
    clc                         ; $808D6C
    adc.w #$0118                ; $808D6D
    tay                         ; $808D70
    ldx.b $50                   ; $808D71
    lda.w #$0085                ; $808D73
    jsr.w $B556                 ; $808D76
    stz.b $48                   ; $808D79
    lda.w #$0000                ; $808D7B
    sta.b $4A                   ; $808D7E
    stz.b $4C                   ; $808D80
    stz.b $4E                   ; $808D82
    stz.b $EE                   ; $808D84
    lda.w #$007F                ; $808D86
    sta.b $14                   ; $808D89
    ldx.b $5E                   ; $808D8B
    lda.b $5C                   ; $808D8D
    clc                         ; $808D8F
    adc.w #$0400                ; $808D90
    tay                         ; $808D93
    lda.w #$03C0                ; $808D94
    jsr.w $B537                 ; $808D97
    lda.w #$0001                ; $808D9A
    jsr.w $8373                 ; $808D9D
    ldx.w #$0000                ; $808DA0
    lda.b $4A                   ; $808DA3
    clc                         ; $808DA5
    adc.w #$2830                ; $808DA6
    sta.b $22                   ; $808DA9
    lda.b $48                   ; $808DAB
    sta.b $20                   ; $808DAD
    beq CODE_808DBD             ; $808DAF
CODE_808DB1:
    lda.w #$2200                ; $808DB1
    sta.b $14                   ; $808DB4
    jsr.w $B2F2                 ; $808DB6
    dec.b $20                   ; $808DB9
    bne CODE_808DB1             ; $808DBB
CODE_808DBD:
    lda.w #$2400                ; $808DBD
    sta.b $14                   ; $808DC0
    jsr.w $B2F2                 ; $808DC2
    cpx.w #$0020                ; $808DC5
    beq CODE_808DD7             ; $808DC8
CODE_808DCA:
    lda.w #$2200                ; $808DCA
    sta.b $14                   ; $808DCD
    jsr.w $B2F2                 ; $808DCF
    cpx.w #$0020                ; $808DD2
    bne CODE_808DCA             ; $808DD5
CODE_808DD7:
    lda.b $48                   ; $808DD7
    asl a                       ; $808DD9
    asl a                       ; $808DDA
    asl a                       ; $808DDB
    xba                         ; $808DDC
    clc                         ; $808DDD
    adc.b $4A                   ; $808DDE
    bpl CODE_808DEC             ; $808DE0
    incbin !baserom:$DE2..$DEC
CODE_808DEC:
    cmp.w #$1800                ; $808DEC
    beq CODE_808DFB             ; $808DEF
    bmi CODE_808DFB             ; $808DF1
    lda.b $4A                   ; $808DF3
    sec                         ; $808DF5
    sbc.w #$0100                ; $808DF6
    sta.b $4A                   ; $808DF9
CODE_808DFB:
    jsr.w $9DD0                 ; $808DFB
    jsr.w $8FFC                 ; $808DFE
    jsr.w $89D6                 ; $808E01
    lda.w #$C1C0                ; $808E04
    trb.b $0E                   ; $808E07
    beq CODE_808E2A             ; $808E09
    lda.b $4A                   ; $808E0B
    pha                         ; $808E0D
    jsr.w $8EB4                 ; $808E0E
    pla                         ; $808E11
    sta.b $4A                   ; $808E12
    ldx.b $50                   ; $808E14
    lda.w #$0085                ; $808E16
    jsr.w $B58D                 ; $808E19
    ldy.b $C4                   ; $808E1C
    lda.w #$0C70                ; $808E1E
    sta.w $0122,y               ; $808E21
    lda.w #$0C69                ; $808E24
    sta.w $0124,y               ; $808E27
CODE_808E2A:
    lda.w #$0800                ; $808E2A
    trb.b $0E                   ; $808E2D
    beq CODE_808E39             ; $808E2F
    incbin !baserom:$E31..$E39
CODE_808E39:
    lda.w #$0400                ; $808E39
    trb.b $0E                   ; $808E3C
    beq CODE_808E48             ; $808E3E
    lda.w #$0018                ; $808E40
    sta.b $EA                   ; $808E43
    jsr.w $B345                 ; $808E45
CODE_808E48:
    lda.w #$0C00                ; $808E48
    bit.b $0C                   ; $808E4B
    beq CODE_808E6C             ; $808E4D
    dec.b $EA                   ; $808E4F
    bne CODE_808E6C             ; $808E51
    incbin !baserom:$E53..$E6C
CODE_808E6C:
    ldy.b $C4                   ; $808E6C
    lda.b $4E                   ; $808E6E
    asl a                       ; $808E70
    and.w #$003E                ; $808E71
    tax                         ; $808E74
    lda.l $80B400,x             ; $808E75
    sta.w $0142,y               ; $808E79
    inc.b $4E                   ; $808E7C
    lda.b $8C                   ; $808E7E
    cmp.b $8E                   ; $808E80
    bne CODE_808E87             ; $808E82
    jsr.w $B59D                 ; $808E84
CODE_808E87:
    lda.b $9A                   ; $808E87
    and.w #$7FFF                ; $808E89
    beq CODE_808E91             ; $808E8C
    jmp.w $8D86                 ; $808E8E
CODE_808E91:
    ldx.b $94                   ; $808E91
    ldy.w #$000A                ; $808E93
CODE_808E96:
    lda.w $0393,x               ; $808E96
    and.w #$FF7F                ; $808E99
    sta.w $0393,x               ; $808E9C
    inx                         ; $808E9F
    dey                         ; $808EA0
    bne CODE_808E96             ; $808EA1
    jsr.w $B950                 ; $808EA3
    lda.w #$8934                ; $808EA6
    sta.w $0008                 ; $808EA9
    lda.w #$89B2                ; $808EAC
    sta.w $0005                 ; $808EAF
    sei                         ; $808EB2
    rts                         ; $808EB3
    lda.b $48                   ; $808EB4
    cmp.b $F0                   ; $808EB6
    db $D0,$0C ; bne            ; $808EB8
    jsl.l $80E4DB               ; $808EBA
    lda.w #$FF80                ; $808EBE
    sta.b $96                   ; $808EC1
    stz.b $98                   ; $808EC3
    rts                         ; $808EC5
    incbin !baserom:$EC6..$FFC
    ldx.b $74                   ; $808FFC
    lda.b $EE                   ; $808FFE
    beq CODE_809004             ; $809000
    incbin !baserom:$1002..$1004
CODE_809004:
    lda.w #$3892                ; $809004
    sta.w $0902,x               ; $809007
    lda.w #$085A                ; $80900A
    clc                         ; $80900D
    adc.b $EE                   ; $80900E
    clc                         ; $809010
    adc.b $E7                   ; $809011
    sta.w $0900,x               ; $809013
    lda.w #$0100                ; $809016
    sta.w $0B01,x               ; $809019
    lda.w #$3890                ; $80901C
    sta.w $0906,x               ; $80901F
    lda.w #$0A5C                ; $809022
    clc                         ; $809025
    adc.b $E7                   ; $809026
    sta.w $0904,x               ; $809028
    lda.w #$0100                ; $80902B
    sta.w $0B05,x               ; $80902E
    txa                         ; $809031
    clc                         ; $809032
    adc.w #$0008                ; $809033
    sta.b $74                   ; $809036
    rts                         ; $809038
    incbin !baserom:$1039..$1DD0
    lda.b $76                   ; $809DD0
    bit.w #$0002                ; $809DD2
    beq CODE_809E0B             ; $809DD5
    incbin !baserom:$1DD7..$1E0B
CODE_809E0B:
    lda.w #$3C00                ; $809E0B
    sta.b $14                   ; $809E0E
    ldx.w #$9E3A                ; $809E10
    lda.w #$5C5C                ; $809E13
    clc                         ; $809E16
    adc.b $E7                   ; $809E17
    tay                         ; $809E19
    lda.w #$0080                ; $809E1A
    jsr.w $841A                 ; $809E1D
    ldx.b $94                   ; $809E20
    lda.w #$3C00                ; $809E22
    sta.b $14                   ; $809E25
    lda.w #$5C94                ; $809E27
    clc                         ; $809E2A
    adc.b $E7                   ; $809E2B
    tay                         ; $809E2D
    lda.w $0390,x               ; $809E2E
    pha                         ; $809E31
    lda.w $038E,x               ; $809E32
    plx                         ; $809E35
    jsr.w $84F0                 ; $809E36
    rts                         ; $809E39
    incbin !baserom:$1E3A..$32F2
    lda.b $22                   ; $80B2F2
    cmp.w #$1000                ; $80B2F4
    bmi CODE_80B311             ; $80B2F7
    cmp.w #$5800                ; $80B2F9
    bpl CODE_80B311             ; $80B2FC
    phx                         ; $80B2FE
    lda.l $80B371,x             ; $80B2FF
    tax                         ; $80B303
    lda.b $22                   ; $80B304
    clc                         ; $80B306
    adc.b $E7                   ; $80B307
    tay                         ; $80B309
    lda.w #$0080                ; $80B30A
    jsr.w $84BE                 ; $80B30D
    plx                         ; $80B310
CODE_80B311:
    lda.b $22                   ; $80B311
    clc                         ; $80B313
    adc.w #$0800                ; $80B314
    sta.b $22                   ; $80B317
    inx                         ; $80B319
    inx                         ; $80B31A
    cpx.w #$0020                ; $80B31B
    beq CODE_80B332             ; $80B31E
    txa                         ; $80B320
    lsr a                       ; $80B321
    tay                         ; $80B322
    lda.w $036E,y               ; $80B323
    and.w #$00FF                ; $80B326
    bne CODE_80B332             ; $80B329
    txa                         ; $80B32B
    lsr a                       ; $80B32C
    sta.b $F0                   ; $80B32D
    ldx.w #$001E                ; $80B32F
CODE_80B332:
    rts                         ; $80B332
    incbin !baserom:$3333..$3345
    jsr.w $B359                 ; $80B345
    bcs CODE_80B358             ; $80B348
    lda.b $48                   ; $80B34A
    cmp.b $F0                   ; $80B34C
    beq CODE_80B358             ; $80B34E
    inc.b $48                   ; $80B350
    stz.b $4E                   ; $80B352
    jsl.l $80E4C5               ; $80B354
CODE_80B358:
    rts                         ; $80B358
    lda.b $48                   ; $80B359
    asl a                       ; $80B35B
    asl a                       ; $80B35C
    asl a                       ; $80B35D
    xba                         ; $80B35E
    clc                         ; $80B35F
    adc.b $4A                   ; $80B360
    bpl CODE_80B366             ; $80B362
    incbin !baserom:$3364..$3366
CODE_80B366:
    cmp.w #$1800                ; $80B366
    beq CODE_80B36F             ; $80B369
    bmi CODE_80B36F             ; $80B36B
    incbin !baserom:$336D..$336F
CODE_80B36F:
    clc                         ; $80B36F
    rts                         ; $80B370
    incbin !baserom:$3371..$349E
    txy                         ; $80B49E
    ldx.b $74                   ; $80B49F
    phb                         ; $80B4A1
    sep.b #$20                  ; $80B4A2
    pha                         ; $80B4A4
    plb                         ; $80B4A5
    rep.b #$20                  ; $80B4A6
    lda.w $0000,y               ; $80B4A8
    beq CODE_80B4ED             ; $80B4AB
CODE_80B4AD:
    clc                         ; $80B4AD
    adc.b $18                   ; $80B4AE
    sta.w $0902,x               ; $80B4B0
    lda.w $0002,y               ; $80B4B3
    clc                         ; $80B4B6
    adc.b $16                   ; $80B4B7
    and.w #$00FF                ; $80B4B9
    xba                         ; $80B4BC
    sta.b $1A                   ; $80B4BD
    lda.w $0004,y               ; $80B4BF
    clc                         ; $80B4C2
    adc.b $14                   ; $80B4C3
    bit.w #$FF00                ; $80B4C5
    beq CODE_80B4CD             ; $80B4C8
    ora.w #$FF00                ; $80B4CA
CODE_80B4CD:
    sta.w $0B00,x               ; $80B4CD
    and.w #$00FF                ; $80B4D0
    ora.b $1A                   ; $80B4D3
    sta.w $0900,x               ; $80B4D5
    lda.w #$0001                ; $80B4D8
    sta.w $0B02,x               ; $80B4DB
    inx                         ; $80B4DE
    inx                         ; $80B4DF
    inx                         ; $80B4E0
    inx                         ; $80B4E1
    tya                         ; $80B4E2
    clc                         ; $80B4E3
    adc.w #$0006                ; $80B4E4
    tay                         ; $80B4E7
    lda.w $0000,y               ; $80B4E8
    bne CODE_80B4AD             ; $80B4EB
CODE_80B4ED:
    iny                         ; $80B4ED
    iny                         ; $80B4EE
    lda.w $0000,y               ; $80B4EF
    beq CODE_80B531             ; $80B4F2
CODE_80B4F4:
    clc                         ; $80B4F4
    adc.b $18                   ; $80B4F5
    sta.w $0902,x               ; $80B4F7
    lda.w $0002,y               ; $80B4FA
    clc                         ; $80B4FD
    adc.b $16                   ; $80B4FE
    and.w #$00FF                ; $80B500
    xba                         ; $80B503
    sta.b $1A                   ; $80B504
    lda.w $0004,y               ; $80B506
    clc                         ; $80B509
    adc.b $14                   ; $80B50A
    bit.w #$FF00                ; $80B50C
    beq CODE_80B514             ; $80B50F
    ora.w #$FF00                ; $80B511
CODE_80B514:
    sta.w $0B00,x               ; $80B514
    and.w #$00FF                ; $80B517
    ora.b $1A                   ; $80B51A
    sta.w $0900,x               ; $80B51C
    stz.w $0B02,x               ; $80B51F
    inx                         ; $80B522
    inx                         ; $80B523
    inx                         ; $80B524
    inx                         ; $80B525
    tya                         ; $80B526
    clc                         ; $80B527
    adc.w #$0006                ; $80B528
    tay                         ; $80B52B
    lda.w $0000,y               ; $80B52C
    bne CODE_80B4F4             ; $80B52F
CODE_80B531:
    stx.b $74                   ; $80B531
    plb                         ; $80B533
    rts                         ; $80B534
    incbin !baserom:$3535..$3537
    phx                         ; $80B537
    ldx.b $8C                   ; $80B538
    sta.w $1900,x               ; $80B53A
    tya                         ; $80B53D
    sta.w $19C0,x               ; $80B53E
    pla                         ; $80B541
    sta.w $1940,x               ; $80B542
    lda.b $14                   ; $80B545
    sta.w $1980,x               ; $80B547
    inx                         ; $80B54A
    inx                         ; $80B54B
    txa                         ; $80B54C
    and.w #$003F                ; $80B54D
    clc                         ; $80B550
    adc.b $8A                   ; $80B551
    sta.b $8C                   ; $80B553
    rts                         ; $80B555
    stx.b $CC                   ; $80B556
    sep.b #$20                  ; $80B558
    sta.b $CE                   ; $80B55A
    rep.b #$20                  ; $80B55C
    sty.b $C8                   ; $80B55E
    stz.b $CA                   ; $80B560
    lda.w #$FFFF                ; $80B562
    sta.b $CF                   ; $80B565
    sta.b $D1                   ; $80B567
    sta.b $D3                   ; $80B569
    sta.b $D5                   ; $80B56B
    sta.b $D7                   ; $80B56D
    sta.b $D9                   ; $80B56F
    sta.b $DB                   ; $80B571
    sta.b $DD                   ; $80B573
    stz.b $C6                   ; $80B575
    stz.b $E3                   ; $80B577
    lda.w #$0100                ; $80B579
    sta.b $E5                   ; $80B57C
    lda.w #$0009                ; $80B57E
    sta.b $E1                   ; $80B581
    stz.b $F6                   ; $80B583
    stz.b $F8                   ; $80B585
    lda.w #$0019                ; $80B587
    sta.b $F9                   ; $80B58A
    rts                         ; $80B58C
    stx.b $CC                   ; $80B58D
    sep.b #$20                  ; $80B58F
    sta.b $CE                   ; $80B591
    rep.b #$20                  ; $80B593
    stz.b $E3                   ; $80B595
    lda.w #$0009                ; $80B597
    sta.b $E1                   ; $80B59A
    rts                         ; $80B59C
    lda.b $C6                   ; $80B59D
    clc                         ; $80B59F
    adc.b $E5                   ; $80B5A0
    sta.b $C6                   ; $80B5A2
    lda.b $C6                   ; $80B5A4
    sec                         ; $80B5A6
    sbc.w #$0100                ; $80B5A7
    bcs CODE_80B5AD             ; $80B5AA
    rts                         ; $80B5AC
CODE_80B5AD:
    sta.b $C6                   ; $80B5AD
    ldx.b $CF                   ; $80B5AF
    cpx.w #$FFFF                ; $80B5B1
    beq CODE_80B5C2             ; $80B5B4
    lda.l $7F0000,x             ; $80B5B6
    clc                         ; $80B5BA
    adc.w #$004C                ; $80B5BB
    sta.l $7F0000,x             ; $80B5BE
CODE_80B5C2:
    ldy.w #$0002                ; $80B5C2
CODE_80B5C5:
    ldx.b $CF,y                 ; $80B5C5
    stx.b $CD,y                 ; $80B5C7
    cpx.w #$FFFF                ; $80B5C9
    beq CODE_80B5DA             ; $80B5CC
    lda.l $7F0000,x             ; $80B5CE
    clc                         ; $80B5D2
    adc.w #$004C                ; $80B5D3
    sta.l $7F0000,x             ; $80B5D6
CODE_80B5DA:
    iny                         ; $80B5DA
    iny                         ; $80B5DB
    cpy.w #$0006                ; $80B5DC
    bne CODE_80B5C5             ; $80B5DF
    ldx.b $CF,y                 ; $80B5E1
    stx.b $CD,y                 ; $80B5E3
    cpx.w #$FFFF                ; $80B5E5
    beq CODE_80B5F6             ; $80B5E8
    lda.l $7F0000,x             ; $80B5EA
    clc                         ; $80B5EE
    adc.w #$031C                ; $80B5EF
    sta.l $7F0000,x             ; $80B5F2
CODE_80B5F6:
    iny                         ; $80B5F6
    iny                         ; $80B5F7
CODE_80B5F8:
    ldx.b $CF,y                 ; $80B5F8
    stx.b $CD,y                 ; $80B5FA
    cpx.w #$FFFF                ; $80B5FC
    beq CODE_80B60D             ; $80B5FF
    lda.l $7F0000,x             ; $80B601
    clc                         ; $80B605
    adc.w #$004C                ; $80B606
    sta.l $7F0000,x             ; $80B609
CODE_80B60D:
    iny                         ; $80B60D
    iny                         ; $80B60E
    cpy.w #$000E                ; $80B60F
    bne CODE_80B5F8             ; $80B612
    ldx.b $DD                   ; $80B614
    stx.b $DB                   ; $80B616
    cpx.w #$FFFF                ; $80B618
    beq CODE_80B62A             ; $80B61B
    lda.b $DF                   ; $80B61D
    sta.l $7F0000,x             ; $80B61F
    bne CODE_80B62A             ; $80B623
    lda.w #$FFFF                ; $80B625
    sta.b $DB                   ; $80B628
CODE_80B62A:
    sep.b #$20                  ; $80B62A
    lda.b [$CC]                 ; $80B62C
    bpl CODE_80B645             ; $80B62E
    asl a                       ; $80B630
    rep.b #$20                  ; $80B631
    and.w #$00FF                ; $80B633
    tax                         ; $80B636
    jmp.w ($B6BC,x)             ; $80B637
    clc                         ; $80B63A
    adc.b $CC                   ; $80B63B
    sta.b $CC                   ; $80B63D
    bcc CODE_80B643             ; $80B63F
    incbin !baserom:$3641..$3643
CODE_80B643:
    bra CODE_80B62A             ; $80B643
CODE_80B645:
    cmp.b #$20                  ; $80B645
    bne CODE_80B65E             ; $80B647
    rep.b #$20                  ; $80B649
    lda.b $C8                   ; $80B64B
    clc                         ; $80B64D
    adc.b $CA                   ; $80B64E
    tax                         ; $80B650
    stz.b $DF                   ; $80B651
    lda.w #$084C                ; $80B653
    sta.l $7F0000,x             ; $80B656
    stx.b $DD                   ; $80B65A
    bra CODE_80B6A1             ; $80B65C
CODE_80B65E:
    ldx.w #$003F                ; $80B65E
CODE_80B661:
    cmp.l $80B79A,x             ; $80B661
    beq CODE_80B689             ; $80B665
    inx                         ; $80B667
    cpx.w #$004C                ; $80B668
    bne CODE_80B661             ; $80B66B
    cmp.b #$41                  ; $80B66D
    bcs CODE_80B676             ; $80B66F
    incbin !baserom:$3671..$3676
CODE_80B676:
    cmp.b #$61                  ; $80B676
    bcs CODE_80B67F             ; $80B678
    sbc.b #$25                  ; $80B67A
    jmp.w $B681                 ; $80B67C
CODE_80B67F:
    sbc.b #$60                  ; $80B67F
    rep.b #$20                  ; $80B681
    and.w #$00FF                ; $80B683
    tay                         ; $80B686
    bra CODE_80B68C             ; $80B687
CODE_80B689:
    rep.b #$20                  ; $80B689
    txy                         ; $80B68B
CODE_80B68C:
    lda.b $C8                   ; $80B68C
    clc                         ; $80B68E
    adc.b $CA                   ; $80B68F
    tax                         ; $80B691
    tya                         ; $80B692
    ora.w #$2800                ; $80B693
    sta.b $DF                   ; $80B696
    lda.w #$284C                ; $80B698
    sta.l $7F0000,x             ; $80B69B
    stx.b $DD                   ; $80B69F
CODE_80B6A1:
    lda.b $CC                   ; $80B6A1
    clc                         ; $80B6A3
    adc.w #$0001                ; $80B6A4
    sta.b $CC                   ; $80B6A7
    bcc CODE_80B6AD             ; $80B6A9
    incbin !baserom:$36AB..$36AD
CODE_80B6AD:
    inc.b $CA                   ; $80B6AD
    inc.b $CA                   ; $80B6AF
    jsr.w $B7BB                 ; $80B6B1
    lda.w #$0009                ; $80B6B4
    sta.b $E1                   ; $80B6B7
    jmp.w $B5A4                 ; $80B6B9
    incbin !baserom:$36BC..$36CA
    lda.b $CA                   ; $80B6CA
    and.w #$FFC0                ; $80B6CC
    clc                         ; $80B6CF
    adc.w #$0040                ; $80B6D0
    sta.b $CA                   ; $80B6D3
    inc.b $F6                   ; $80B6D5
    lda.w #$0001                ; $80B6D7
    jmp.w $B63A                 ; $80B6DA
    incbin !baserom:$36DD..$36F2
    dec.b $E1                   ; $80B6F2
    bne CODE_80B709             ; $80B6F4
    stz.b $DF                   ; $80B6F6
    lda.b $C8                   ; $80B6F8
    clc                         ; $80B6FA
    adc.b $CA                   ; $80B6FB
    sta.b $DD                   ; $80B6FD
    lda.w #$0010                ; $80B6FF
    sta.b $E1                   ; $80B702
    jsr.w $B7BB                 ; $80B704
    bra CODE_80B727             ; $80B707
CODE_80B709:
    lda.b $E1                   ; $80B709
    cmp.w #$0008                ; $80B70B
    bne CODE_80B722             ; $80B70E
    lda.b $C8                   ; $80B710
    clc                         ; $80B712
    adc.b $CA                   ; $80B713
    tax                         ; $80B715
    lda.w #$284C                ; $80B716
    sta.b $DF                   ; $80B719
    stx.b $DD                   ; $80B71B
    jsr.w $B7BB                 ; $80B71D
    bra CODE_80B727             ; $80B720
CODE_80B722:
    lda.w #$FFFF                ; $80B722
    sta.b $DD                   ; $80B725
CODE_80B727:
    jmp.w $B5A4                 ; $80B727
    lda.b $E3                   ; $80B72A
    bne CODE_80B735             ; $80B72C
    ldy.w #$0001                ; $80B72E
    lda.b [$CC],y               ; $80B731
    sta.b $E3                   ; $80B733
CODE_80B735:
    dec.b $E3                   ; $80B735
    bne CODE_80B73F             ; $80B737
    lda.w #$0003                ; $80B739
    jmp.w $B63A                 ; $80B73C
CODE_80B73F:
    dec.b $E1                   ; $80B73F
    bne CODE_80B756             ; $80B741
    stz.b $DF                   ; $80B743
    lda.b $C8                   ; $80B745
    clc                         ; $80B747
    adc.b $CA                   ; $80B748
    sta.b $DD                   ; $80B74A
    lda.w #$0010                ; $80B74C
    sta.b $E1                   ; $80B74F
    jsr.w $B7BB                 ; $80B751
    bra CODE_80B774             ; $80B754
CODE_80B756:
    lda.b $E1                   ; $80B756
    cmp.w #$0008                ; $80B758
    bne CODE_80B76F             ; $80B75B
    lda.b $C8                   ; $80B75D
    clc                         ; $80B75F
    adc.b $CA                   ; $80B760
    tax                         ; $80B762
    lda.w #$284C                ; $80B763
    sta.b $DF                   ; $80B766
    stx.b $DD                   ; $80B768
    jsr.w $B7BB                 ; $80B76A
    bra CODE_80B774             ; $80B76D
CODE_80B76F:
    lda.w #$FFFF                ; $80B76F
    sta.b $DD                   ; $80B772
CODE_80B774:
    jmp.w $B5A4                 ; $80B774
    lda.w #$FFFF                ; $80B777
    sta.b $CF                   ; $80B77A
    sta.b $D1                   ; $80B77C
    sta.b $D3                   ; $80B77E
    sta.b $D5                   ; $80B780
    sta.b $D7                   ; $80B782
    sta.b $D9                   ; $80B784
    sta.b $DB                   ; $80B786
    sta.b $DD                   ; $80B788
    lda.b $CA                   ; $80B78A
    and.w #$FFC0                ; $80B78C
    clc                         ; $80B78F
    adc.b $C8                   ; $80B790
    tax                         ; $80B792
    ldy.b $F9                   ; $80B793
    lda.w #$0000                ; $80B795
CODE_80B798:
    sta.l $7F0000,x             ; $80B798
    inx                         ; $80B79C
    inx                         ; $80B79D
    dey                         ; $80B79E
    bne CODE_80B798             ; $80B79F
    lda.b $CA                   ; $80B7A1
    and.w #$FFC0                ; $80B7A3
    sta.b $CA                   ; $80B7A6
    beq CODE_80B7B5             ; $80B7A8
    sec                         ; $80B7AA
    sbc.w #$0040                ; $80B7AB
    sta.b $CA                   ; $80B7AE
    dec.b $F6                   ; $80B7B0
    jmp.w $B5A4                 ; $80B7B2
CODE_80B7B5:
    lda.w #$0001                ; $80B7B5
    jmp.w $B63A                 ; $80B7B8
    lda.b $DD                   ; $80B7BB
    ldy.w #$FFFF                ; $80B7BD
    ldx.w #$000E                ; $80B7C0
CODE_80B7C3:
    cmp.b $CD,x                 ; $80B7C3
    bne CODE_80B7C9             ; $80B7C5
    sty.b $CD,x                 ; $80B7C7
CODE_80B7C9:
    dex                         ; $80B7C9
    dex                         ; $80B7CA
    bne CODE_80B7C3             ; $80B7CB
    rts                         ; $80B7CD
    lda.w #$0001                ; $80B7CE
    tsb.b $F8                   ; $80B7D1
    lda.w #$0001                ; $80B7D3
    jmp.w $B63A                 ; $80B7D6
    incbin !baserom:$37D9..$37E7
    phb                         ; $80B7E7
    ldx.w #$A2EF                ; $80B7E8
    ldy.w #$0196                ; $80B7EB
    lda.w #$002B                ; $80B7EE
    mvn $81,$84                 ; $80B7F1
    ldx.w #$A2EF                ; $80B7F4
    ldy.w #$0296                ; $80B7F7
    lda.w #$002B                ; $80B7FA
    mvn $81,$84                 ; $80B7FD
    plb                         ; $80B800
    stz.w $01C2                 ; $80B801
    stz.w $02C2                 ; $80B804
    stz.w $0164                 ; $80B807
    stz.w $0264                 ; $80B80A
    stz.w $0196                 ; $80B80D
    stz.w $0296                 ; $80B810
    stz.w $019A                 ; $80B813
    stz.w $029A                 ; $80B816
    stz.w $0176                 ; $80B819
    stz.w $0276                 ; $80B81C
    lda.l $8103DC               ; $80B81F
    bit.w #$000F                ; $80B823
    beq CODE_80B830             ; $80B826
    incbin !baserom:$3828..$3830
CODE_80B830:
    lda.w #$0020                ; $80B830
    tsb.w $0276                 ; $80B833
    lda.w #$0500                ; $80B836
    sta.w $01C4                 ; $80B839
    lda.w #$0700                ; $80B83C
    sta.w $02C4                 ; $80B83F
    stz.w $015C                 ; $80B842
    lda.w #$4000                ; $80B845
    sta.w $025C                 ; $80B848
    stz.w $015E                 ; $80B84B
    lda.w #$8000                ; $80B84E
    sta.w $025E                 ; $80B851
    stz.w $0170                 ; $80B854
    lda.w #$0400                ; $80B857
    sta.w $0172                 ; $80B85A
    sta.w $0174                 ; $80B85D
    lda.w #$0900                ; $80B860
    sta.w $016E                 ; $80B863
    lda.w #$0D00                ; $80B866
    sta.w $016C                 ; $80B869
    lda.w #$0800                ; $80B86C
    sta.w $0270                 ; $80B86F
    lda.w #$0C00                ; $80B872
    sta.w $0272                 ; $80B875
    sta.w $0274                 ; $80B878
    lda.w #$1100                ; $80B87B
    sta.w $026E                 ; $80B87E
    lda.w #$1500                ; $80B881
    sta.w $026C                 ; $80B884
    stz.w $01E7                 ; $80B887
    stz.w $01E8                 ; $80B88A
    lda.w #$7400                ; $80B88D
    sta.w $02E7                 ; $80B890
    lda.w #$0074                ; $80B893
    sta.w $02E8                 ; $80B896
    stz.w $018A                 ; $80B899
    stz.w $018C                 ; $80B89C
    stz.w $018E                 ; $80B89F
    lda.w #$0100                ; $80B8A2
    sta.w $028A                 ; $80B8A5
    sta.w $028C                 ; $80B8A8
    sta.w $028E                 ; $80B8AB
    lda.w #$B94F                ; $80B8AE
    sta.w $01EC                 ; $80B8B1
    sta.w $02EC                 ; $80B8B4
    lda.w #$0000                ; $80B8B7
    sta.w $0192                 ; $80B8BA
    lda.w #$001B                ; $80B8BD
    sta.w $0292                 ; $80B8C0
    stz.w $0194                 ; $80B8C3
    lda.w #$002F                ; $80B8C6
    sta.w $0294                 ; $80B8C9
    ldx.w #$0200                ; $80B8CC
CODE_80B8CF:
    lda.w #$E0E0                ; $80B8CF
    sta.w $08FE,x               ; $80B8D2
    sta.w $0CFE,x               ; $80B8D5
    sta.w $10FE,x               ; $80B8D8
    sta.w $14FE,x               ; $80B8DB
    lda.w #$0000                ; $80B8DE
    sta.w $0AFE,x               ; $80B8E1
    sta.w $0EFE,x               ; $80B8E4
    sta.w $12FE,x               ; $80B8E7
    sta.w $16FE,x               ; $80B8EA
    dex                         ; $80B8ED
    dex                         ; $80B8EE
    bne CODE_80B8CF             ; $80B8EF
    lda.w #$0001                ; $80B8F1
    jsr.w $8373                 ; $80B8F4
    lda.w #$1D8C                ; $80B8F7
    sta.w $0278                 ; $80B8FA
    lda.w #$0200                ; $80B8FD
    sta.w $1D8D                 ; $80B900
    stz.w $1D8F                 ; $80B903
    stz.w $1D91                 ; $80B906
    stz.w $1D93                 ; $80B909
    sep.b #$20                  ; $80B90C
    phb                         ; $80B90E
    pla                         ; $80B90F
    sta.w $1D95                 ; $80B910
    stz.w $1D96                 ; $80B913
    rep.b #$20                  ; $80B916
    lda.w #$B94E                ; $80B918
    sta.w $1D97                 ; $80B91B
    sep.b #$20                  ; $80B91E
    lda.b #$80                  ; $80B920
    sta.w $1D99                 ; $80B922
    rep.b #$20                  ; $80B925
    pla                         ; $80B927
    pha                         ; $80B928
    sta.w $1D9A                 ; $80B929
    lda.w #$B97B                ; $80B92C
    sta.w $0008                 ; $80B92F
    lda.w #$006C                ; $80B932
    sta.w $4209                 ; $80B935
    lda.w #$0010                ; $80B938
    sta.w $4207                 ; $80B93B
    sep.b #$20                  ; $80B93E
    lda.b #$B1                  ; $80B940
    sta.w $4200                 ; $80B942
    rep.b #$20                  ; $80B945
    lda.w #$BC9B                ; $80B947
    sta.w $0005                 ; $80B94A
    cli                         ; $80B94D
    rts                         ; $80B94E
    rts                         ; $80B94F
    lda.w #$0010                ; $80B950
    tsb.b $76                   ; $80B953
    lda.w #$0008                ; $80B955
CODE_80B958:
    bit.b $76                   ; $80B958
    beq CODE_80B958             ; $80B95A
    rts                         ; $80B95C
    pha                         ; $80B95D
CODE_80B95E:
    lda.b $64                   ; $80B95E
    bne CODE_80B95E             ; $80B960
    pla                         ; $80B962
    ora.w #$7F00                ; $80B963
    phy                         ; $80B966
    ldy.b $5E                   ; $80B967
    jsr.w $FBA0                 ; $80B969
    ply                         ; $80B96C
    sty.b $6A                   ; $80B96D
    ldy.b $5E                   ; $80B96F
    sty.b $66                   ; $80B971
    ldy.w #$007F                ; $80B973
    sty.b $68                   ; $80B976
    sta.b $64                   ; $80B978
    rts                         ; $80B97A
    sep.b #$20                  ; $80B97B
    lda.b #$80                  ; $80B97D
    sta.w $2100                 ; $80B97F
    rep.b #$20                  ; $80B982
    lda.w #$0C00                ; $80B984
    sta.b $2C                   ; $80B987
    lda.w $0296                 ; $80B989
    beq CODE_80B9A9             ; $80B98C
    clc                         ; $80B98E
    adc.w $029A                 ; $80B98F
    sta.w $029A                 ; $80B992
    sec                         ; $80B995
    sbc.w $0298                 ; $80B996
    beq CODE_80B9A0             ; $80B999
    eor.w $0296                 ; $80B99B
    bmi CODE_80B9A9             ; $80B99E
CODE_80B9A0:
    lda.w $0298                 ; $80B9A0
    sta.w $029A                 ; $80B9A3
    stz.w $0296                 ; $80B9A6
CODE_80B9A9:
    lda.w $000C                 ; $80B9A9
    tax                         ; $80B9AC
    eor.w $010C                 ; $80B9AD
    stx.w $010C                 ; $80B9B0
    ora.w $010E                 ; $80B9B3
    and.w $010C                 ; $80B9B6
    sta.w $010E                 ; $80B9B9
    lda.w $0010                 ; $80B9BC
    tax                         ; $80B9BF
    eor.w $020C                 ; $80B9C0
    stx.w $020C                 ; $80B9C3
    ora.w $020E                 ; $80B9C6
    and.w $020C                 ; $80B9C9
    sta.w $020E                 ; $80B9CC
    lda.b $24                   ; $80B9CF
    and.w #$0001                ; $80B9D1
    sta.w $0290                 ; $80B9D4
    eor.w #$0001                ; $80B9D7
    sta.w $0190                 ; $80B9DA
    lda.b $24                   ; $80B9DD
    and.w #$0001                ; $80B9DF
    bne CODE_80B9E7             ; $80B9E2
    jmp.w $BA6C                 ; $80B9E4
CODE_80B9E7:
    ldx.w $028E                 ; $80B9E7
    cpx.w $028C                 ; $80B9EA
    beq CODE_80BA37             ; $80B9ED
    lda.w #$1801                ; $80B9EF
    sta.w $4300                 ; $80B9F2
CODE_80B9F5:
    lda.w $19C0,x               ; $80B9F5
    sta.w $2116                 ; $80B9F8
    lda.w $1940,x               ; $80B9FB
    sta.w $4302                 ; $80B9FE
    lda.w $1900,x               ; $80BA01
    cmp.b $2C                   ; $80BA04
    beq CODE_80BA0A             ; $80BA06
    db $B0,$30 ; bcs            ; $80BA08
CODE_80BA0A:
    sta.w $4305                 ; $80BA0A
    eor.w #$FFFF                ; $80BA0D
    sec                         ; $80BA10
    adc.b $2C                   ; $80BA11
    sta.b $2C                   ; $80BA13
    sep.b #$20                  ; $80BA15
    lda.w $1980,x               ; $80BA17
    sta.w $4304                 ; $80BA1A
    lda.b #$01                  ; $80BA1D
    sta.w $420B                 ; $80BA1F
    rep.b #$20                  ; $80BA22
    inx                         ; $80BA24
    inx                         ; $80BA25
    txa                         ; $80BA26
    and.w #$003F                ; $80BA27
    clc                         ; $80BA2A
    adc.w #$0100                ; $80BA2B
    sta.w $028E                 ; $80BA2E
    tax                         ; $80BA31
    cmp.w $028C                 ; $80BA32
    bne CODE_80B9F5             ; $80BA35
CODE_80BA37:
    jmp.w $BAE9                 ; $80BA37
    incbin !baserom:$3A3A..$3A6C
    ldx.w $018E                 ; $80BA6C
    cpx.w $018C                 ; $80BA6F
    beq CODE_80BAB8             ; $80BA72
    lda.w #$1801                ; $80BA74
    sta.w $4300                 ; $80BA77
CODE_80BA7A:
    lda.w $19C0,x               ; $80BA7A
    sta.w $2116                 ; $80BA7D
    lda.w $1940,x               ; $80BA80
    sta.w $4302                 ; $80BA83
    lda.w $1900,x               ; $80BA86
    cmp.b $2C                   ; $80BA89
    beq CODE_80BA8F             ; $80BA8B
    db $B0,$2C ; bcs            ; $80BA8D
CODE_80BA8F:
    sta.w $4305                 ; $80BA8F
    eor.w #$FFFF                ; $80BA92
    sec                         ; $80BA95
    adc.b $2C                   ; $80BA96
    sta.b $2C                   ; $80BA98
    sep.b #$20                  ; $80BA9A
    lda.w $1980,x               ; $80BA9C
    sta.w $4304                 ; $80BA9F
    lda.b #$01                  ; $80BAA2
    sta.w $420B                 ; $80BAA4
    rep.b #$20                  ; $80BAA7
    inx                         ; $80BAA9
    inx                         ; $80BAAA
    txa                         ; $80BAAB
    and.w #$003F                ; $80BAAC
    sta.w $018E                 ; $80BAAF
    tax                         ; $80BAB2
    cmp.w $018C                 ; $80BAB3
    bne CODE_80BA7A             ; $80BAB6
CODE_80BAB8:
    jmp.w $BAE9                 ; $80BAB8
    incbin !baserom:$3ABB..$3AE9
    lda.w #$0100                ; $80BAE9
    tcd                         ; $80BAEC
    sep.b #$20                  ; $80BAED
    lda.b $9C                   ; $80BAEF
    sta.w $210D                 ; $80BAF1
    lda.b $9D                   ; $80BAF4
    sta.w $210D                 ; $80BAF6
    lda.b $9E                   ; $80BAF9
    sec                         ; $80BAFB
    sbc.b $C2                   ; $80BAFC
    sta.w $210E                 ; $80BAFE
    lda.b $9F                   ; $80BB01
    sbc.b $C4                   ; $80BB03
    sta.w $210E                 ; $80BB05
    lda.b $A0                   ; $80BB08
    sta.w $210F                 ; $80BB0A
    lda.b $A1                   ; $80BB0D
    sta.w $210F                 ; $80BB0F
    lda.b $A2                   ; $80BB12
    sec                         ; $80BB14
    sbc.b $C2                   ; $80BB15
    sta.w $2110                 ; $80BB17
    lda.b $A3                   ; $80BB1A
    sbc.b $C4                   ; $80BB1C
    sta.w $2110                 ; $80BB1E
    lda.b $A4                   ; $80BB21
    sta.w $2111                 ; $80BB23
    lda.b $A5                   ; $80BB26
    sta.w $2111                 ; $80BB28
    lda.b $A6                   ; $80BB2B
    sec                         ; $80BB2D
    sbc.b $C2                   ; $80BB2E
    sta.w $2112                 ; $80BB30
    lda.b $A7                   ; $80BB33
    sbc.b $C4                   ; $80BB35
    sta.w $2112                 ; $80BB37
    lda.b $A8                   ; $80BB3A
    sta.w $2105                 ; $80BB3C
    lda.b $A9                   ; $80BB3F
    sta.w $2125                 ; $80BB41
    lda.b $AA                   ; $80BB44
    ora.b #$20                  ; $80BB46
    sta.w $2132                 ; $80BB48
    lda.b $AB                   ; $80BB4B
    ora.b #$40                  ; $80BB4D
    sta.w $2132                 ; $80BB4F
    lda.b $AC                   ; $80BB52
    ora.b #$80                  ; $80BB54
    sta.w $2132                 ; $80BB56
    lda.b $AD                   ; $80BB59
    sta.w $2101                 ; $80BB5B
    rep.b #$20                  ; $80BB5E
    lda.b $AE                   ; $80BB60
    sta.w $2126                 ; $80BB62
    lda.b $B0                   ; $80BB65
    sta.w $2128                 ; $80BB67
    lda.b $B2                   ; $80BB6A
    sta.w $212A                 ; $80BB6C
    lda.b $B4                   ; $80BB6F
    sta.w $2107                 ; $80BB71
    lda.b $B6                   ; $80BB74
    sta.w $2109                 ; $80BB76
    lda.b $B8                   ; $80BB79
    sta.w $210B                 ; $80BB7B
    lda.b $BA                   ; $80BB7E
    sta.w $2123                 ; $80BB80
    lda.b $BC                   ; $80BB83
    sta.w $212C                 ; $80BB85
    lda.b $BE                   ; $80BB88
    sta.w $212E                 ; $80BB8A
    lda.b $C0                   ; $80BB8D
    sta.w $2130                 ; $80BB8F
    lda.w #$2200                ; $80BB92
    sta.w $4300                 ; $80BB95
    lda.w #$0500                ; $80BB98
    sta.w $4302                 ; $80BB9B
    lda.w #$0200                ; $80BB9E
    sta.w $4305                 ; $80BBA1
    sep.b #$20                  ; $80BBA4
    stz.w $2121                 ; $80BBA6
    lda.b #$81                  ; $80BBA9
    sta.w $4304                 ; $80BBAB
    lda.b #$01                  ; $80BBAE
    sta.w $420B                 ; $80BBB0
    rep.b #$20                  ; $80BBB3
    stz.w $2102                 ; $80BBB5
    lda.w #$0402                ; $80BBB8
    sta.w $4300                 ; $80BBBB
    lda.w $016E                 ; $80BBBE
    sta.w $4302                 ; $80BBC1
    lda.w #$0220                ; $80BBC4
    sta.w $4305                 ; $80BBC7
    sep.b #$20                  ; $80BBCA
    lda.b #$81                  ; $80BBCC
    sta.w $4304                 ; $80BBCE
    lda.b #$01                  ; $80BBD1
    sta.w $420B                 ; $80BBD3
    rep.b #$20                  ; $80BBD6
    stz.w $2102                 ; $80BBD8
    lda.w #$006C                ; $80BBDB
    sta.w $4209                 ; $80BBDE
    lda.w #$0010                ; $80BBE1
    sta.w $4207                 ; $80BBE4
    sep.b #$20                  ; $80BBE7
    lda.b #$B1                  ; $80BBE9
    sta.w $4200                 ; $80BBEB
    rep.b #$20                  ; $80BBEE
    lda.w #$BC9B                ; $80BBF0
    sta.w $0005                 ; $80BBF3
    lda.b $64                   ; $80BBF6
    beq CODE_80BC46             ; $80BBF8
    lda.b $6A                   ; $80BBFA
    sta.w $2116                 ; $80BBFC
    lda.b $66                   ; $80BBFF
    sta.w $4302                 ; $80BC01
    lda.b $64                   ; $80BC04
    cmp.w #$1C00                ; $80BC06
    bcc CODE_80BC2D             ; $80BC09
    sec                         ; $80BC0B
    sbc.w #$1C00                ; $80BC0C
    sta.b $64                   ; $80BC0F
    lda.b $66                   ; $80BC11
    clc                         ; $80BC13
    adc.w #$1C00                ; $80BC14
    sta.b $66                   ; $80BC17
    lda.b $68                   ; $80BC19
    adc.w #$0000                ; $80BC1B
    sta.b $68                   ; $80BC1E
    lda.b $6A                   ; $80BC20
    clc                         ; $80BC22
    adc.w #$0E00                ; $80BC23
    sta.b $6A                   ; $80BC26
    lda.w #$1C00                ; $80BC28
    bra CODE_80BC2F             ; $80BC2B
CODE_80BC2D:
    stz.b $64                   ; $80BC2D
CODE_80BC2F:
    sta.w $4305                 ; $80BC2F
    lda.w #$1801                ; $80BC32
    sta.w $4300                 ; $80BC35
    sep.b #$20                  ; $80BC38
    lda.b $68                   ; $80BC3A
    sta.w $4304                 ; $80BC3C
    lda.b #$01                  ; $80BC3F
    sta.w $420B                 ; $80BC41
    rep.b #$20                  ; $80BC44
CODE_80BC46:
    sep.b #$20                  ; $80BC46
    lda.w $019B                 ; $80BC48
    bne CODE_80BC4F             ; $80BC4B
    ora.b #$80                  ; $80BC4D
CODE_80BC4F:
    sta.w $2100                 ; $80BC4F
    rep.b #$20                  ; $80BC52
    lda.w #$0000                ; $80BC54
    tcd                         ; $80BC57
    tsc                         ; $80BC58
    sta.w $0278                 ; $80BC59
    lda.w $0178                 ; $80BC5C
    tcs                         ; $80BC5F
    lda.w $0176                 ; $80BC60
    and.w $0276                 ; $80BC63
    and.w #$0010                ; $80BC66
    beq CODE_80BC77             ; $80BC69
    lsr a                       ; $80BC6B
    tsb.w $0176                 ; $80BC6C
    tsb.w $0276                 ; $80BC6F
    bra CODE_80BC7D             ; $80BC72
    incbin !baserom:$3C74..$3C77
CODE_80BC77:
    trb.w $0176                 ; $80BC77
    trb.w $0276                 ; $80BC7A
CODE_80BC7D:
    lda.w #$0080                ; $80BC7D
CODE_80BC80:
    bit.w $4212                 ; $80BC80
    bne CODE_80BC80             ; $80BC83
    phd                         ; $80BC85
    phb                         ; $80BC86
    pea.w $8181                 ; $80BC87
    plb                         ; $80BC8A
    plb                         ; $80BC8B
    lda.w #$0100                ; $80BC8C
    tcd                         ; $80BC8F
    ldx.w #$0000                ; $80BC90
    jsr.w ($01EC,x)             ; $80BC93
    plb                         ; $80BC96
    pld                         ; $80BC97
    jmp.w $8934                 ; $80BC98
    rep.b #$30                  ; $80BC9B
    phb                         ; $80BC9D
    pha                         ; $80BC9E
    phx                         ; $80BC9F
    phy                         ; $80BCA0
    phd                         ; $80BCA1
    phk                         ; $80BCA2
    plb                         ; $80BCA3
    lda.w $4210                 ; $80BCA4
    lda.w #$2100                ; $80BCA7
    tcd                         ; $80BCAA
    lda.w #$0402                ; $80BCAB
    sta.w $4300                 ; $80BCAE
    lda.w $026E                 ; $80BCB1
    sta.w $4302                 ; $80BCB4
    lda.w #$0220                ; $80BCB7
    sta.w $4305                 ; $80BCBA
    sep.b #$20                  ; $80BCBD
    lda.b #$81                  ; $80BCBF
    sta.w $4304                 ; $80BCC1
    lda.w $029B                 ; $80BCC4
    bne CODE_80BCCB             ; $80BCC7
    ora.b #$80                  ; $80BCC9
CODE_80BCCB:
    pha                         ; $80BCCB
    rep.b #$20                  ; $80BCCC
    lda.w #$029C                ; $80BCCE
    stz.w $2182                 ; $80BCD1
    sta.w $2181                 ; $80BCD4
    lda.w #$010A                ; $80BCD7
    sta.w $4207                 ; $80BCDA
    sep.b #$20                  ; $80BCDD
    lda.b #$91                  ; $80BCDF
    sta.w $4200                 ; $80BCE1
    lda.w $4211                 ; $80BCE4
    lda.b #$80                  ; $80BCE7
    wai                         ; $80BCE9
    sta.b $00                   ; $80BCEA
    lda.b $80                   ; $80BCEC
    sta.b $0D                   ; $80BCEE
    lda.b $80                   ; $80BCF0
    sta.b $0D                   ; $80BCF2
    sec                         ; $80BCF4
    lda.b $80                   ; $80BCF5
    sbc.b #$74                  ; $80BCF7
    sta.b $0E                   ; $80BCF9
    lda.b $80                   ; $80BCFB
    sbc.b #$00                  ; $80BCFD
    sta.b $0E                   ; $80BCFF
    lda.b $80                   ; $80BD01
    sta.b $0F                   ; $80BD03
    lda.b $80                   ; $80BD05
    sta.b $0F                   ; $80BD07
    sec                         ; $80BD09
    lda.b $80                   ; $80BD0A
    sbc.b #$74                  ; $80BD0C
    sta.b $10                   ; $80BD0E
    lda.b $80                   ; $80BD10
    sbc.b #$00                  ; $80BD12
    sta.b $10                   ; $80BD14
    lda.b $80                   ; $80BD16
    sta.b $11                   ; $80BD18
    lda.b $80                   ; $80BD1A
    sta.b $11                   ; $80BD1C
    sec                         ; $80BD1E
    lda.b $80                   ; $80BD1F
    sbc.b #$74                  ; $80BD21
    sta.b $12                   ; $80BD23
    lda.b $80                   ; $80BD25
    sbc.b #$00                  ; $80BD27
    sta.b $12                   ; $80BD29
    lda.b $80                   ; $80BD2B
    sta.b $05                   ; $80BD2D
    lda.b $80                   ; $80BD2F
    sta.b $25                   ; $80BD31
    lda.b $80                   ; $80BD33
    ora.b #$20                  ; $80BD35
    sta.b $32                   ; $80BD37
    lda.b $80                   ; $80BD39
    ora.b #$40                  ; $80BD3B
    sta.b $32                   ; $80BD3D
    lda.b $80                   ; $80BD3F
    ora.b #$80                  ; $80BD41
    sta.b $32                   ; $80BD43
    lda.b $80                   ; $80BD45
    sta.b $01                   ; $80BD47
    stz.b $21                   ; $80BD49
    rep.b #$20                  ; $80BD4B
    lda.w $02AE                 ; $80BD4D
    sta.b $26                   ; $80BD50
    lda.w $02B0                 ; $80BD52
    sta.b $28                   ; $80BD55
    lda.w $01B2                 ; $80BD57
    sta.b $2A                   ; $80BD5A
    lda.w $02B4                 ; $80BD5C
    sta.b $07                   ; $80BD5F
    lda.w $02B6                 ; $80BD61
    sta.b $09                   ; $80BD64
    lda.w $02B8                 ; $80BD66
    sta.b $0B                   ; $80BD69
    lda.w $02BA                 ; $80BD6B
    sta.b $23                   ; $80BD6E
    lda.w $02BC                 ; $80BD70
    sta.b $2C                   ; $80BD73
    lda.w $02BE                 ; $80BD75
    sta.b $2E                   ; $80BD78
    lda.w $02C0                 ; $80BD7A
    sta.b $30                   ; $80BD7D
    stz.b $02                   ; $80BD7F
    lda.w #$4300                ; $80BD81
    tcd                         ; $80BD84
    lda.w #$0100                ; $80BD85
    sta.w $420A                 ; $80BD88
    lda.w #$2200                ; $80BD8B
    sta.b $00                   ; $80BD8E
    lda.w #$0700                ; $80BD90
    sta.b $02                   ; $80BD93
    lda.w #$0200                ; $80BD95
    sta.b $05                   ; $80BD98
    sep.b #$20                  ; $80BD9A
    lda.b #$81                  ; $80BD9C
    sta.b $04                   ; $80BD9E
    lda.b #$01                  ; $80BDA0
    sta.w $420B                 ; $80BDA2
    lda.b #$0A                  ; $80BDA5
    sta.w $4207                 ; $80BDA7
    lda.b #$01                  ; $80BDAA
    sta.w $4208                 ; $80BDAC
    lda.w $4211                 ; $80BDAF
    pla                         ; $80BDB2
    wai                         ; $80BDB3
    sta.w $2100                 ; $80BDB4
    lda.w $4211                 ; $80BDB7
    lda.b #$B1                  ; $80BDBA
    sta.w $4200                 ; $80BDBC
    rep.b #$20                  ; $80BDBF
    tsc                         ; $80BDC1
    sta.w $0178                 ; $80BDC2
    lda.w $0278                 ; $80BDC5
    tcs                         ; $80BDC8
    lda.w #$0200                ; $80BDC9
    tcd                         ; $80BDCC
    lda.b $64                   ; $80BDCD
    beq CODE_80BE1D             ; $80BDCF
    lda.b $6A                   ; $80BDD1
    sta.w $2116                 ; $80BDD3
    lda.b $66                   ; $80BDD6
    sta.w $4302                 ; $80BDD8
    lda.b $64                   ; $80BDDB
    cmp.w #$1C00                ; $80BDDD
    bcc CODE_80BE04             ; $80BDE0
    sec                         ; $80BDE2
    sbc.w #$1C00                ; $80BDE3
    sta.b $64                   ; $80BDE6
    lda.b $66                   ; $80BDE8
    clc                         ; $80BDEA
    adc.w #$1C00                ; $80BDEB
    sta.b $66                   ; $80BDEE
    lda.b $68                   ; $80BDF0
    adc.w #$0000                ; $80BDF2
    sta.b $68                   ; $80BDF5
    lda.b $6A                   ; $80BDF7
    clc                         ; $80BDF9
    adc.w #$0E00                ; $80BDFA
    sta.b $6A                   ; $80BDFD
    lda.w #$1C00                ; $80BDFF
    bra CODE_80BE06             ; $80BE02
CODE_80BE04:
    stz.b $64                   ; $80BE04
CODE_80BE06:
    sta.w $4305                 ; $80BE06
    lda.w #$1801                ; $80BE09
    sta.w $4300                 ; $80BE0C
    sep.b #$20                  ; $80BE0F
    lda.b $68                   ; $80BE11
    sta.w $4304                 ; $80BE13
    lda.b #$01                  ; $80BE16
    sta.w $420B                 ; $80BE18
    rep.b #$20                  ; $80BE1B
CODE_80BE1D:
    pea.w $8181                 ; $80BE1D
    plb                         ; $80BE20
    plb                         ; $80BE21
    lda.w #$0200                ; $80BE22
    tcd                         ; $80BE25
    ldx.w #$0000                ; $80BE26
    jsr.w ($02EC,x)             ; $80BE29
    pld                         ; $80BE2C
    ply                         ; $80BE2D
    plx                         ; $80BE2E
    pla                         ; $80BE2F
    plb                         ; $80BE30
    rti                         ; $80BE31
    stz.w $019A                 ; $80BE32
    ldx.w #$A26F                ; $80BE35
    lda.w #$0084                ; $80BE38
    jsr.w $81CC                 ; $80BE3B
    lda.w #$0001                ; $80BE3E
    jsr.w $8373                 ; $80BE41
    phb                         ; $80BE44
    ldx.w #$BF2A                ; $80BE45
    ldy.w #$1B00                ; $80BE48
    lda.w #$0015                ; $80BE4B
    mvn $81,$81                 ; $80BE4E
    plb                         ; $80BE51
    lda.w #$1002                ; $80BE52
    sta.w $4310                 ; $80BE55
    lda.w #$1B00                ; $80BE58
    sta.w $4312                 ; $80BE5B
    sep.b #$20                  ; $80BE5E
    lda.b #$81                  ; $80BE60
    sta.w $4314                 ; $80BE62
    sta.w $4317                 ; $80BE65
    lda.b #$02                  ; $80BE68
    sta.w $420C                 ; $80BE6A
    rep.b #$20                  ; $80BE6D
    stz.w $01F2                 ; $80BE6F
    lda.w #$000C                ; $80BE72
    sta.w $02F2                 ; $80BE75
    stz.w $013E                 ; $80BE78
    stz.w $0140                 ; $80BE7B
    stz.w $0142                 ; $80BE7E
    stz.w $0144                 ; $80BE81
    stz.w $0146                 ; $80BE84
    stz.w $023E                 ; $80BE87
    stz.w $0240                 ; $80BE8A
    stz.w $0242                 ; $80BE8D
    stz.w $0244                 ; $80BE90
    stz.w $0246                 ; $80BE93
    stz.w $0148                 ; $80BE96
    stz.w $014A                 ; $80BE99
    stz.w $014C                 ; $80BE9C
    stz.w $014E                 ; $80BE9F
    stz.w $0248                 ; $80BEA2
    stz.w $024A                 ; $80BEA5
    stz.w $024C                 ; $80BEA8
    stz.w $024E                 ; $80BEAB
    ldx.w #$002A                ; $80BEAE
CODE_80BEB1:
    stz.w $0112,x               ; $80BEB1
    stz.w $0212,x               ; $80BEB4
    dex                         ; $80BEB7
    dex                         ; $80BEB8
    bne CODE_80BEB1             ; $80BEB9
    jsr.w $B7E7                 ; $80BEBB
    lda.w #$0040                ; $80BEBE
    tsb.b $76                   ; $80BEC1
    jmp.w $BF0F                 ; $80BEC3
    stz.w $019A                 ; $80BEC6
    ldx.w #$A26F                ; $80BEC9
    lda.w #$0084                ; $80BECC
    jsr.w $81CC                 ; $80BECF
    lda.w #$0001                ; $80BED2
    jsr.w $8373                 ; $80BED5
    phb                         ; $80BED8
    ldx.w #$BF2A                ; $80BED9
    ldy.w #$1B00                ; $80BEDC
    lda.w #$0015                ; $80BEDF
    mvn $81,$81                 ; $80BEE2
    plb                         ; $80BEE5
    lda.w #$1002                ; $80BEE6
    sta.w $4310                 ; $80BEE9
    lda.w #$1B00                ; $80BEEC
    sta.w $4312                 ; $80BEEF
    sep.b #$20                  ; $80BEF2
    lda.b #$81                  ; $80BEF4
    sta.w $4314                 ; $80BEF6
    sta.w $4317                 ; $80BEF9
    lda.b #$02                  ; $80BEFC
    sta.w $420C                 ; $80BEFE
    rep.b #$20                  ; $80BF01
    stz.w $01F2                 ; $80BF03
    lda.w #$000C                ; $80BF06
    sta.w $02F2                 ; $80BF09
    jsr.w $B7E7                 ; $80BF0C
    ldx.b $92                   ; $80BF0F
    lda.w $8B2C,x               ; $80BF11
    sta.b $B8                   ; $80BF14
    lda.w $8B2F,x               ; $80BF16
    sta.b $B4                   ; $80BF19
    lda.w $8B31,x               ; $80BF1B
    sta.b $B6                   ; $80BF1E
    lda.w $8B33,x               ; $80BF20
    sta.b $5C                   ; $80BF23
    sep.b #$20                  ; $80BF25
    lda.w $8B2E,x               ; $80BF27
    sta.b $AD                   ; $80BF2A
    rep.b #$20                  ; $80BF2C
    lda.b $5C                   ; $80BF2E
    clc                         ; $80BF30
    adc.w #$3000                ; $80BF31
    tay                         ; $80BF34
    ldx.w #$8190                ; $80BF35
    lda.w #$0087                ; $80BF38
    jsr.w $B95D                 ; $80BF3B
    lda.b $5C                   ; $80BF3E
    clc                         ; $80BF40
    adc.w #$3500                ; $80BF41
    tay                         ; $80BF44
    ldx.w #$C320                ; $80BF45
    lda.w #$0090                ; $80BF48
    jsr.w $B95D                 ; $80BF4B
    lda.b $5C                   ; $80BF4E
    clc                         ; $80BF50
    adc.w #$3540                ; $80BF51
    tay                         ; $80BF54
    ldx.w #$86BE                ; $80BF55
    lda.w #$0087                ; $80BF58
    jsr.w $B95D                 ; $80BF5B
    ldx.w #$83CC                ; $80BF5E
    lda.b $5C                   ; $80BF61
    clc                         ; $80BF63
    adc.w #$1000                ; $80BF64
    tay                         ; $80BF67
    lda.w #$008F                ; $80BF68
    jsr.w $B95D                 ; $80BF6B
    ldx.w #$8000                ; $80BF6E
    lda.b $5C                   ; $80BF71
    clc                         ; $80BF73
    adc.w #$1200                ; $80BF74
    tay                         ; $80BF77
    lda.w #$0098                ; $80BF78
    jsr.w $B95D                 ; $80BF7B
CODE_80BF7E:
    lda.b $64                   ; $80BF7E
    bne CODE_80BF7E             ; $80BF80
    lda.b $5E                   ; $80BF82
    clc                         ; $80BF84
    adc.w #$0800                ; $80BF85
    tay                         ; $80BF88
    lda.w #$7F92                ; $80BF89
    ldx.w #$8000                ; $80BF8C
    jsr.w $FBA0                 ; $80BF8F
    lda.b $5E                   ; $80BF92
    clc                         ; $80BF94
    adc.w #$1000                ; $80BF95
    tay                         ; $80BF98
    lda.w #$7F87                ; $80BF99
    ldx.w #$8404                ; $80BF9C
    jsr.w $FBA0                 ; $80BF9F
    lda.b $5E                   ; $80BFA2
    clc                         ; $80BFA4
    adc.w #$1000                ; $80BFA5
    tax                         ; $80BFA8
    lda.b $5E                   ; $80BFA9
    clc                         ; $80BFAB
    adc.w #$08C4                ; $80BFAC
    tay                         ; $80BFAF
    jsl.l $86CE4A               ; $80BFB0
    jsr.w $C31A                 ; $80BFB4
    lda.b $5C                   ; $80BFB7
    clc                         ; $80BFB9
    adc.w #$0400                ; $80BFBA
    sta.b $6A                   ; $80BFBD
    lda.b $5E                   ; $80BFBF
    clc                         ; $80BFC1
    adc.w #$0800                ; $80BFC2
    sta.b $66                   ; $80BFC5
    lda.w #$007F                ; $80BFC7
    sta.b $68                   ; $80BFCA
    lda.w #$0800                ; $80BFCC
    sta.b $64                   ; $80BFCF
    phb                         ; $80BFD1
    ldx.b $5E                   ; $80BFD2
    lda.w #$0000                ; $80BFD4
    sta.l $7F0000,x             ; $80BFD7
    lda.b $5E                   ; $80BFDB
    clc                         ; $80BFDD
    adc.w #$0002                ; $80BFDE
    tay                         ; $80BFE1
    lda.w #$07FD                ; $80BFE2
    mvn $7F,$7F                 ; $80BFE5
    plb                         ; $80BFE8
CODE_80BFE9:
    lda.b $64                   ; $80BFE9
    bne CODE_80BFE9             ; $80BFEB
    lda.b $5C                   ; $80BFED
    sta.b $6A                   ; $80BFEF
    lda.b $5E                   ; $80BFF1
    sta.b $66                   ; $80BFF3
    lda.w #$007F                ; $80BFF5
    sta.b $68                   ; $80BFF8
    lda.w #$0800                ; $80BFFA
    sta.b $64                   ; $80BFFD
CODE_80BFFF:
    lda.b $64                   ; $80BFFF
    bne CODE_80BFFF             ; $80C001
    lda.w #$0027                ; $80C003
    sta.b $9E                   ; $80C006
    sta.b $18                   ; $80C008
    stz.b $16                   ; $80C00A
    lda.w #$0010                ; $80C00C
    sta.b $1A                   ; $80C00F
    lda.w #$007F                ; $80C011
    sta.b $14                   ; $80C014
    lda.b $5C                   ; $80C016
    clc                         ; $80C018
    adc.w #$0060                ; $80C019
    tay                         ; $80C01C
    lda.b $5E                   ; $80C01D
    clc                         ; $80C01F
    adc.w $BDF8                 ; $80C020
    tax                         ; $80C023
    lda.w #$0200                ; $80C024
    jsr.w $B537                 ; $80C027
    lda.w #$0013                ; $80C02A
    sta.b $BC                   ; $80C02D
    lda.b $76                   ; $80C02F
    bit.w #$0040                ; $80C031
    beq CODE_80C039             ; $80C034
    jmp.w $D17C                 ; $80C036
CODE_80C039:
    lda.w #$0F00                ; $80C039
    sta.b $98                   ; $80C03C
    lda.w #$0100                ; $80C03E
    sta.b $96                   ; $80C041
    lda.w #$0010                ; $80C043
    jsr.w $8373                 ; $80C046
    lda.b $76                   ; $80C049
    bit.w #$0020                ; $80C04B
    beq CODE_80C053             ; $80C04E
    jmp.w $D16C                 ; $80C050
CODE_80C053:
    ldx.w #$0000                ; $80C053
CODE_80C056:
    lda.w #$0001                ; $80C056
    jsr.w $8373                 ; $80C059
    dec.b $1A                   ; $80C05C
    bne CODE_80C082             ; $80C05E
    lda.w #$007F                ; $80C060
    sta.b $14                   ; $80C063
    lda.b $5C                   ; $80C065
    clc                         ; $80C067
    adc.w #$0060                ; $80C068
    tay                         ; $80C06B
    lda.b $5E                   ; $80C06C
    clc                         ; $80C06E
    adc.w $BDF8,x               ; $80C06F
    phx                         ; $80C072
    tax                         ; $80C073
    lda.w #$0200                ; $80C074
    jsr.w $B537                 ; $80C077
    plx                         ; $80C07A
    inx                         ; $80C07B
    inx                         ; $80C07C
    lda.w #$0004                ; $80C07D
    sta.b $1A                   ; $80C080
CODE_80C082:
    lda.b $18                   ; $80C082
    cmp.w #$FFFF                ; $80C084
    beq CODE_80C091             ; $80C087
    sec                         ; $80C089
    sbc.w #$0002                ; $80C08A
    sta.b $18                   ; $80C08D
    sta.b $9E                   ; $80C08F
CODE_80C091:
    cpx.w #$0010                ; $80C091
    bne CODE_80C056             ; $80C094
    ldy.w #$000C                ; $80C096
    ldx.b $5E                   ; $80C099
    lda.w #$0000                ; $80C09B
CODE_80C09E:
    sta.l $7F08D4,x             ; $80C09E
    sta.l $7F0914,x             ; $80C0A2
    sta.l $7F0954,x             ; $80C0A6
    sta.l $7F0994,x             ; $80C0AA
    sta.l $7F09D4,x             ; $80C0AE
    sta.l $7F0A14,x             ; $80C0B2
    sta.l $7F0A54,x             ; $80C0B6
    sta.l $7F0A94,x             ; $80C0BA
    inx                         ; $80C0BE
    inx                         ; $80C0BF
    dey                         ; $80C0C0
    bne CODE_80C09E             ; $80C0C1
    lda.w #$007F                ; $80C0C3
    sta.b $14                   ; $80C0C6
    lda.b $5C                   ; $80C0C8
    clc                         ; $80C0CA
    adc.w #$0400                ; $80C0CB
    tay                         ; $80C0CE
    lda.b $5E                   ; $80C0CF
    clc                         ; $80C0D1
    adc.w #$0800                ; $80C0D2
    tax                         ; $80C0D5
    lda.w #$0380                ; $80C0D6
    jsr.w $B537                 ; $80C0D9
    lda.w #$007F                ; $80C0DC
    sta.b $14                   ; $80C0DF
    lda.b $5C                   ; $80C0E1
    clc                         ; $80C0E3
    adc.w #$0060                ; $80C0E4
    tay                         ; $80C0E7
    lda.b $5E                   ; $80C0E8
    clc                         ; $80C0EA
    adc.w $BE08                 ; $80C0EB
    tax                         ; $80C0EE
    lda.w #$0200                ; $80C0EF
    jsr.w $B537                 ; $80C0F2
    phb                         ; $80C0F5
    pea.w $7F7F                 ; $80C0F6
    plb                         ; $80C0F9
    plb                         ; $80C0FA
    ldx.b $5E                   ; $80C0FB
    ldy.w #$0008                ; $80C0FD
    lda.w #$FFFF                ; $80C100
CODE_80C103:
    sta.w $0000,x               ; $80C103
    sta.w $0020,x               ; $80C106
    sta.w $0040,x               ; $80C109
    sta.w $0060,x               ; $80C10C
    sta.w $0080,x               ; $80C10F
    sta.w $00A0,x               ; $80C112
    sta.w $00C0,x               ; $80C115
    sta.w $00E0,x               ; $80C118
    sta.w $0100,x               ; $80C11B
    sta.w $0120,x               ; $80C11E
    sta.w $0140,x               ; $80C121
    sta.w $0160,x               ; $80C124
    sta.w $0180,x               ; $80C127
    sta.w $01A0,x               ; $80C12A
    sta.w $01C0,x               ; $80C12D
    sta.w $01E0,x               ; $80C130
    inx                         ; $80C133
    inx                         ; $80C134
    dey                         ; $80C135
    bne CODE_80C103             ; $80C136
    plb                         ; $80C138
    lda.w #$0008                ; $80C139
    sta.b $16                   ; $80C13C
    lda.b $8C                   ; $80C13E
CODE_80C140:
    cmp.b $8E                   ; $80C140
    bne CODE_80C140             ; $80C142
    phb                         ; $80C144
    pea.w $7F7F                 ; $80C145
    plb                         ; $80C148
    plb                         ; $80C149
    ldx.b $5E                   ; $80C14A
    ldy.w #$000C                ; $80C14C
CODE_80C14F:
    jsr.w $87E9                 ; $80C14F
    and.w #$C00F                ; $80C152
    clc                         ; $80C155
    adc.w #$0D60                ; $80C156
    sta.w $08D4,x               ; $80C159
    jsr.w $87E9                 ; $80C15C
    and.w #$C00F                ; $80C15F
    clc                         ; $80C162
    adc.w #$0D60                ; $80C163
    sta.w $0914,x               ; $80C166
    jsr.w $87E9                 ; $80C169
    and.w #$C00F                ; $80C16C
    clc                         ; $80C16F
    adc.w #$0D60                ; $80C170
    sta.w $0954,x               ; $80C173
    jsr.w $87E9                 ; $80C176
    and.w #$C00F                ; $80C179
    clc                         ; $80C17C
    adc.w #$0D60                ; $80C17D
    sta.w $0994,x               ; $80C180
    jsr.w $87E9                 ; $80C183
    and.w #$C00F                ; $80C186
    clc                         ; $80C189
    adc.w #$0D60                ; $80C18A
    sta.w $09D4,x               ; $80C18D
    jsr.w $87E9                 ; $80C190
    and.w #$C00F                ; $80C193
    clc                         ; $80C196
    adc.w #$0D60                ; $80C197
    sta.w $0A14,x               ; $80C19A
    jsr.w $87E9                 ; $80C19D
    and.w #$C00F                ; $80C1A0
    clc                         ; $80C1A3
    adc.w #$0D60                ; $80C1A4
    sta.w $0A54,x               ; $80C1A7
    jsr.w $87E9                 ; $80C1AA
    and.w #$C00F                ; $80C1AD
    clc                         ; $80C1B0
    adc.w #$0D60                ; $80C1B1
    sta.w $0A94,x               ; $80C1B4
    inx                         ; $80C1B7
    inx                         ; $80C1B8
    dey                         ; $80C1B9
    bne CODE_80C14F             ; $80C1BA
    ldx.b $5E                   ; $80C1BC
    ldy.w #$0008                ; $80C1BE
CODE_80C1C1:
    jsr.w $87E9                 ; $80C1C1
    sta.w $0010,x               ; $80C1C4
    jsr.w $87E9                 ; $80C1C7
    sta.w $0030,x               ; $80C1CA
    jsr.w $87E9                 ; $80C1CD
    sta.w $0050,x               ; $80C1D0
    jsr.w $87E9                 ; $80C1D3
    sta.w $0070,x               ; $80C1D6
    jsr.w $87E9                 ; $80C1D9
    sta.w $0090,x               ; $80C1DC
    jsr.w $87E9                 ; $80C1DF
    sta.w $00B0,x               ; $80C1E2
    jsr.w $87E9                 ; $80C1E5
    sta.w $00D0,x               ; $80C1E8
    jsr.w $87E9                 ; $80C1EB
    sta.w $00F0,x               ; $80C1EE
    jsr.w $87E9                 ; $80C1F1
    sta.w $0110,x               ; $80C1F4
    jsr.w $87E9                 ; $80C1F7
    sta.w $0130,x               ; $80C1FA
    jsr.w $87E9                 ; $80C1FD
    sta.w $0150,x               ; $80C200
    jsr.w $87E9                 ; $80C203
    sta.w $0170,x               ; $80C206
    jsr.w $87E9                 ; $80C209
    sta.w $0190,x               ; $80C20C
    jsr.w $87E9                 ; $80C20F
    sta.w $01B0,x               ; $80C212
    jsr.w $87E9                 ; $80C215
    sta.w $01D0,x               ; $80C218
    jsr.w $87E9                 ; $80C21B
    sta.w $01F0,x               ; $80C21E
    inx                         ; $80C221
    inx                         ; $80C222
    dey                         ; $80C223
    bne CODE_80C1C1             ; $80C224
    plb                         ; $80C226
    lda.b $8C                   ; $80C227
CODE_80C229:
    cmp.b $8E                   ; $80C229
    bne CODE_80C229             ; $80C22B
    lda.w #$007F                ; $80C22D
    sta.b $14                   ; $80C230
    lda.b $5C                   ; $80C232
    clc                         ; $80C234
    adc.w #$0400                ; $80C235
    tay                         ; $80C238
    lda.b $5E                   ; $80C239
    clc                         ; $80C23B
    adc.w #$0800                ; $80C23C
    tax                         ; $80C23F
    lda.w #$0380                ; $80C240
    jsr.w $B537                 ; $80C243
    lda.w #$007F                ; $80C246
    sta.b $14                   ; $80C249
    lda.b $5C                   ; $80C24B
    clc                         ; $80C24D
    adc.w #$2600                ; $80C24E
    tay                         ; $80C251
    ldx.b $5E                   ; $80C252
    lda.w #$0200                ; $80C254
    jsr.w $B537                 ; $80C257
    dec.b $16                   ; $80C25A
    beq CODE_80C261             ; $80C25C
    jmp.w $C144                 ; $80C25E
CODE_80C261:
    ldy.w #$000C                ; $80C261
    ldx.b $5E                   ; $80C264
    lda.w #$0000                ; $80C266
CODE_80C269:
    sta.l $7F08D4,x             ; $80C269
    sta.l $7F0914,x             ; $80C26D
    sta.l $7F0954,x             ; $80C271
    sta.l $7F0994,x             ; $80C275
    sta.l $7F09D4,x             ; $80C279
    sta.l $7F0A14,x             ; $80C27D
    sta.l $7F0A54,x             ; $80C281
    sta.l $7F0A94,x             ; $80C285
    inx                         ; $80C289
    inx                         ; $80C28A
    dey                         ; $80C28B
    bne CODE_80C269             ; $80C28C
    stz.b $48                   ; $80C28E
    stz.b $4A                   ; $80C290
    jsr.w $C35E                 ; $80C292
    jsr.w $89D6                 ; $80C295
    lda.w #$007F                ; $80C298
    sta.b $14                   ; $80C29B
    lda.b $5C                   ; $80C29D
    clc                         ; $80C29F
    adc.w #$0400                ; $80C2A0
    tay                         ; $80C2A3
    lda.b $5E                   ; $80C2A4
    clc                         ; $80C2A6
    adc.w #$0800                ; $80C2A7
    tax                         ; $80C2AA
    lda.w #$0380                ; $80C2AB
    jsr.w $B537                 ; $80C2AE
    lda.w #$0001                ; $80C2B1
    jsr.w $8373                 ; $80C2B4
    stz.b $1E                   ; $80C2B7
    stz.b $4C                   ; $80C2B9
CODE_80C2BB:
    lda.b $1F                   ; $80C2BB
    and.w #$001E                ; $80C2BD
    tax                         ; $80C2C0
    ldy.b $C4                   ; $80C2C1
    lda.w $BE1A,x               ; $80C2C3
    sta.w $0122,y               ; $80C2C6
    lda.w $BE3A,x               ; $80C2C9
    sta.w $0124,y               ; $80C2CC
    lda.b $1E                   ; $80C2CF
    clc                         ; $80C2D1
    adc.w #$0020                ; $80C2D2
    sta.b $1E                   ; $80C2D5
    jsr.w $C35E                 ; $80C2D7
    jsr.w $89D6                 ; $80C2DA
    lda.w #$0800                ; $80C2DD
    trb.b $0E                   ; $80C2E0
    beq CODE_80C2F0             ; $80C2E2
    incbin !baserom:$42E4..$42F0
CODE_80C2F0:
    lda.w #$0400                ; $80C2F0
    trb.b $0E                   ; $80C2F3
    beq CODE_80C306             ; $80C2F5
    lda.b $48                   ; $80C2F7
    cmp.w #$0003                ; $80C2F9
    beq CODE_80C306             ; $80C2FC
    inc.b $48                   ; $80C2FE
    stz.b $1E                   ; $80C300
    jsl.l $80E4C5               ; $80C302
CODE_80C306:
    lda.w #$C0C0                ; $80C306
    trb.b $0E                   ; $80C309
    beq CODE_80C318             ; $80C30B
    lda.b $4C                   ; $80C30D
    bne CODE_80C318             ; $80C30F
    jsl.l $80E4DB               ; $80C311
    jsr.w $C4F9                 ; $80C315
CODE_80C318:
    bra CODE_80C2BB             ; $80C318
    phb                         ; $80C31A
    ldx.w #$8000                ; $80C31B
    ldy.b $C4                   ; $80C31E
    lda.w #$005F                ; $80C320
    mvn $81,$9B                 ; $80C323
    ldx.w #$B2D2                ; $80C326
    lda.b $C4                   ; $80C329
    clc                         ; $80C32B
    adc.w #$0060                ; $80C32C
    tay                         ; $80C32F
    lda.w #$001F                ; $80C330
    mvn $81,$80                 ; $80C333
    plb                         ; $80C336
    ldy.b $C4                   ; $80C337
    lda.w #$0E03                ; $80C339
    sta.w $0102,y               ; $80C33C
    lda.w #$0040                ; $80C33F
    sta.w $0104,y               ; $80C342
    lda.w #$109F                ; $80C345
    sta.w $0106,y               ; $80C348
    lda.w #$0000                ; $80C34B
    sta.w $0108,y               ; $80C34E
    lda.w #$0E03                ; $80C351
    sta.w $0122,y               ; $80C354
    lda.w #$0040                ; $80C357
    sta.w $0124,y               ; $80C35A
    rts                         ; $80C35D
    lda.b $4A                   ; $80C35E
    and.w #$FF00                ; $80C360
    ldx.w #$0000                ; $80C363
CODE_80C366:
    cmp.w $BE0A,x               ; $80C366
    bmi CODE_80C371             ; $80C369
    beq CODE_80C3B5             ; $80C36B
    inx                         ; $80C36D
    inx                         ; $80C36E
    bra CODE_80C366             ; $80C36F
CODE_80C371:
    lda.b $4A                   ; $80C371
    and.w #$FF00                ; $80C373
    eor.w #$FFFF                ; $80C376
    sec                         ; $80C379
    adc.w $BE0A,x               ; $80C37A
    sta.w $4204                 ; $80C37D
    sep.b #$20                  ; $80C380
    lda.w $BE0B,x               ; $80C382
    sec                         ; $80C385
    sbc.w $BE09,x               ; $80C386
    sta.w $4206                 ; $80C389
    xba                         ; $80C38C
    xba                         ; $80C38D
    xba                         ; $80C38E
    xba                         ; $80C38F
    nop                         ; $80C390
    nop                         ; $80C391
    lda.w $4214                 ; $80C392
    sta.w $211B                 ; $80C395
    lda.w $4215                 ; $80C398
    sta.w $211B                 ; $80C39B
    lda.w $BE12,x               ; $80C39E
    sec                         ; $80C3A1
    sbc.w $BE10,x               ; $80C3A2
    sta.w $211C                 ; $80C3A5
    rep.b #$20                  ; $80C3A8
    lda.w $BE12,x               ; $80C3AA
    sec                         ; $80C3AD
    sbc.w $2135                 ; $80C3AE
    sta.b $14                   ; $80C3B1
    bra CODE_80C3BA             ; $80C3B3
CODE_80C3B5:
    lda.w $BE12,x               ; $80C3B5
    sta.b $14                   ; $80C3B8
CODE_80C3BA:
    ldx.b $74                   ; $80C3BA
    lda.b $4A                   ; $80C3BC
    clc                         ; $80C3BE
    adc.w #$1D80                ; $80C3BF
    clc                         ; $80C3C2
    adc.b $E7                   ; $80C3C3
    sta.b $18                   ; $80C3C5
    stz.b $1A                   ; $80C3C7
    jsr.w $C475                 ; $80C3C9
    lda.b $18                   ; $80C3CC
    clc                         ; $80C3CE
    adc.w #$0600                ; $80C3CF
    sta.b $18                   ; $80C3D2
    lda.w #$8000                ; $80C3D4
    sta.b $1A                   ; $80C3D7
    jsr.w $C475                 ; $80C3D9
    stx.b $74                   ; $80C3DC
    lda.w #$2080                ; $80C3DE
    clc                         ; $80C3E1
    adc.b $E7                   ; $80C3E2
    sta.b $18                   ; $80C3E4
    lda.b $48                   ; $80C3E6
    inc a                       ; $80C3E8
    sta.b $1A                   ; $80C3E9
    lda.w #$0100                ; $80C3EB
    sta.b $14                   ; $80C3EE
    dec.b $1A                   ; $80C3F0
    bne CODE_80C3F9             ; $80C3F2
    lda.w #$0300                ; $80C3F4
    sta.b $14                   ; $80C3F7
CODE_80C3F9:
    ldy.b $18                   ; $80C3F9
    ldx.w #$BDE0                ; $80C3FB
    lda.w #$0081                ; $80C3FE
    jsr.w $84BE                 ; $80C401
    lda.w #$0100                ; $80C404
    sta.b $14                   ; $80C407
    dec.b $1A                   ; $80C409
    bne CODE_80C412             ; $80C40B
    lda.w #$0300                ; $80C40D
    sta.b $14                   ; $80C410
CODE_80C412:
    lda.b $18                   ; $80C412
    clc                         ; $80C414
    adc.w #$0B00                ; $80C415
    tay                         ; $80C418
    ldx.w #$BDE5                ; $80C419
    lda.w #$0081                ; $80C41C
    jsr.w $84BE                 ; $80C41F
    lda.w #$0100                ; $80C422
    sta.b $14                   ; $80C425
    dec.b $1A                   ; $80C427
    bne CODE_80C430             ; $80C429
    lda.w #$0300                ; $80C42B
    sta.b $14                   ; $80C42E
CODE_80C430:
    lda.b $18                   ; $80C430
    clc                         ; $80C432
    adc.w #$1600                ; $80C433
    tay                         ; $80C436
    ldx.w #$BDED                ; $80C437
    lda.w #$0081                ; $80C43A
    jsr.w $84BE                 ; $80C43D
    lda.w #$0100                ; $80C440
    sta.b $14                   ; $80C443
    dec.b $1A                   ; $80C445
    bne CODE_80C44E             ; $80C447
    lda.w #$0300                ; $80C449
    sta.b $14                   ; $80C44C
CODE_80C44E:
    lda.b $18                   ; $80C44E
    clc                         ; $80C450
    adc.w #$2100                ; $80C451
    tay                         ; $80C454
    ldx.w #$BDF3                ; $80C455
    lda.w #$0081                ; $80C458
    jsr.w $84BE                 ; $80C45B
    lda.b $48                   ; $80C45E
    asl a                       ; $80C460
    tax                         ; $80C461
    lda.b $4A                   ; $80C462
    and.w #$FF00                ; $80C464
    cmp.w $BE0A,x               ; $80C467
    beq CODE_80C474             ; $80C46A
    bpl CODE_80C472             ; $80C46C
    inc.b $4B                   ; $80C46E
    inc.b $4B                   ; $80C470
CODE_80C472:
    dec.b $4B                   ; $80C472
CODE_80C474:
    rts                         ; $80C474
    lda.b $14                   ; $80C475
    lsr a                       ; $80C477
    eor.w #$FFFF                ; $80C478
    sec                         ; $80C47B
    adc.b $18                   ; $80C47C
    sta.w $0900,x               ; $80C47E
    clc                         ; $80C481
    adc.w #$0008                ; $80C482
    sta.w $0904,x               ; $80C485
    clc                         ; $80C488
    adc.w #$0008                ; $80C489
    sta.w $0908,x               ; $80C48C
    clc                         ; $80C48F
    adc.w #$0008                ; $80C490
    sta.w $090C,x               ; $80C493
    lda.b $14                   ; $80C496
    lsr a                       ; $80C498
    adc.b $18                   ; $80C499
    sec                         ; $80C49B
    sbc.w #$0020                ; $80C49C
    sta.w $0910,x               ; $80C49F
    clc                         ; $80C4A2
    adc.w #$0008                ; $80C4A3
    sta.w $0914,x               ; $80C4A6
    clc                         ; $80C4A9
    adc.w #$0006                ; $80C4AA
    sta.w $0918,x               ; $80C4AD
    clc                         ; $80C4B0
    adc.w #$0008                ; $80C4B1
    sta.w $091C,x               ; $80C4B4
    lda.b $1A                   ; $80C4B7
    clc                         ; $80C4B9
    adc.w #$0150                ; $80C4BA
    sta.w $0902,x               ; $80C4BD
    inc a                       ; $80C4C0
    sta.w $0906,x               ; $80C4C1
    sta.w $090A,x               ; $80C4C4
    sta.w $090E,x               ; $80C4C7
    sta.w $0912,x               ; $80C4CA
    sta.w $0916,x               ; $80C4CD
    sta.w $091A,x               ; $80C4D0
    dec a                       ; $80C4D3
    ora.w #$4000                ; $80C4D4
    sta.w $091E,x               ; $80C4D7
    stz.w $0B01,x               ; $80C4DA
    stz.w $0B05,x               ; $80C4DD
    stz.w $0B09,x               ; $80C4E0
    stz.w $0B0D,x               ; $80C4E3
    stz.w $0B11,x               ; $80C4E6
    stz.w $0B15,x               ; $80C4E9
    stz.w $0B19,x               ; $80C4EC
    stz.w $0B1D,x               ; $80C4EF
    txa                         ; $80C4F2
    clc                         ; $80C4F3
    adc.w #$0020                ; $80C4F4
    tax                         ; $80C4F7
    rts                         ; $80C4F8
    lda.b $48                   ; $80C4F9
    asl a                       ; $80C4FB
    tax                         ; $80C4FC
    lda.b $48                   ; $80C4FD
    pha                         ; $80C4FF
    lda.b $4A                   ; $80C500
    pha                         ; $80C502
    jsr.w ($C5D5,x)             ; $80C503
    incbin !baserom:$4506..$45E0
    ldx.b $F2                   ; $80C5E0
    sep.b #$20                  ; $80C5E2
    lda.b #$40                  ; $80C5E4
    sec                         ; $80C5E6
    sbc.b $4C                   ; $80C5E7
    sbc.b $4C                   ; $80C5E9
    bne CODE_80C60D             ; $80C5EB
    lda.b #$01                  ; $80C5ED
    sta.w $1B03,x               ; $80C5EF
    lda.b #$13                  ; $80C5F2
    clc                         ; $80C5F4
    adc.b $4C                   ; $80C5F5
    sta.w $1B06,x               ; $80C5F7
    rep.b #$20                  ; $80C5FA
    lda.w #$FFFF                ; $80C5FC
    sec                         ; $80C5FF
    sbc.b $E8                   ; $80C600
    clc                         ; $80C602
    adc.b $4C                   ; $80C603
    sta.w $1B07,x               ; $80C605
    sta.w $1B04,x               ; $80C608
    bra CODE_80C62F             ; $80C60B
CODE_80C60D:
    sta.w $1B03,x               ; $80C60D
    lda.b #$14                  ; $80C610
    clc                         ; $80C612
    adc.b $4C                   ; $80C613
    sta.w $1B06,x               ; $80C615
    rep.b #$20                  ; $80C618
    lda.w #$FFFF                ; $80C61A
    sec                         ; $80C61D
    sbc.b $E8                   ; $80C61E
    clc                         ; $80C620
    adc.b $4C                   ; $80C621
    sta.w $1B07,x               ; $80C623
    lda.w #$FFFF                ; $80C626
    sec                         ; $80C629
    sbc.b $E8                   ; $80C62A
    sta.w $1B04,x               ; $80C62C
CODE_80C62F:
    lda.b $4E                   ; $80C62F
    asl a                       ; $80C631
    tax                         ; $80C632
    jmp.w ($C636,x)             ; $80C633
    incbin !baserom:$4636..$4642
    lda.b $4C                   ; $80C642
    cmp.w #$0020                ; $80C644
    beq CODE_80C6B7             ; $80C647
    clc                         ; $80C649
    adc.w #$0004                ; $80C64A
    sta.b $4C                   ; $80C64D
    bra CODE_80C6B9             ; $80C64F
    incbin !baserom:$4651..$46B7
CODE_80C6B7:
    stz.b $4E                   ; $80C6B7
CODE_80C6B9:
    ldx.b $F2                   ; $80C6B9
    sep.b #$20                  ; $80C6BB
    lda.b #$18                  ; $80C6BD
    clc                         ; $80C6BF
    adc.b $4C                   ; $80C6C0
    sta.w $1B00,x               ; $80C6C2
    rep.b #$20                  ; $80C6C5
    lda.w #$FFFF                ; $80C6C7
    sec                         ; $80C6CA
    sbc.b $E8                   ; $80C6CB
    sec                         ; $80C6CD
    sbc.b $4C                   ; $80C6CE
    sta.w $1B01,x               ; $80C6D0
    rts                         ; $80C6D3
    incbin !baserom:$46D4..$5168
    pla                         ; $80D168
    pla                         ; $80D169
    pla                         ; $80D16A
    pla                         ; $80D16B
    stz.b $4C                   ; $80D16C
    lda.w #$0001                ; $80D16E
    sta.b $4E                   ; $80D171
    lda.w #$C5E0                ; $80D173
    sta.b $EC                   ; $80D176
CODE_80D178:
    lda.b $4E                   ; $80D178
    bne CODE_80D178             ; $80D17A
    jsr.w $B950                 ; $80D17C
    lda.w #$00FF                ; $80D17F
    trb.w $036A                 ; $80D182
    phb                         ; $80D185
    ldx.w #$C1E9                ; $80D186
    ldy.w #$1B00                ; $80D189
    lda.w #$001D                ; $80D18C
    mvn $81,$81                 ; $80D18F
    plb                         ; $80D192
    lda.w #$FFDB                ; $80D193
    sep.b #$20                  ; $80D196
    sta.w $2110                 ; $80D198
    xba                         ; $80D19B
    sta.w $2110                 ; $80D19C
    lda.b #$40                  ; $80D19F
CODE_80D1A1:
    bit.w $4212                 ; $80D1A1
    bne CODE_80D1A1             ; $80D1A4
    lda.b #$10                  ; $80D1A6
CODE_80D1A8:
    cmp.w $431A                 ; $80D1A8
    bne CODE_80D1A8             ; $80D1AB
    lda.b #$14                  ; $80D1AD
    sta.w $431A                 ; $80D1AF
    rep.b #$20                  ; $80D1B2
    lda.w #$8934                ; $80D1B4
    sta.w $0008                 ; $80D1B7
    lda.w #$89B2                ; $80D1BA
    sta.w $0005                 ; $80D1BD
    sei                         ; $80D1C0
    lda.w #$0008                ; $80D1C1
    sta.b $14                   ; $80D1C4
    lda.w $0176                 ; $80D1C6
    bit.w #$0040                ; $80D1C9
    beq CODE_80D1D3             ; $80D1CC
    lda.w #$003A                ; $80D1CE
    sta.b $14                   ; $80D1D1
CODE_80D1D3:
    stz.b $16                   ; $80D1D3
    lda.w #$E175                ; $80D1D5
    sta.w $0008                 ; $80D1D8
    ldy.w #$0000                ; $80D1DB
    lda.w #$7F9C                ; $80D1DE
    ldx.w #$8FC3                ; $80D1E1
    jsr.w $FBA0                 ; $80D1E4
    ldy.w #$1000                ; $80D1E7
    ldx.w #$8190                ; $80D1EA
    lda.w #$7F87                ; $80D1ED
    jsr.w $FBA0                 ; $80D1F0
    pha                         ; $80D1F3
    ldy.w #$2700                ; $80D1F4
    ldx.w #$80B2                ; $80D1F7
    lda.w #$7F92                ; $80D1FA
    jsr.w $FBA0                 ; $80D1FD
    ldx.w #$0700                ; $80D200
    lda.w #$0048                ; $80D203
CODE_80D206:
    sta.l $7F1FFE,x             ; $80D206
    dex                         ; $80D20A
    dex                         ; $80D20B
    bne CODE_80D206             ; $80D20C
    ldx.w #$0280                ; $80D20E
    lda.w #$0000                ; $80D211
CODE_80D214:
    sta.l $7FC40E,x             ; $80D214
    sta.l $7FC68E,x             ; $80D218
    sta.l $7FC90E,x             ; $80D21C
    sta.l $7FCB8E,x             ; $80D220
    sta.l $7FCE0E,x             ; $80D224
    sta.l $7FD08E,x             ; $80D228
    sta.l $7FD30E,x             ; $80D22C
    sta.l $7FD58E,x             ; $80D230
    dex                         ; $80D234
    dex                         ; $80D235
    bne CODE_80D214             ; $80D236
    jsr.w $DCBB                 ; $80D238
    jsr.w $E0D6                 ; $80D23B
    jsr.w $83E8                 ; $80D23E
CODE_80D241:
    lda.b $16                   ; $80D241
    beq CODE_80D241             ; $80D243
    lda.w #$0001                ; $80D245
    jsr.w $8373                 ; $80D248
CODE_80D24B:
    ldx.b $2A                   ; $80D24B
    cpx.b $28                   ; $80D24D
    bne CODE_80D24B             ; $80D24F
    lda.w #$D784                ; $80D251
    sta.w $0008                 ; $80D254
    lda.w #$0001                ; $80D257
    jsr.w $8373                 ; $80D25A
    lda.w #$8934                ; $80D25D
    sta.w $0008                 ; $80D260
    lda.w #$0001                ; $80D263
    jsr.w $8373                 ; $80D266
    stz.w $0504                 ; $80D269
    lda.w #$2D6B                ; $80D26C
    sta.w $0502                 ; $80D26F
    lda.w #$1BE6                ; $80D272
    sta.w $0500                 ; $80D275
    lda.w #$0000                ; $80D278
    sta.l $7F0000               ; $80D27B
    lda.w #$2601                ; $80D27F
    sta.w $4320                 ; $80D282
    lda.w #$0000                ; $80D285
    sta.w $4322                 ; $80D288
    sep.b #$20                  ; $80D28B
    lda.b #$7F                  ; $80D28D
    sta.w $4324                 ; $80D28F
    rep.b #$20                  ; $80D292
    lda.w #$2801                ; $80D294
    sta.w $4330                 ; $80D297
    lda.w #$0100                ; $80D29A
    sta.w $4332                 ; $80D29D
    sep.b #$20                  ; $80D2A0
    lda.b #$7F                  ; $80D2A2
    sta.w $4334                 ; $80D2A4
    rep.b #$20                  ; $80D2A7
    lda.w #$2E01                ; $80D2A9
    sta.w $4360                 ; $80D2AC
    lda.w #$0D28                ; $80D2AF
    sta.w $4362                 ; $80D2B2
    sep.b #$20                  ; $80D2B5
    lda.b #$81                  ; $80D2B7
    sta.w $4364                 ; $80D2B9
    rep.b #$20                  ; $80D2BC
    lda.w #$0501                ; $80D2BE
    sta.w $4350                 ; $80D2C1
    lda.w #$0D00                ; $80D2C4
    sta.w $4352                 ; $80D2C7
    sep.b #$20                  ; $80D2CA
    lda.b #$81                  ; $80D2CC
    sta.w $4354                 ; $80D2CE
    rep.b #$20                  ; $80D2D1
    stz.w $0D00                 ; $80D2D3
    stz.w $0D14                 ; $80D2D6
    lda.w #$0F02                ; $80D2D9
    sta.w $4340                 ; $80D2DC
    lda.w #$0D14                ; $80D2DF
    sta.w $4342                 ; $80D2E2
    sep.b #$20                  ; $80D2E5
    lda.b #$95                  ; $80D2E7
    sta.w $4344                 ; $80D2E9
    lda.b #$7E                  ; $80D2EC
    sta.w $420C                 ; $80D2EE
    rep.b #$20                  ; $80D2F1
    lda.w #$00FF                ; $80D2F3
    sta.w $2126                 ; $80D2F6
    lda.w #$0AAA                ; $80D2F9
    sta.w $212A                 ; $80D2FC
    lda.w #$17FF                ; $80D2FF
    sta.w $0622                 ; $80D302
    lda.w #$00C6                ; $80D305
    sta.w $0624                 ; $80D308
    phb                         ; $80D30B
    ldx.w #$8452                ; $80D30C
    ldy.w #$0520                ; $80D30F
    lda.w #$005F                ; $80D312
    mvn $81,$89                 ; $80D315
    ldx.w #$84B2                ; $80D318
    ldy.w #$0600                ; $80D31B
    lda.w #$001F                ; $80D31E
    mvn $81,$89                 ; $80D321
    ldx.w #$84D2                ; $80D324
    ldy.w #$0640                ; $80D327
    lda.w #$001F                ; $80D32A
    mvn $81,$89                 ; $80D32D
    plb                         ; $80D330
    lda.w #$0001                ; $80D331
    tsb.w $0026                 ; $80D334
    lda.w #$007F                ; $80D337
    sta.b $14                   ; $80D33A
    ldx.w #$AE50                ; $80D33C
    ldy.w #$5000                ; $80D33F
    lda.w #$0700                ; $80D342
    jsr.w $83FC                 ; $80D345
    lda.w #$007F                ; $80D348
    sta.b $14                   ; $80D34B
    ldx.w #$CB10                ; $80D34D
    ldy.w #$5400                ; $80D350
    lda.w #$0700                ; $80D353
    jsr.w $83FC                 ; $80D356
    lda.w #$007F                ; $80D359
    sta.b $14                   ; $80D35C
    ldx.w #$B550                ; $80D35E
    ldy.w #$0000                ; $80D361
    lda.w #$0700                ; $80D364
    jsr.w $83FC                 ; $80D367
    lda.w #$007F                ; $80D36A
    sta.b $14                   ; $80D36D
    ldx.w #$BD10                ; $80D36F
    ldy.w #$5800                ; $80D372
    lda.w #$0700                ; $80D375
    jsr.w $83FC                 ; $80D378
    lda.w #$007F                ; $80D37B
    sta.b $14                   ; $80D37E
    ldx.w #$C410                ; $80D380
    ldy.w #$5C00                ; $80D383
    lda.w #$0700                ; $80D386
    jsr.w $83FC                 ; $80D389
    lda.w #$007F                ; $80D38C
    sta.b $14                   ; $80D38F
    ldx.w #$D210                ; $80D391
    ldy.w #$3010                ; $80D394
    lda.w #$0600                ; $80D397
    jsr.w $83FC                 ; $80D39A
    lda.w #$007F                ; $80D39D
    sta.b $14                   ; $80D3A0
    ldx.w #$2000                ; $80D3A2
    ldy.w #$0400                ; $80D3A5
    lda.w #$0700                ; $80D3A8
    jsr.w $83FC                 ; $80D3AB
    lda.w #$007F                ; $80D3AE
    sta.b $14                   ; $80D3B1
    ldx.w #$1000                ; $80D3B3
    ldy.w #$6000                ; $80D3B6
    pla                         ; $80D3B9
    jsr.w $83FC                 ; $80D3BA
    lda.w #$007F                ; $80D3BD
    sta.b $14                   ; $80D3C0
    ldx.w #$2700                ; $80D3C2
    ldy.w #$1400                ; $80D3C5
    lda.w #$24E0                ; $80D3C8
    jsr.w $83FC                 ; $80D3CB
    lda.w #$007F                ; $80D3CE
    sta.b $14                   ; $80D3D1
    ldx.w #$4BE0                ; $80D3D3
    ldy.w #$7000                ; $80D3D6
    lda.w #$1A40                ; $80D3D9
    jsr.w $83FC                 ; $80D3DC
    lda.w #$007F                ; $80D3DF
    sta.b $14                   ; $80D3E2
    ldx.w #$6620                ; $80D3E4
    ldy.w #$3000                ; $80D3E7
    lda.w #$0020                ; $80D3EA
    jsr.w $83FC                 ; $80D3ED
    lda.w #$0072                ; $80D3F0
    sta.b $7A                   ; $80D3F3
    jsr.w $D6A0                 ; $80D3F5
    jsr.w $DA5F                 ; $80D3F8
    jsr.w $D823                 ; $80D3FB
    jsr.w $89D1                 ; $80D3FE
    lda.w #$0002                ; $80D401
    tsb.w $0026                 ; $80D404
    lda.w $0368                 ; $80D407
    asl a                       ; $80D40A
    tax                         ; $80D40B
    lda.w $CB14,x               ; $80D40C
    jsr.w $D6F3                 ; $80D40F
    ldx.w $0444                 ; $80D412
    lda.l $84A240,x             ; $80D415
    and.w #$00FF                ; $80D419
    sta.w $0368                 ; $80D41C
    lda.w $0028                 ; $80D41F
CODE_80D422:
    cmp.w $002A                 ; $80D422
    bne CODE_80D422             ; $80D425
    lda.w #$1017                ; $80D427
    sta.w $212C                 ; $80D42A
    lda.w #$0017                ; $80D42D
    sta.w $212E                 ; $80D430
    lda.w #$5FA6                ; $80D433
    sta.b $7C                   ; $80D436
    lda.w #$E28C                ; $80D438
    sta.w $0008                 ; $80D43B
    stz.b $16                   ; $80D43E
    lda.w $0176                 ; $80D440
    bit.w #$0040                ; $80D443
    beq CODE_80D46F             ; $80D446
    sep.b #$20                  ; $80D448
    lda.b #$FF                  ; $80D44A
    sta.w $2112                 ; $80D44C
    sta.w $2112                 ; $80D44F
    sta.w $210E                 ; $80D452
    sta.w $210E                 ; $80D455
    sta.w $2110                 ; $80D458
    sta.w $2110                 ; $80D45B
    rep.b #$20                  ; $80D45E
    lda.w #$0F00                ; $80D460
    sta.w $0198                 ; $80D463
    lda.w #$0100                ; $80D466
    sta.w $0196                 ; $80D469
    sta.w $019A                 ; $80D46C
CODE_80D46F:
    lda.w #$0001                ; $80D46F
    jsr.w $8373                 ; $80D472
    cli                         ; $80D475
    jsr.w $DA5F                 ; $80D476
    lda.b $22                   ; $80D479
    sta.b $20                   ; $80D47B
    lda.b $2A                   ; $80D47D
    sta.b $2E                   ; $80D47F
    lda.w #$8000                ; $80D481
    sta.b $14                   ; $80D484
    lda.w #$3010                ; $80D486
    sta.w $2116                 ; $80D489
    lda.w #$1902                ; $80D48C
    sta.w $4300                 ; $80D48F
    lda.w #$007E                ; $80D492
    sta.w $4304                 ; $80D495
    ldx.w #$000A                ; $80D498
    lda.w $0700,x               ; $80D49B
    beq CODE_80D4D3             ; $80D49E
CODE_80D4A0:
    clc                         ; $80D4A0
    adc.w $0702,x               ; $80D4A1
    sta.w $0702,x               ; $80D4A4
    and.w #$0F00                ; $80D4A7
    lsr a                       ; $80D4AA
    clc                         ; $80D4AB
    adc.b $14                   ; $80D4AC
    sta.w $4302                 ; $80D4AE
    lda.w #$0080                ; $80D4B1
    sta.w $4305                 ; $80D4B4
    sep.b #$20                  ; $80D4B7
    lda.b #$01                  ; $80D4B9
    sta.w $420B                 ; $80D4BB
    rep.b #$20                  ; $80D4BE
    lda.w #$0800                ; $80D4C0
    clc                         ; $80D4C3
    adc.b $14                   ; $80D4C4
    sta.b $14                   ; $80D4C6
    txa                         ; $80D4C8
    clc                         ; $80D4C9
    adc.w #$000A                ; $80D4CA
    tax                         ; $80D4CD
    lda.w $0700,x               ; $80D4CE
    bne CODE_80D4A0             ; $80D4D1
CODE_80D4D3:
    lda.w $0700                 ; $80D4D3
    beq CODE_80D520             ; $80D4D6
    incbin !baserom:$54D8..$5520
CODE_80D520:
    ldx.w $0444                 ; $80D520
    lda.l $81C7B8,x             ; $80D523
    and.w #$00FF                ; $80D527
    sta.b $2C                   ; $80D52A
    tax                         ; $80D52C
    lda.w $0806,x               ; $80D52D
    tax                         ; $80D530
    lda.w $C7E8,x               ; $80D531
    sta.b $32                   ; $80D534
    ldx.b $2C                   ; $80D536
    lda.w $0804,x               ; $80D538
    sec                         ; $80D53B
    sbc.b $2A                   ; $80D53C
    cmp.w #$0008                ; $80D53E
    bpl CODE_80D54D             ; $80D541
    cmp.w #$FFF8                ; $80D543
    bpl CODE_80D550             ; $80D546
    incbin !baserom:$5548..$554D
CODE_80D54D:
    lda.w #$0008                ; $80D54D
CODE_80D550:
    cmp.w #$8000                ; $80D550
    ror a                       ; $80D553
    bmi CODE_80D559             ; $80D554
    adc.w #$0000                ; $80D556
CODE_80D559:
    clc                         ; $80D559
    adc.b $2A                   ; $80D55A
    sta.b $2A                   ; $80D55C
    lda.b $2A                   ; $80D55E
    sec                         ; $80D560
    sbc.b $20                   ; $80D561
    sec                         ; $80D563
    sbc.w #$0080                ; $80D564
    bpl CODE_80D57D             ; $80D567
    cmp.w #$FFFD                ; $80D569
    bpl CODE_80D592             ; $80D56C
    lda.b $22                   ; $80D56E
    beq CODE_80D592             ; $80D570
    incbin !baserom:$5572..$557D
CODE_80D57D:
    cmp.w #$0003                ; $80D57D
    bmi CODE_80D592             ; $80D580
    lda.b $22                   ; $80D582
    cmp.w #$00FF                ; $80D584
    beq CODE_80D592             ; $80D587
    clc                         ; $80D589
    adc.w #$0003                ; $80D58A
    sta.b $22                   ; $80D58D
    jsr.w $DACF                 ; $80D58F
CODE_80D592:
    ldx.w #$0000                ; $80D592
CODE_80D595:
    lda.b $2E                   ; $80D595
    cmp.w $0884,x               ; $80D597
    beq CODE_80D5A9             ; $80D59A
    bcc CODE_80D5B3             ; $80D59C
    inx                         ; $80D59E
    inx                         ; $80D59F
    inx                         ; $80D5A0
    inx                         ; $80D5A1
    cpx.w $0880                 ; $80D5A2
    bne CODE_80D595             ; $80D5A5
    incbin !baserom:$55A7..$55A9
CODE_80D5A9:
    ldy.w $0882,x               ; $80D5A9
    lda.w $CAEE,y               ; $80D5AC
    sta.b $1E                   ; $80D5AF
    bra CODE_80D5F7             ; $80D5B1
CODE_80D5B3:
    lda.b $2E                   ; $80D5B3
    sec                         ; $80D5B5
    sbc.w $0880,x               ; $80D5B6
    xba                         ; $80D5B9
    sta.w $4204                 ; $80D5BA
    lda.w $0884,x               ; $80D5BD
    sec                         ; $80D5C0
    sbc.w $0880,x               ; $80D5C1
    sep.b #$20                  ; $80D5C4
    sta.w $4206                 ; $80D5C6
    rep.b #$20                  ; $80D5C9
    nop                         ; $80D5CB
    nop                         ; $80D5CC
    xba                         ; $80D5CD
    xba                         ; $80D5CE
    xba                         ; $80D5CF
    lda.w $4214                 ; $80D5D0
    sep.b #$20                  ; $80D5D3
    sta.w $211B                 ; $80D5D5
    xba                         ; $80D5D8
    sta.w $211B                 ; $80D5D9
    ldy.w $0882,x               ; $80D5DC
    lda.w $CAEE,y               ; $80D5DF
    ldy.w $087E,x               ; $80D5E2
    sec                         ; $80D5E5
    sbc.w $CAEE,y               ; $80D5E6
    sta.w $211C                 ; $80D5E9
    rep.b #$20                  ; $80D5EC
    lda.w $CAEE,y               ; $80D5EE
    clc                         ; $80D5F1
    adc.w $2135                 ; $80D5F2
    sta.b $1E                   ; $80D5F5
CODE_80D5F7:
    lda.w #$C0C0                ; $80D5F7
    trb.w $0012                 ; $80D5FA
    bne CODE_80D604             ; $80D5FD
    trb.w $000E                 ; $80D5FF
    beq CODE_80D611             ; $80D602
CODE_80D604:
    stz.w $0198                 ; $80D604
    lda.w #$FF80                ; $80D607
    sta.w $0196                 ; $80D60A
    jsl.l $80E4DB               ; $80D60D
CODE_80D611:
    lda.b $30                   ; $80D611
    cmp.w #$000D                ; $80D613
    bcs CODE_80D62C             ; $80D616
    lda.b $26                   ; $80D618
    beq CODE_80D62C             ; $80D61A
    lda.b $34                   ; $80D61C
    clc                         ; $80D61E
    adc.b $38                   ; $80D61F
    sta.b $34                   ; $80D621
    lda.b $36                   ; $80D623
    clc                         ; $80D625
    adc.b $3A                   ; $80D626
    sta.b $36                   ; $80D628
    dec.b $26                   ; $80D62A
CODE_80D62C:
    jsr.w $D823                 ; $80D62C
    jsr.w $89D1                 ; $80D62F
    lda.w #$0002                ; $80D632
    tsb.w $0026                 ; $80D635
    dec.b $30                   ; $80D638
    bpl CODE_80D64C             ; $80D63A
    lda.w #$000C                ; $80D63C
    sta.b $30                   ; $80D63F
    sep.b #$20                  ; $80D641
    lda.b $37                   ; $80D643
    xba                         ; $80D645
    lda.b $35                   ; $80D646
    rep.b #$20                  ; $80D648
    sta.b $24                   ; $80D64A
CODE_80D64C:
    lda.b $30                   ; $80D64C
    cmp.w #$000C                ; $80D64E
    bcc CODE_80D656             ; $80D651
    lda.w #$0000                ; $80D653
CODE_80D656:
    asl a                       ; $80D656
    tax                         ; $80D657
    lda.w $CAFA,x               ; $80D658
    sta.b $7C                   ; $80D65B
    lda.w $019A                 ; $80D65D
    and.w #$0F00                ; $80D660
    beq CODE_80D668             ; $80D663
    jmp.w $D46F                 ; $80D665
CODE_80D668:
    sei                         ; $80D668
    stz.w $019A                 ; $80D669
    lda.w #$89B2                ; $80D66C
    sta.w $0005                 ; $80D66F
    stz.w $420B                 ; $80D672
    lda.w #$8934                ; $80D675
    sta.w $0008                 ; $80D678
    stz.w $0196                 ; $80D67B
    ldx.b $2C                   ; $80D67E
    lda.w $0808,x               ; $80D680
    ora.w $036A                 ; $80D683
    sta.w $036A                 ; $80D686
    ldy.w #$0000                ; $80D689
    lda.w #$0001                ; $80D68C
CODE_80D68F:
    bit.w $0808,x               ; $80D68F
    bne CODE_80D698             ; $80D692
    asl a                       ; $80D694
    iny                         ; $80D695
    bra CODE_80D68F             ; $80D696
CODE_80D698:
    sep.b #$10                  ; $80D698
    sty.w $036B                 ; $80D69A
    rep.b #$10                  ; $80D69D
    rts                         ; $80D69F
    stz.b $28                   ; $80D6A0
    stz.b $34                   ; $80D6A2
    stz.b $36                   ; $80D6A4
    stz.b $24                   ; $80D6A6
    stz.b $1E                   ; $80D6A8
    stz.b $2C                   ; $80D6AA
    lda.w #$0028                ; $80D6AC
    sta.b $30                   ; $80D6AF
    lda.w #$04B0                ; $80D6B1
    sta.b $32                   ; $80D6B4
    stz.b $38                   ; $80D6B6
    stz.b $3A                   ; $80D6B8
    lda.w #$0050                ; $80D6BA
    sta.b $3C                   ; $80D6BD
    lda.w $0804                 ; $80D6BF
    sta.b $2A                   ; $80D6C2
    sta.b $2E                   ; $80D6C4
    sec                         ; $80D6C6
    sbc.w #$0080                ; $80D6C7
    bcs CODE_80D6CF             ; $80D6CA
    lda.w #$0000                ; $80D6CC
CODE_80D6CF:
    cmp.w #$0100                ; $80D6CF
    bcc CODE_80D6D7             ; $80D6D2
    incbin !baserom:$56D4..$56D7
CODE_80D6D7:
    sta.b $22                   ; $80D6D7
    sta.w $4204                 ; $80D6D9
    sep.b #$20                  ; $80D6DC
    lda.b #$03                  ; $80D6DE
    sta.w $4206                 ; $80D6E0
    rep.b #$20                  ; $80D6E3
    xba                         ; $80D6E5
    xba                         ; $80D6E6
    xba                         ; $80D6E7
    lda.b $22                   ; $80D6E8
    sec                         ; $80D6EA
    sbc.w $4216                 ; $80D6EB
    sta.b $22                   ; $80D6EE
    sta.b $20                   ; $80D6F0
    rts                         ; $80D6F2
    sta.b $26                   ; $80D6F3
    and.w #$00FF                ; $80D6F5
    xba                         ; $80D6F8
    sec                         ; $80D6F9
    sbc.b $34                   ; $80D6FA
    sta.b $16                   ; $80D6FC
    bpl CODE_80D704             ; $80D6FE
    incbin !baserom:$5700..$5704
CODE_80D704:
    sta.b $14                   ; $80D704
    lda.b $26                   ; $80D706
    and.w #$FF00                ; $80D708
    sec                         ; $80D70B
    sbc.b $36                   ; $80D70C
    sta.b $18                   ; $80D70E
    bpl CODE_80D716             ; $80D710
    incbin !baserom:$5712..$5716
CODE_80D716:
    cmp.b $14                   ; $80D716
    bmi CODE_80D742             ; $80D718
    xba                         ; $80D71A
    and.w #$00FF                ; $80D71B
    sta.b $26                   ; $80D71E
    lda.b $14                   ; $80D720
    and.w #$FF00                ; $80D722
    sta.w $4204                 ; $80D725
    sep.b #$20                  ; $80D728
    lda.b $26                   ; $80D72A
    sta.w $4206                 ; $80D72C
    rep.b #$20                  ; $80D72F
    nop                         ; $80D731
    nop                         ; $80D732
    xba                         ; $80D733
    xba                         ; $80D734
    xba                         ; $80D735
    lda.w $4214                 ; $80D736
    sta.b $38                   ; $80D739
    lda.w #$0100                ; $80D73B
    sta.b $3A                   ; $80D73E
    bra CODE_80D763             ; $80D740
CODE_80D742:
    sta.w $4204                 ; $80D742
    lda.b $14                   ; $80D745
    xba                         ; $80D747
    and.w #$00FF                ; $80D748
    sta.b $26                   ; $80D74B
    sep.b #$20                  ; $80D74D
    sta.w $4206                 ; $80D74F
    rep.b #$20                  ; $80D752
    nop                         ; $80D754
    nop                         ; $80D755
    xba                         ; $80D756
    xba                         ; $80D757
    xba                         ; $80D758
    lda.w $4214                 ; $80D759
    sta.b $3A                   ; $80D75C
    lda.w #$0100                ; $80D75E
    sta.b $38                   ; $80D761
CODE_80D763:
    lda.b $16                   ; $80D763
    bpl CODE_80D76F             ; $80D765
    incbin !baserom:$5767..$576F
CODE_80D76F:
    lda.b $18                   ; $80D76F
    bpl CODE_80D77B             ; $80D771
    incbin !baserom:$5773..$577B
CODE_80D77B:
    sep.b #$20                  ; $80D77B
    sta.b $34                   ; $80D77D
    sta.b $36                   ; $80D77F
    rep.b #$20                  ; $80D781
    rts                         ; $80D783
    phb                         ; $80D784
    ldx.w #$C207                ; $80D785
    ldy.w #$1B00                ; $80D788
    lda.w #$0006                ; $80D78B
    mvn $81,$81                 ; $80D78E
    plb                         ; $80D791
    ldy.w #$1801                ; $80D792
    sty.w $4300                 ; $80D795
    ldy.w #$0000                ; $80D798
    sty.w $4302                 ; $80D79B
    lda.w #$0800                ; $80D79E
    sta.w $4305                 ; $80D7A1
    stz.w $2116                 ; $80D7A4
    sep.b #$20                  ; $80D7A7
    lda.b #$7F                  ; $80D7A9
    sta.w $4304                 ; $80D7AB
    lda.b #$01                  ; $80D7AE
    sta.w $420B                 ; $80D7B0
    rep.b #$20                  ; $80D7B3
    ldy.w #$1801                ; $80D7B5
    sty.w $4300                 ; $80D7B8
    ldy.w #$0800                ; $80D7BB
    sty.w $4302                 ; $80D7BE
    lda.w #$0800                ; $80D7C1
    sta.w $4305                 ; $80D7C4
    lda.w #$0800                ; $80D7C7
    sta.w $2116                 ; $80D7CA
    sep.b #$20                  ; $80D7CD
    lda.b #$7F                  ; $80D7CF
    sta.w $4304                 ; $80D7D1
    lda.b #$01                  ; $80D7D4
    sta.w $420B                 ; $80D7D6
    rep.b #$20                  ; $80D7D9
    sep.b #$20                  ; $80D7DB
    lda.b #$02                  ; $80D7DD
    sta.w $212C                 ; $80D7DF
    lda.b #$03                  ; $80D7E2
    sta.w $2108                 ; $80D7E4
    lda.b #$51                  ; $80D7E7
    sta.w $2107                 ; $80D7E9
    lda.b #$59                  ; $80D7EC
    sta.w $2109                 ; $80D7EE
    lda.b #$09                  ; $80D7F1
    sta.w $2105                 ; $80D7F3
    lda.b #$03                  ; $80D7F6
    sta.w $2101                 ; $80D7F8
    lda.b #$03                  ; $80D7FB
    sta.w $210C                 ; $80D7FD
    stz.w $2111                 ; $80D800
    stz.w $2111                 ; $80D803
    lda.b #$FF                  ; $80D806
    sta.w $2112                 ; $80D808
    sta.w $2112                 ; $80D80B
    lda.b #$02                  ; $80D80E
    sta.w $2130                 ; $80D810
    lda.b #$02                  ; $80D813
    sta.w $2121                 ; $80D815
    stz.w $2122                 ; $80D818
    stz.w $2122                 ; $80D81B
    rep.b #$20                  ; $80D81E
    jmp.w $8934                 ; $80D820
    ldx.w #$0000                ; $80D823
    sep.b #$20                  ; $80D826
    lda.b $37                   ; $80D828
    xba                         ; $80D82A
    lda.b $35                   ; $80D82B
    rep.b #$20                  ; $80D82D
    jsr.w $DB7D                 ; $80D82F
    ldx.w #$0100                ; $80D832
    lda.b $24                   ; $80D835
    jsr.w $DBCB                 ; $80D837
    lda.w #$0000                ; $80D83A
    sec                         ; $80D83D
    sbc.b $22                   ; $80D83E
    sta.b $14                   ; $80D840
    cmp.w #$0100                ; $80D842
    bpl CODE_80D85C             ; $80D845
    cmp.w #$FFF0                ; $80D847
    bmi CODE_80D85C             ; $80D84A
    lda.w #$0098                ; $80D84C
    sta.b $16                   ; $80D84F
    stz.b $18                   ; $80D851
    ldx.w #$C272                ; $80D853
    lda.w #$0081                ; $80D856
    jsr.w $B49E                 ; $80D859
CODE_80D85C:
    lda.w #$2200                ; $80D85C
    sta.b $14                   ; $80D85F
    lda.b $28                   ; $80D861
    jsr.w $86C9                 ; $80D863
    ldy.w #$4846                ; $80D866
    jsr.w $8667                 ; $80D869
    lda.w #$2200                ; $80D86C
    sta.b $14                   ; $80D86F
    sep.b #$20                  ; $80D871
    lda.b $28                   ; $80D873
    sta.w $211B                 ; $80D875
    lda.b $29                   ; $80D878
    sta.w $211B                 ; $80D87A
    lda.b #$4D                  ; $80D87D
    sta.w $211C                 ; $80D87F
    lda.w $2134                 ; $80D882
    asl a                       ; $80D885
    sta.b $17                   ; $80D886
    rep.b #$20                  ; $80D888
    lda.w $2135                 ; $80D88A
    rol a                       ; $80D88D
    asl.b $16                   ; $80D88E
    adc.b $28                   ; $80D890
    jsr.w $86C9                 ; $80D892
    ldy.w #$5046                ; $80D895
    jsr.w $8667                 ; $80D898
    lda.w #$2200                ; $80D89B
    sta.b $14                   ; $80D89E
    lda.b $34                   ; $80D8A0
    and.w #$FF00                ; $80D8A2
    sep.b #$20                  ; $80D8A5
    sta.w $211B                 ; $80D8A7
    xba                         ; $80D8AA
    sta.w $211B                 ; $80D8AB
    lda.b #$63                  ; $80D8AE
    sta.w $211C                 ; $80D8B0
    rep.b #$20                  ; $80D8B3
    lda.w $2135                 ; $80D8B5
    jsr.w $86C9                 ; $80D8B8
    ldy.w #$2CD2                ; $80D8BB
    jsr.w $84F0                 ; $80D8BE
    lda.w #$2200                ; $80D8C1
    sta.b $14                   ; $80D8C4
    lda.b $36                   ; $80D8C6
    and.w #$FF00                ; $80D8C8
    sep.b #$20                  ; $80D8CB
    sta.w $211B                 ; $80D8CD
    xba                         ; $80D8D0
    sta.w $211B                 ; $80D8D1
    lda.b #$41                  ; $80D8D4
    sta.w $211C                 ; $80D8D6
    lda.w $2134                 ; $80D8D9
    asl a                       ; $80D8DC
    rep.b #$20                  ; $80D8DD
    lda.w $2135                 ; $80D8DF
    rol a                       ; $80D8E2
    jsr.w $86C9                 ; $80D8E3
    ldy.w #$7662                ; $80D8E6
    jsr.w $8698                 ; $80D8E9
    lda.w $0368                 ; $80D8EC
    asl a                       ; $80D8EF
    tax                         ; $80D8F0
    lda.l $86CEA7,x             ; $80D8F1
    tax                         ; $80D8F5
    lda.w #$2200                ; $80D8F6
    sta.b $14                   ; $80D8F9
    lda.w #$0086                ; $80D8FB
    ldy.w #$BDA8                ; $80D8FE
    jsr.w $84BE                 ; $80D901
    lda.w $0444                 ; $80D904
    asl a                       ; $80D907
    tax                         ; $80D908
    lda.l $86CEBF,x             ; $80D909
    tax                         ; $80D90D
    lda.w #$2200                ; $80D90E
    sta.b $14                   ; $80D911
    lda.w #$0086                ; $80D913
    ldy.w #$CDA8                ; $80D916
    jsr.w $84BE                 ; $80D919
    lda.w $0444                 ; $80D91C
    asl a                       ; $80D91F
    tax                         ; $80D920
    lda.l $86D0A7,x             ; $80D921
    tax                         ; $80D925
    lda.w #$2200                ; $80D926
    sta.b $14                   ; $80D929
    lda.w #$0086                ; $80D92B
    ldy.w #$6832                ; $80D92E
    jsr.w $84BE                 ; $80D931
    jsr.w $DAED                 ; $80D934
    lda.b $32                   ; $80D937
    sec                         ; $80D939
    sbc.b $28                   ; $80D93A
    cmp.w #$0010                ; $80D93C
    bpl CODE_80D94B             ; $80D93F
    cmp.w #$FFF0                ; $80D941
    bpl CODE_80D94E             ; $80D944
    incbin !baserom:$5946..$594B
CODE_80D94B:
    lda.w #$0010                ; $80D94B
CODE_80D94E:
    clc                         ; $80D94E
    adc.b $28                   ; $80D94F
    sta.b $28                   ; $80D951
    stz.b $1C                   ; $80D953
    ldx.w #$000A                ; $80D955
    lda.w $070A                 ; $80D958
    beq CODE_80D991             ; $80D95B
CODE_80D95D:
    lda.w $0706,x               ; $80D95D
    sec                         ; $80D960
    sbc.b $22                   ; $80D961
    sta.b $14                   ; $80D963
    cmp.w #$0100                ; $80D965
    bpl CODE_80D983             ; $80D968
    cmp.w #$FFE0                ; $80D96A
    bmi CODE_80D983             ; $80D96D
    lda.w #$0098                ; $80D96F
    sta.b $16                   ; $80D972
    lda.w $0708,x               ; $80D974
    sta.b $18                   ; $80D977
    lda.w $0704,x               ; $80D979
    tax                         ; $80D97C
    lda.w #$0081                ; $80D97D
    jsr.w $B49E                 ; $80D980
CODE_80D983:
    lda.b $1C                   ; $80D983
    clc                         ; $80D985
    adc.w #$000A                ; $80D986
    sta.b $1C                   ; $80D989
    tax                         ; $80D98B
    lda.w $0700,x               ; $80D98C
    bne CODE_80D95D             ; $80D98F
CODE_80D991:
    lda.w $0700                 ; $80D991
    beq CODE_80D9BC             ; $80D994
    incbin !baserom:$5996..$59BC
CODE_80D9BC:
    ldy.w $0174                 ; $80D9BC
    ldx.w #$0000                ; $80D9BF
CODE_80D9C2:
    cpy.w #$01FC                ; $80D9C2
    bcs CODE_80DA18             ; $80D9C5
    lda.w $0F06,x               ; $80D9C7
    and.w #$FF80                ; $80D9CA
    cmp.w #$8000                ; $80D9CD
    rol a                       ; $80D9D0
    xba                         ; $80D9D1
    sec                         ; $80D9D2
    sbc.w #$0008                ; $80D9D3
    bit.w #$FF00                ; $80D9D6
    beq CODE_80D9DE             ; $80D9D9
    ora.w #$FF00                ; $80D9DB
CODE_80D9DE:
    sta.w $0B00,y               ; $80D9DE
    sta.w $0B04,y               ; $80D9E1
    and.w #$00FF                ; $80D9E4
    ora.w $0F02,x               ; $80D9E7
    sta.w $0900,y               ; $80D9EA
    clc                         ; $80D9ED
    adc.w #$0800                ; $80D9EE
    sta.w $0904,y               ; $80D9F1
    lda.w $0F04,x               ; $80D9F4
    sta.w $0902,y               ; $80D9F7
    inc a                       ; $80D9FA
    sta.w $0906,y               ; $80D9FB
    lda.w #$0000                ; $80D9FE
    sta.w $0B02,y               ; $80DA01
    sta.w $0B06,y               ; $80DA04
    tya                         ; $80DA07
    clc                         ; $80DA08
    adc.w #$0008                ; $80DA09
    tay                         ; $80DA0C
    txa                         ; $80DA0D
    clc                         ; $80DA0E
    adc.w #$0008                ; $80DA0F
    tax                         ; $80DA12
    cpx.w #$0010                ; $80DA13
    bne CODE_80D9C2             ; $80DA16
CODE_80DA18:
    cpy.w #$0200                ; $80DA18
    bcs CODE_80DA5B             ; $80DA1B
    lda.w $0F06,x               ; $80DA1D
    and.w #$FF80                ; $80DA20
    cmp.w #$8000                ; $80DA23
    rol a                       ; $80DA26
    xba                         ; $80DA27
    sec                         ; $80DA28
    sbc.w #$0008                ; $80DA29
    bit.w #$FF00                ; $80DA2C
    beq CODE_80DA34             ; $80DA2F
    ora.w #$FF00                ; $80DA31
CODE_80DA34:
    sta.w $0B00,y               ; $80DA34
    and.w #$00FF                ; $80DA37
    ora.w $0F02,x               ; $80DA3A
    sta.w $0900,y               ; $80DA3D
    lda.w $0F04,x               ; $80DA40
    sta.w $0902,y               ; $80DA43
    lda.w #$0000                ; $80DA46
    sta.w $0B02,y               ; $80DA49
    iny                         ; $80DA4C
    iny                         ; $80DA4D
    iny                         ; $80DA4E
    iny                         ; $80DA4F
    txa                         ; $80DA50
    clc                         ; $80DA51
    adc.w #$0008                ; $80DA52
    tax                         ; $80DA55
    cpx.w #$0200                ; $80DA56
    bne CODE_80DA18             ; $80DA59
CODE_80DA5B:
    sty.w $0174                 ; $80DA5B
    rts                         ; $80DA5E
    sep.b #$20                  ; $80DA5F
    lda.b $28                   ; $80DA61
    sta.w $211B                 ; $80DA63
    lda.b $29                   ; $80DA66
    sta.w $211B                 ; $80DA68
    lda.b #$0A                  ; $80DA6B
    sta.w $211C                 ; $80DA6D
    rep.b #$20                  ; $80DA70
    lda.w $2135                 ; $80DA72
    inc a                       ; $80DA75
    inc a                       ; $80DA76
    sta.b $14                   ; $80DA77
    ldy.w #$000A                ; $80DA79
    ldx.w #$00C2                ; $80DA7C
    stx.w $2116                 ; $80DA7F
    lsr a                       ; $80DA82
    lsr a                       ; $80DA83
    lsr a                       ; $80DA84
    beq CODE_80DA92             ; $80DA85
    tax                         ; $80DA87
    lda.w #$0847                ; $80DA88
CODE_80DA8B:
    sta.w $2118                 ; $80DA8B
    dey                         ; $80DA8E
    dex                         ; $80DA8F
    bne CODE_80DA8B             ; $80DA90
CODE_80DA92:
    lda.b $14                   ; $80DA92
    and.w #$0007                ; $80DA94
    beq CODE_80DAA1             ; $80DA97
    clc                         ; $80DA99
    adc.w #$0840                ; $80DA9A
    sta.w $2118                 ; $80DA9D
    dey                         ; $80DAA0
CODE_80DAA1:
    tya                         ; $80DAA1
    beq CODE_80DAAD             ; $80DAA2
    lda.w #$0848                ; $80DAA4
CODE_80DAA7:
    sta.w $2118                 ; $80DAA7
    dey                         ; $80DAAA
    bne CODE_80DAA7             ; $80DAAB
CODE_80DAAD:
    rts                         ; $80DAAD
    incbin !baserom:$5AAE..$5ACF
    ldx.w #$0000                ; $80DACF
CODE_80DAD2:
    lda.w $0F06,x               ; $80DAD2
    sec                         ; $80DAD5
    sbc.w $0F00,x               ; $80DAD6
    bcs CODE_80DADE             ; $80DAD9
    adc.w #$8800                ; $80DADB
CODE_80DADE:
    sta.w $0F06,x               ; $80DADE
    txa                         ; $80DAE1
    clc                         ; $80DAE2
    adc.w #$0008                ; $80DAE3
    tax                         ; $80DAE6
    cpx.w #$0200                ; $80DAE7
    bne CODE_80DAD2             ; $80DAEA
    rts                         ; $80DAEC
    stz.b $14                   ; $80DAED
    ldy.b $74                   ; $80DAEF
CODE_80DAF1:
    lda.b $14                   ; $80DAF1
    cmp.w $0368                 ; $80DAF3
    bcs CODE_80DB16             ; $80DAF6
    asl a                       ; $80DAF8
    tax                         ; $80DAF9
CODE_80DAFA:
    lda.w $CB2C,x               ; $80DAFA
    tax                         ; $80DAFD
    lda.w $CB44,x               ; $80DAFE
    sta.w $0900,y               ; $80DB01
    lda.w $CB46,x               ; $80DB04
    sta.w $0902,y               ; $80DB07
    lda.b $14                   ; $80DB0A
    beq CODE_80DB11             ; $80DB0C
    lda.w #$0100                ; $80DB0E
CODE_80DB11:
    sta.w $0B01,y               ; $80DB11
    bra CODE_80DB30             ; $80DB14
CODE_80DB16:
    beq CODE_80DB40             ; $80DB16
    asl a                       ; $80DB18
    tax                         ; $80DB19
    lda.w $CB14,x               ; $80DB1A
    clc                         ; $80DB1D
    adc.w #$3068                ; $80DB1E
    sta.w $0900,y               ; $80DB21
    lda.w #$25C2                ; $80DB24
    sta.w $0902,y               ; $80DB27
    lda.w #$0000                ; $80DB2A
    sta.w $0B01,y               ; $80DB2D
CODE_80DB30:
    iny                         ; $80DB30
    iny                         ; $80DB31
    iny                         ; $80DB32
    iny                         ; $80DB33
    inc.b $14                   ; $80DB34
    lda.b $14                   ; $80DB36
    cmp.w #$000C                ; $80DB38
    bne CODE_80DAF1             ; $80DB3B
    sty.b $74                   ; $80DB3D
    rts                         ; $80DB3F
CODE_80DB40:
    asl a                       ; $80DB40
    tax                         ; $80DB41
    lda.b $3C                   ; $80DB42
    beq CODE_80DAFA             ; $80DB44
    lda.b $26                   ; $80DB46
    bne CODE_80DB65             ; $80DB48
    lda.b $3C                   ; $80DB4A
    dec.b $3C                   ; $80DB4C
    bne CODE_80DB60             ; $80DB4E
    phx                         ; $80DB50
    lda.b $14                   ; $80DB51
    pha                         ; $80DB53
    lda.w $CB14,x               ; $80DB54
    jsr.w $D6F3                 ; $80DB57
    pla                         ; $80DB5A
    sta.b $14                   ; $80DB5B
    plx                         ; $80DB5D
    lda.b $3C                   ; $80DB5E
CODE_80DB60:
    bit.w #$0010                ; $80DB60
    beq CODE_80DAFA             ; $80DB63
CODE_80DB65:
    lda.w $CB14,x               ; $80DB65
    clc                         ; $80DB68
    adc.w #$3068                ; $80DB69
    sta.w $0900,y               ; $80DB6C
    lda.w #$25C3                ; $80DB6F
    sta.w $0902,y               ; $80DB72
    lda.w #$0000                ; $80DB75
    sta.w $0B01,y               ; $80DB78
    bra CODE_80DB30             ; $80DB7B
    pea.w $817F                 ; $80DB7D
    plb                         ; $80DB80
    jsr.w $DC5A                 ; $80DB81
    txa                         ; $80DB84
    clc                         ; $80DB85
    adc.w #$0033                ; $80DB86
    tay                         ; $80DB89
    phy                         ; $80DB8A
    lda.w #$001A                ; $80DB8B
    sta.b $14                   ; $80DB8E
    lda.b $1E                   ; $80DB90
    and.w #$FFFE                ; $80DB92
    tax                         ; $80DB95
    lda.l $81C852,x             ; $80DB96
    tax                         ; $80DB9A
    lda.b $1E                   ; $80DB9B
    lsr a                       ; $80DB9D
    clc                         ; $80DB9E
    adc.w #$000C                ; $80DB9F
    sta.b $16                   ; $80DBA2
CODE_80DBA4:
    sep.b #$20                  ; $80DBA4
    lda.l $81C872,x             ; $80DBA6
    lsr a                       ; $80DBAA
    clc                         ; $80DBAB
    adc.b $2E                   ; $80DBAC
    sec                         ; $80DBAE
    sbc.b $20                   ; $80DBAF
    sta.w $0001,y               ; $80DBB1
    sec                         ; $80DBB4
    sbc.l $81C872,x             ; $80DBB5
    inc a                       ; $80DBB9
    sta.w $0000,y               ; $80DBBA
    iny                         ; $80DBBD
    iny                         ; $80DBBE
    inx                         ; $80DBBF
    rep.b #$20                  ; $80DBC0
    dec.b $14                   ; $80DBC2
    dec.b $16                   ; $80DBC4
    bne CODE_80DBA4             ; $80DBC6
    jmp.w $DC19                 ; $80DBC8
    pea.w $817F                 ; $80DBCB
    plb                         ; $80DBCE
    jsr.w $DC5A                 ; $80DBCF
    txa                         ; $80DBD2
    clc                         ; $80DBD3
    adc.w #$0033                ; $80DBD4
    tay                         ; $80DBD7
    phy                         ; $80DBD8
    lda.w #$001A                ; $80DBD9
    sta.b $14                   ; $80DBDC
    lda.b $1E                   ; $80DBDE
    and.w #$FFFE                ; $80DBE0
    tax                         ; $80DBE3
    lda.l $81C852,x             ; $80DBE4
    tax                         ; $80DBE8
    lda.b $1E                   ; $80DBE9
    lsr a                       ; $80DBEB
    clc                         ; $80DBEC
    adc.w #$000C                ; $80DBED
    sta.b $16                   ; $80DBF0
CODE_80DBF2:
    sep.b #$20                  ; $80DBF2
    lda.l $81C9AA,x             ; $80DBF4
    lsr a                       ; $80DBF8
    clc                         ; $80DBF9
    adc.b $2E                   ; $80DBFA
    sec                         ; $80DBFC
    sbc.b $20                   ; $80DBFD
    sta.w $0001,y               ; $80DBFF
    sec                         ; $80DC02
    sbc.l $81C9AA,x             ; $80DC03
    inc a                       ; $80DC07
    sta.w $0000,y               ; $80DC08
    iny                         ; $80DC0B
    iny                         ; $80DC0C
    inx                         ; $80DC0D
    rep.b #$20                  ; $80DC0E
    dec.b $14                   ; $80DC10
    dec.b $16                   ; $80DC12
    bne CODE_80DBF2             ; $80DC14
    jmp.w $DC19                 ; $80DC16
    ldx.b $14                   ; $80DC19
    beq CODE_80DC28             ; $80DC1B
    lda.w #$00FF                ; $80DC1D
CODE_80DC20:
    sta.w $0000,y               ; $80DC20
    iny                         ; $80DC23
    iny                         ; $80DC24
    dex                         ; $80DC25
    bne CODE_80DC20             ; $80DC26
CODE_80DC28:
    lda.w #$00FF                ; $80DC28
    sta.w $0000,y               ; $80DC2B
    lda.w #$0000                ; $80DC2E
    sta.w $0002,y               ; $80DC31
    ply                         ; $80DC34
    tyx                         ; $80DC35
    lda.w #$0019                ; $80DC36
    sta.b $14                   ; $80DC39
    lda.b $1E                   ; $80DC3B
    lsr a                       ; $80DC3D
    bcc CODE_80DC4A             ; $80DC3E
    dex                         ; $80DC40
    dex                         ; $80DC41
    lda.w $0000,y               ; $80DC42
    sta.w $0000,x               ; $80DC45
    dec.b $14                   ; $80DC48
CODE_80DC4A:
    dex                         ; $80DC4A
    dex                         ; $80DC4B
    iny                         ; $80DC4C
    iny                         ; $80DC4D
    lda.w $0000,y               ; $80DC4E
    sta.w $0000,x               ; $80DC51
    dec.b $14                   ; $80DC54
    bne CODE_80DC4A             ; $80DC56
    plb                         ; $80DC58
    rts                         ; $80DC59
    sta.b $14                   ; $80DC5A
    and.w #$00FF                ; $80DC5C
    clc                         ; $80DC5F
    adc.w #$0069                ; $80DC60
    sta.b $16                   ; $80DC63
    lda.w #$FF31                ; $80DC65
    sta.w $0000,x               ; $80DC68
    lda.w #$00FF                ; $80DC6B
    sta.w $0001,x               ; $80DC6E
    inx                         ; $80DC71
    inx                         ; $80DC72
    inx                         ; $80DC73
    lda.b $15                   ; $80DC74
    and.w #$00FF                ; $80DC76
    sta.b $18                   ; $80DC79
    lda.b $15                   ; $80DC7B
    and.w #$00FF                ; $80DC7D
    beq CODE_80DC90             ; $80DC80
    sta.w $0000,x               ; $80DC82
    lda.b $16                   ; $80DC85
    xba                         ; $80DC87
    ora.b $16                   ; $80DC88
    sta.w $0001,x               ; $80DC8A
    inx                         ; $80DC8D
    inx                         ; $80DC8E
    inx                         ; $80DC8F
CODE_80DC90:
    lda.w #$0001                ; $80DC90
    sta.w $0000,x               ; $80DC93
    lda.b $16                   ; $80DC96
    xba                         ; $80DC98
    ora.w #$0069                ; $80DC99
    sta.w $0001,x               ; $80DC9C
    inx                         ; $80DC9F
    inx                         ; $80DCA0
    inx                         ; $80DCA1
    lda.w #$004D                ; $80DCA2
    sec                         ; $80DCA5
    sbc.b $18                   ; $80DCA6
    sta.w $0000,x               ; $80DCA8
    lda.w #$00FF                ; $80DCAB
    sta.w $0001,x               ; $80DCAE
    inx                         ; $80DCB1
    inx                         ; $80DCB2
    inx                         ; $80DCB3
    lda.w #$00B4                ; $80DCB4
    sta.w $0000,x               ; $80DCB7
    rts                         ; $80DCBA
    ldx.w $0444                 ; $80DCBB
    lda.l $84A240,x             ; $80DCBE
    and.w #$00FF                ; $80DCC2
    asl a                       ; $80DCC5
    tay                         ; $80DCC6
    lda.w $C3D0,y               ; $80DCC7
    tay                         ; $80DCCA
    lda.w #$8000                ; $80DCCB
    sta.b $7C                   ; $80DCCE
    lda.w #$2002                ; $80DCD0
    sta.b $7E                   ; $80DCD3
    lda.w #$000F                ; $80DCD5
    sta.b $7A                   ; $80DCD8
    lda.w #$000A                ; $80DCDA
    sta.b $80                   ; $80DCDD
    stz.b $82                   ; $80DCDF
    stz.b $86                   ; $80DCE1
    lda.w #$0001                ; $80DCE3
    sta.b $88                   ; $80DCE6
CODE_80DCE8:
    phy                         ; $80DCE8
    ldx.b $86                   ; $80DCE9
    lda.w $C3E8,y               ; $80DCEB
    bne CODE_80DCFC             ; $80DCEE
    lda.b $82                   ; $80DCF0
    sta.w $0800                 ; $80DCF2
    lda.b $86                   ; $80DCF5
    sta.w $0880                 ; $80DCF7
    ply                         ; $80DCFA
    rts                         ; $80DCFB
CODE_80DCFC:
    and.w #$7FFF                ; $80DCFC
    sta.w $0882,x               ; $80DCFF
    tax                         ; $80DD02
    lda.w $C3EA,y               ; $80DD03
    asl a                       ; $80DD06
    asl a                       ; $80DD07
    asl a                       ; $80DD08
    clc                         ; $80DD09
    adc.w $CAE2,x               ; $80DD0A
    ldx.b $86                   ; $80DD0D
    sta.w $0884,x               ; $80DD0F
    inx                         ; $80DD12
    inx                         ; $80DD13
    inx                         ; $80DD14
    inx                         ; $80DD15
    stx.b $86                   ; $80DD16
    lda.w $C3E8,y               ; $80DD18
    bpl CODE_80DD59             ; $80DD1B
    lda.b $88                   ; $80DD1D
    and.w $036A                 ; $80DD1F
    bne CODE_80DD51             ; $80DD22
    lda.w $C3E8,y               ; $80DD24
    ldx.b $82                   ; $80DD27
    and.w #$7FFF                ; $80DD29
    sta.w $0802,x               ; $80DD2C
    tax                         ; $80DD2F
    lda.w $C3EA,y               ; $80DD30
    asl a                       ; $80DD33
    asl a                       ; $80DD34
    asl a                       ; $80DD35
    clc                         ; $80DD36
    adc.w $CAE2,x               ; $80DD37
    ldx.b $82                   ; $80DD3A
    sta.w $0804,x               ; $80DD3C
    lda.w $C3F4,y               ; $80DD3F
    sta.w $0806,x               ; $80DD42
    lda.b $88                   ; $80DD45
    sta.w $0808,x               ; $80DD47
    txa                         ; $80DD4A
    clc                         ; $80DD4B
    adc.w #$0008                ; $80DD4C
    sta.b $82                   ; $80DD4F
CODE_80DD51:
    asl.b $88                   ; $80DD51
    lda.w $C3E8,y               ; $80DD53
    and.w #$7FFF                ; $80DD56
CODE_80DD59:
    tax                         ; $80DD59
    jmp.w ($DD65,x)             ; $80DD5A
    pla                         ; $80DD5D
    clc                         ; $80DD5E
    adc.w #$000E                ; $80DD5F
    tay                         ; $80DD62
    bra CODE_80DCE8             ; $80DD63
    incbin !baserom:$5D65..$5E4D
    ldx.w #$BC50                ; $80DE4D
    lda.b $7A                   ; $80DE50
    bit.w #$0001                ; $80DE52
    beq CODE_80DE5A             ; $80DE55
    ldx.w #$BC70                ; $80DE57
CODE_80DE5A:
    asl a                       ; $80DE5A
    and.w #$001C                ; $80DE5B
    xba                         ; $80DE5E
    sta.b $84                   ; $80DE5F
    jmp.w $DEA8                 ; $80DE61
    ldx.w #$BC90                ; $80DE64
    lda.b $7A                   ; $80DE67
    bit.w #$0001                ; $80DE69
    beq CODE_80DE71             ; $80DE6C
    ldx.w #$BCB0                ; $80DE6E
CODE_80DE71:
    asl a                       ; $80DE71
    and.w #$001C                ; $80DE72
    xba                         ; $80DE75
    sta.b $84                   ; $80DE76
    bra CODE_80DEA8             ; $80DE78
    ldx.w #$BCD0                ; $80DE7A
    lda.b $7A                   ; $80DE7D
    bit.w #$0001                ; $80DE7F
    beq CODE_80DE87             ; $80DE82
    ldx.w #$BCE8                ; $80DE84
CODE_80DE87:
    asl a                       ; $80DE87
    and.w #$001C                ; $80DE88
    xba                         ; $80DE8B
    sta.b $84                   ; $80DE8C
    jmp.w $DF38                 ; $80DE8E
    ldx.w #$BD00                ; $80DE91
    lda.b $7A                   ; $80DE94
    bit.w #$0001                ; $80DE96
    beq CODE_80DE9E             ; $80DE99
    ldx.w #$BD08                ; $80DE9B
CODE_80DE9E:
    asl a                       ; $80DE9E
    and.w #$001C                ; $80DE9F
    xba                         ; $80DEA2
    sta.b $84                   ; $80DEA3
    jmp.w $DFA8                 ; $80DEA5
CODE_80DEA8:
    lda.w $C3EA,y               ; $80DEA8
    jsr.w $E0C8                 ; $80DEAB
    tay                         ; $80DEAE
    pea.w $817F                 ; $80DEAF
    plb                         ; $80DEB2
    clc                         ; $80DEB3
    lda.w $0000,x               ; $80DEB4
    adc.b $84                   ; $80DEB7
    sta.w $0000,y               ; $80DEB9
    lda.w $0002,x               ; $80DEBC
    adc.b $84                   ; $80DEBF
    sta.w $0002,y               ; $80DEC1
    lda.w $0004,x               ; $80DEC4
    adc.b $84                   ; $80DEC7
    sta.w $0004,y               ; $80DEC9
    lda.w $0006,x               ; $80DECC
    adc.b $84                   ; $80DECF
    sta.w $0006,y               ; $80DED1
    lda.w $0008,x               ; $80DED4
    adc.b $84                   ; $80DED7
    sta.w $0040,y               ; $80DED9
    lda.w $000A,x               ; $80DEDC
    adc.b $84                   ; $80DEDF
    sta.w $0042,y               ; $80DEE1
    lda.w $000C,x               ; $80DEE4
    adc.b $84                   ; $80DEE7
    sta.w $0044,y               ; $80DEE9
    lda.w $000E,x               ; $80DEEC
    adc.b $84                   ; $80DEEF
    sta.w $0046,y               ; $80DEF1
    lda.w $0010,x               ; $80DEF4
    adc.b $84                   ; $80DEF7
    sta.w $0080,y               ; $80DEF9
    lda.w $0012,x               ; $80DEFC
    adc.b $84                   ; $80DEFF
    sta.w $0082,y               ; $80DF01
    lda.w $0014,x               ; $80DF04
    adc.b $84                   ; $80DF07
    sta.w $0084,y               ; $80DF09
    lda.w $0016,x               ; $80DF0C
    adc.b $84                   ; $80DF0F
    sta.w $0086,y               ; $80DF11
    lda.w $0018,x               ; $80DF14
    adc.b $84                   ; $80DF17
    sta.w $00C0,y               ; $80DF19
    lda.w $001A,x               ; $80DF1C
    adc.b $84                   ; $80DF1F
    sta.w $00C2,y               ; $80DF21
    lda.w $001C,x               ; $80DF24
    adc.b $84                   ; $80DF27
    sta.w $00C4,y               ; $80DF29
    lda.w $001E,x               ; $80DF2C
    adc.b $84                   ; $80DF2F
    sta.w $00C6,y               ; $80DF31
    plb                         ; $80DF34
    jmp.w $DFDC                 ; $80DF35
    lda.w $C3EA,y               ; $80DF38
    jsr.w $E0C8                 ; $80DF3B
    tay                         ; $80DF3E
    pea.w $817F                 ; $80DF3F
    plb                         ; $80DF42
    clc                         ; $80DF43
    lda.w $0000,x               ; $80DF44
    adc.b $84                   ; $80DF47
    sta.w $0000,y               ; $80DF49
    lda.w $0002,x               ; $80DF4C
    adc.b $84                   ; $80DF4F
    sta.w $0002,y               ; $80DF51
    lda.w $0004,x               ; $80DF54
    adc.b $84                   ; $80DF57
    sta.w $0004,y               ; $80DF59
    lda.w $0006,x               ; $80DF5C
    adc.b $84                   ; $80DF5F
    sta.w $0040,y               ; $80DF61
    lda.w $0008,x               ; $80DF64
    adc.b $84                   ; $80DF67
    sta.w $0042,y               ; $80DF69
    lda.w $000A,x               ; $80DF6C
    adc.b $84                   ; $80DF6F
    sta.w $0044,y               ; $80DF71
    lda.w $000C,x               ; $80DF74
    adc.b $84                   ; $80DF77
    sta.w $0080,y               ; $80DF79
    lda.w $000E,x               ; $80DF7C
    adc.b $84                   ; $80DF7F
    sta.w $0082,y               ; $80DF81
    lda.w $0010,x               ; $80DF84
    adc.b $84                   ; $80DF87
    sta.w $0084,y               ; $80DF89
    lda.w $0012,x               ; $80DF8C
    adc.b $84                   ; $80DF8F
    sta.w $00C0,y               ; $80DF91
    lda.w $0014,x               ; $80DF94
    adc.b $84                   ; $80DF97
    sta.w $00C2,y               ; $80DF99
    lda.w $0016,x               ; $80DF9C
    adc.b $84                   ; $80DF9F
    sta.w $00C4,y               ; $80DFA1
    plb                         ; $80DFA4
    jmp.w $DFDC                 ; $80DFA5
    lda.w $C3EA,y               ; $80DFA8
    jsr.w $E0C8                 ; $80DFAB
    clc                         ; $80DFAE
    adc.w #$0040                ; $80DFAF
    tay                         ; $80DFB2
    pea.w $817F                 ; $80DFB3
    plb                         ; $80DFB6
    clc                         ; $80DFB7
    lda.w $0000,x               ; $80DFB8
    adc.b $84                   ; $80DFBB
    sta.w $0000,y               ; $80DFBD
    lda.w $0002,x               ; $80DFC0
    adc.b $84                   ; $80DFC3
    sta.w $0002,y               ; $80DFC5
    lda.w $0004,x               ; $80DFC8
    adc.b $84                   ; $80DFCB
    sta.w $0040,y               ; $80DFCD
    lda.w $0006,x               ; $80DFD0
    adc.b $84                   ; $80DFD3
    sta.w $0042,y               ; $80DFD5
    plb                         ; $80DFD8
    jmp.w $DFDC                 ; $80DFD9
    lda.b $01,s                 ; $80DFDC
    tay                         ; $80DFDE
    lda.w $C3EC,y               ; $80DFDF
    tax                         ; $80DFE2
    phb                         ; $80DFE3
    lda.b $7A                   ; $80DFE4
    asl a                       ; $80DFE6
    asl a                       ; $80DFE7
    asl a                       ; $80DFE8
    asl a                       ; $80DFE9
    clc                         ; $80DFEA
    adc.w #$0500                ; $80DFEB
    tay                         ; $80DFEE
    sta.b $84                   ; $80DFEF
    lda.w #$000F                ; $80DFF1
    mvn $81,$86                 ; $80DFF4
    plb                         ; $80DFF7
    lda.b $01,s                 ; $80DFF8
    tay                         ; $80DFFA
    lda.w $C3F2,y               ; $80DFFB
    tax                         ; $80DFFE
    phb                         ; $80DFFF
    lda.b $84                   ; $80E000
    clc                         ; $80E002
    adc.w #$0100                ; $80E003
    tay                         ; $80E006
    lda.w #$000F                ; $80E007
    mvn $81,$86                 ; $80E00A
    plb                         ; $80E00D
    lda.b $01,s                 ; $80E00E
    tay                         ; $80E010
    ldx.w $C3EE,y               ; $80E011
    ldy.b $7C                   ; $80E014
    lda.w #$7E91                ; $80E016
    jsr.w $FBA0                 ; $80E019
    clc                         ; $80E01C
    adc.b $7C                   ; $80E01D
    sta.b $7C                   ; $80E01F
    lda.b $01,s                 ; $80E021
    tay                         ; $80E023
    lda.w $C3EA,y               ; $80E024
    asl a                       ; $80E027
    cmp.w #$0040                ; $80E028
    bcs CODE_80E032             ; $80E02B
    adc.w #$1310                ; $80E02D
    bra CODE_80E035             ; $80E030
CODE_80E032:
    adc.w #$19CF                ; $80E032
CODE_80E035:
    tax                         ; $80E035
    lda.b $7E                   ; $80E036
    sta.l $7FAE40,x             ; $80E038
    inc a                       ; $80E03C
    sta.l $7FAE42,x             ; $80E03D
    inc a                       ; $80E041
    sta.l $7FAE44,x             ; $80E042
    inc a                       ; $80E046
    sta.l $7FAE46,x             ; $80E047
    inc a                       ; $80E04B
    sta.l $7FAE80,x             ; $80E04C
    inc a                       ; $80E050
    sta.l $7FAE82,x             ; $80E051
    inc a                       ; $80E055
    sta.l $7FAE84,x             ; $80E056
    inc a                       ; $80E05A
    sta.l $7FAE86,x             ; $80E05B
    inc a                       ; $80E05F
    sta.l $7FAEC0,x             ; $80E060
    inc a                       ; $80E064
    sta.l $7FAEC2,x             ; $80E065
    inc a                       ; $80E069
    sta.l $7FAEC4,x             ; $80E06A
    inc a                       ; $80E06E
    sta.l $7FAEC6,x             ; $80E06F
    inc a                       ; $80E073
    sta.l $7FAF00,x             ; $80E074
    inc a                       ; $80E078
    sta.l $7FAF02,x             ; $80E079
    inc a                       ; $80E07D
    sta.l $7FAF04,x             ; $80E07E
    inc a                       ; $80E082
    sta.l $7FAF06,x             ; $80E083
    inc a                       ; $80E087
    sta.b $7E                   ; $80E088
    lda.w $C3F0,y               ; $80E08A
    ldx.b $80                   ; $80E08D
    sta.w $0700,x               ; $80E08F
    stz.w $0702,x               ; $80E092
    lda.b $7A                   ; $80E095
    and.w #$0001                ; $80E097
    ora.w $C3E8,y               ; $80E09A
    asl a                       ; $80E09D
    tax                         ; $80E09E
    lda.w $C2DA,x               ; $80E09F
    ldx.b $80                   ; $80E0A2
    sta.w $0704,x               ; $80E0A4
    lda.w $C3EA,y               ; $80E0A7
    asl a                       ; $80E0AA
    asl a                       ; $80E0AB
    asl a                       ; $80E0AC
    sta.w $0706,x               ; $80E0AD
    lda.b $7A                   ; $80E0B0
    xba                         ; $80E0B2
    and.w #$FE00                ; $80E0B3
    sta.w $0708,x               ; $80E0B6
    stz.w $070A,x               ; $80E0B9
    txa                         ; $80E0BC
    clc                         ; $80E0BD
    adc.w #$000A                ; $80E0BE
    sta.b $80                   ; $80E0C1
    dec.b $7A                   ; $80E0C3
    jmp.w $DD5D                 ; $80E0C5
    asl a                       ; $80E0C8
    cmp.w #$0040                ; $80E0C9
    bcs CODE_80E0D2             ; $80E0CC
    adc.w #$B990                ; $80E0CE
    rts                         ; $80E0D1
CODE_80E0D2:
    adc.w #$23FF                ; $80E0D2
    rts                         ; $80E0D5
    ldx.w #$0000                ; $80E0D6
CODE_80E0D9:
    lda.w #$0100                ; $80E0D9
    sta.w $0F00,x               ; $80E0DC
    jsr.w $E166                 ; $80E0DF
    lda.w #$018C                ; $80E0E2
    sta.w $0F04,x               ; $80E0E5
    jsr.w $87E9                 ; $80E0E8
    and.w #$7FFF                ; $80E0EB
    sta.w $0F06,x               ; $80E0EE
    txa                         ; $80E0F1
    clc                         ; $80E0F2
    adc.w #$0008                ; $80E0F3
    tax                         ; $80E0F6
    cpx.w #$0010                ; $80E0F7
    bne CODE_80E0D9             ; $80E0FA
CODE_80E0FC:
    lda.w #$0080                ; $80E0FC
    sta.w $0F00,x               ; $80E0FF
    jsr.w $E166                 ; $80E102
    lda.w #$018E                ; $80E105
    sta.w $0F04,x               ; $80E108
    jsr.w $87E9                 ; $80E10B
    and.w #$7FFF                ; $80E10E
    sta.w $0F06,x               ; $80E111
    txa                         ; $80E114
    clc                         ; $80E115
    adc.w #$0008                ; $80E116
    tax                         ; $80E119
    cpx.w #$0060                ; $80E11A
    bne CODE_80E0FC             ; $80E11D
CODE_80E11F:
    lda.w #$0040                ; $80E11F
    sta.w $0F00,x               ; $80E122
    jsr.w $E166                 ; $80E125
    lda.w #$018F                ; $80E128
    sta.w $0F04,x               ; $80E12B
    jsr.w $87E9                 ; $80E12E
    and.w #$7FFF                ; $80E131
    sta.w $0F06,x               ; $80E134
    txa                         ; $80E137
    clc                         ; $80E138
    adc.w #$0008                ; $80E139
    tax                         ; $80E13C
    cpx.w #$0120                ; $80E13D
    bne CODE_80E11F             ; $80E140
CODE_80E142:
    lda.w #$0020                ; $80E142
    sta.w $0F00,x               ; $80E145
    jsr.w $E166                 ; $80E148
    lda.w #$019A                ; $80E14B
    sta.w $0F04,x               ; $80E14E
    jsr.w $87E9                 ; $80E151
    and.w #$7FFF                ; $80E154
    sta.w $0F06,x               ; $80E157
    txa                         ; $80E15A
    clc                         ; $80E15B
    adc.w #$0008                ; $80E15C
    tax                         ; $80E15F
    cpx.w #$0200                ; $80E160
    bne CODE_80E142             ; $80E163
    rts                         ; $80E165
CODE_80E166:
    jsr.w $87E9                 ; $80E166
    cmp.w #$DC00                ; $80E169
    bcs CODE_80E166             ; $80E16C
    and.w #$FF00                ; $80E16E
    sta.w $0F02,x               ; $80E171
    rts                         ; $80E174
    phd                         ; $80E175
    lda.w #$0100                ; $80E176
    tcd                         ; $80E179
    lda.b $16                   ; $80E17A
    beq CODE_80E182             ; $80E17C
    pld                         ; $80E17E
    jmp.w $8934                 ; $80E17F
CODE_80E182:
    ldx.w #$0000                ; $80E182
    lda.b $14                   ; $80E185
    clc                         ; $80E187
    adc.w #$0038                ; $80E188
    sta.w $1B00,x               ; $80E18B
    inx                         ; $80E18E
    lda.w #$FFDF                ; $80E18F
    sec                         ; $80E192
    sbc.b $14                   ; $80E193
    sta.w $1B00,x               ; $80E195
    inx                         ; $80E198
    inx                         ; $80E199
    lda.w #$0034                ; $80E19A
    sec                         ; $80E19D
    sbc.b $14                   ; $80E19E
    beq CODE_80E1FB             ; $80E1A0
    bmi CODE_80E1FB             ; $80E1A2
    sta.w $1B00,x               ; $80E1A4
    inx                         ; $80E1A7
    lda.w #$001F                ; $80E1A8
    sta.w $1B00,x               ; $80E1AB
    inx                         ; $80E1AE
    inx                         ; $80E1AF
    lda.w #$0088                ; $80E1B0
    sta.w $1B00,x               ; $80E1B3
    lda.w #$FFEB                ; $80E1B6
    sta.w $1B01,x               ; $80E1B9
    dec a                       ; $80E1BC
    sta.w $1B03,x               ; $80E1BD
    dec a                       ; $80E1C0
    sta.w $1B05,x               ; $80E1C1
    dec a                       ; $80E1C4
    sta.w $1B07,x               ; $80E1C5
    dec a                       ; $80E1C8
    sta.w $1B09,x               ; $80E1C9
    dec a                       ; $80E1CC
    sta.w $1B0B,x               ; $80E1CD
    dec a                       ; $80E1D0
    sta.w $1B0D,x               ; $80E1D1
    dec a                       ; $80E1D4
    sta.w $1B0F,x               ; $80E1D5
    lda.w #$0038                ; $80E1D8
    sec                         ; $80E1DB
    sbc.b $14                   ; $80E1DC
    sta.w $1B11,x               ; $80E1DE
    lda.w #$FF6B                ; $80E1E1
    sta.w $1B12,x               ; $80E1E4
    lda.w #$0001                ; $80E1E7
    sta.w $1B14,x               ; $80E1EA
    lda.w #$FFAB                ; $80E1ED
    clc                         ; $80E1F0
    adc.b $14                   ; $80E1F1
    sta.w $1B15,x               ; $80E1F3
    stz.w $1B17,x               ; $80E1F6
    bra CODE_80E268             ; $80E1F9
CODE_80E1FB:
    clc                         ; $80E1FB
    adc.w #$0008                ; $80E1FC
    beq CODE_80E21B             ; $80E1FF
    tay                         ; $80E201
    stx.b $18                   ; $80E202
    ora.w #$0080                ; $80E204
    sta.w $1B00,x               ; $80E207
    inx                         ; $80E20A
    and.w #$007F                ; $80E20B
    clc                         ; $80E20E
    adc.w #$FFE3                ; $80E20F
CODE_80E212:
    sta.w $1B00,x               ; $80E212
    dec a                       ; $80E215
    inx                         ; $80E216
    inx                         ; $80E217
    dey                         ; $80E218
    bne CODE_80E212             ; $80E219
CODE_80E21B:
    lda.w #$0038                ; $80E21B
    sec                         ; $80E21E
    sbc.b $14                   ; $80E21F
    beq CODE_80E225             ; $80E221
    bpl CODE_80E24A             ; $80E223
CODE_80E225:
    sta.b $1A                   ; $80E225
    asl a                       ; $80E227
    sta.b $16                   ; $80E228
    txa                         ; $80E22A
    clc                         ; $80E22B
    adc.b $16                   ; $80E22C
    pha                         ; $80E22E
    ldx.b $18                   ; $80E22F
    sep.b #$20                  ; $80E231
    lda.w $1B00,x               ; $80E233
    clc                         ; $80E236
    adc.b $1A                   ; $80E237
    sta.w $1B00,x               ; $80E239
    cmp.b #$80                  ; $80E23C
    bne CODE_80E245             ; $80E23E
    lda.b $01,s                 ; $80E240
    dec a                       ; $80E242
    sta.b $01,s                 ; $80E243
CODE_80E245:
    rep.b #$20                  ; $80E245
    plx                         ; $80E247
    bra CODE_80E256             ; $80E248
CODE_80E24A:
    sta.w $1B00,x               ; $80E24A
    inx                         ; $80E24D
    lda.w #$FF6B                ; $80E24E
    sta.w $1B00,x               ; $80E251
    inx                         ; $80E254
    inx                         ; $80E255
CODE_80E256:
    lda.w #$0001                ; $80E256
    sta.w $1B00,x               ; $80E259
    lda.w #$FFAB                ; $80E25C
    clc                         ; $80E25F
    adc.b $14                   ; $80E260
    sta.w $1B01,x               ; $80E262
    stz.w $1B03,x               ; $80E265
CODE_80E268:
    lda.b $14                   ; $80E268
    cmp.w #$003A                ; $80E26A
    beq CODE_80E283             ; $80E26D
    clc                         ; $80E26F
    adc.w #$0004                ; $80E270
    cmp.w #$003A                ; $80E273
    bcc CODE_80E27B             ; $80E276
    lda.w #$003A                ; $80E278
CODE_80E27B:
    sta.b $14                   ; $80E27B
    stz.b $16                   ; $80E27D
    pld                         ; $80E27F
    jmp.w $8934                 ; $80E280
CODE_80E283:
    lda.w #$0001                ; $80E283
    sta.b $16                   ; $80E286
    pld                         ; $80E288
    jmp.w $8934                 ; $80E289
    lda.w $017A                 ; $80E28C
    sec                         ; $80E28F
    sbc.w #$0004                ; $80E290
    cmp.w #$0000                ; $80E293
    bpl CODE_80E29B             ; $80E296
    lda.w #$0000                ; $80E298
CODE_80E29B:
    sta.w $017A                 ; $80E29B
    ldx.w #$0000                ; $80E29E
    lda.w $017A                 ; $80E2A1
    beq CODE_80E2A8             ; $80E2A4
    bpl CODE_80E2CB             ; $80E2A6
CODE_80E2A8:
    lda.w #$0072                ; $80E2A8
    sta.w $1B00                 ; $80E2AB
    sta.w $0D00                 ; $80E2AE
    sta.w $0D28                 ; $80E2B1
    lda.w #$FFFF                ; $80E2B4
    sta.w $1B01                 ; $80E2B7
    lda.w #$0009                ; $80E2BA
    sta.w $0D01                 ; $80E2BD
    lda.w #$0017                ; $80E2C0
    sta.w $0D29                 ; $80E2C3
    ldx.w #$0003                ; $80E2C6
    bra CODE_80E311             ; $80E2C9
CODE_80E2CB:
    sta.w $1B00                 ; $80E2CB
    sta.w $0D00                 ; $80E2CE
    sta.w $0D28                 ; $80E2D1
    lda.w #$FFFF                ; $80E2D4
    sec                         ; $80E2D7
    sbc.w $017A                 ; $80E2D8
    sta.w $1B01                 ; $80E2DB
    lda.w #$0001                ; $80E2DE
    sta.w $0D01                 ; $80E2E1
    lda.w #$0000                ; $80E2E4
    sta.w $0D29                 ; $80E2E7
    lda.w #$0072                ; $80E2EA
    sec                         ; $80E2ED
    sbc.w $017A                 ; $80E2EE
    beq CODE_80E33B             ; $80E2F1
    sta.w $1B03                 ; $80E2F3
    sta.w $0D03                 ; $80E2F6
    sta.w $0D2B                 ; $80E2F9
    lda.w #$FFFF                ; $80E2FC
    sta.w $1B04                 ; $80E2FF
    lda.w #$0009                ; $80E302
    sta.w $0D04                 ; $80E305
    lda.w #$0017                ; $80E308
    sta.w $0D2C                 ; $80E30B
    ldx.w #$0006                ; $80E30E
CODE_80E311:
    lda.w #$0072                ; $80E311
    sec                         ; $80E314
    sbc.w $017A                 ; $80E315
    cmp.w #$006E                ; $80E318
    bcc CODE_80E320             ; $80E31B
    lda.w #$0000                ; $80E31D
CODE_80E320:
    sta.w $1B00,x               ; $80E320
    sta.w $0D00,x               ; $80E323
    sta.w $0D28,x               ; $80E326
    lda.w #$FFFF                ; $80E329
    sta.w $1B01,x               ; $80E32C
    lda.w #$0009                ; $80E32F
    sta.w $0D01,x               ; $80E332
    lda.w #$0017                ; $80E335
    sta.w $0D29,x               ; $80E338
CODE_80E33B:
    lda.w #$0001                ; $80E33B
    sta.w $1B03,x               ; $80E33E
    sta.w $0D03,x               ; $80E341
    sta.w $0D2B,x               ; $80E344
    lda.w #$FFFB                ; $80E347
    clc                         ; $80E34A
    adc.w $017A                 ; $80E34B
    sta.w $1B04,x               ; $80E34E
    lda.w #$0001                ; $80E351
    sta.w $0D04,x               ; $80E354
    lda.w #$0000                ; $80E357
    sta.w $0D2C,x               ; $80E35A
    stz.w $1B06,x               ; $80E35D
    stz.w $0D06,x               ; $80E360
    stz.w $0D2E,x               ; $80E363
    lda.w #$0072                ; $80E366
    sta.w $0D14                 ; $80E369
    stz.w $0D15                 ; $80E36C
    lda.w #$0072                ; $80E36F
    sec                         ; $80E372
    sbc.w $017A                 ; $80E373
    sta.w $0D17                 ; $80E376
    stz.w $0D18                 ; $80E379
    lda.w #$0001                ; $80E37C
    sta.w $0D1A                 ; $80E37F
    stz.w $0D1B                 ; $80E382
    stz.w $0D1D                 ; $80E385
    sep.b #$20                  ; $80E388
    lda.b #$B1                  ; $80E38A
    sta.w $4200                 ; $80E38C
    rep.b #$20                  ; $80E38F
    lda.w #$007E                ; $80E391
    sta.w $4209                 ; $80E394
    lda.w #$0010                ; $80E397
    sta.w $4207                 ; $80E39A
    lda.w #$8000                ; $80E39D
    sta.w $0006                 ; $80E3A0
    lda.w #$E3F7                ; $80E3A3
    sta.w $0005                 ; $80E3A6
    sep.b #$20                  ; $80E3A9
    stz.w $210D                 ; $80E3AB
    stz.w $210D                 ; $80E3AE
    stz.w $210F                 ; $80E3B1
    stz.w $210F                 ; $80E3B4
    stz.w $2111                 ; $80E3B7
    stz.w $2111                 ; $80E3BA
    stz.w $2121                 ; $80E3BD
    lda.b #$E6                  ; $80E3C0
    sta.w $2122                 ; $80E3C2
    lda.b #$1B                  ; $80E3C5
    sta.w $2122                 ; $80E3C7
    lda.b #$17                  ; $80E3CA
    sta.w $2131                 ; $80E3CC
    lda.w $017C                 ; $80E3CF
    sta.w $2132                 ; $80E3D2
    lda.w $017D                 ; $80E3D5
    sta.w $2132                 ; $80E3D8
    lda.b #$12                  ; $80E3DB
    sta.w $2130                 ; $80E3DD
    rep.b #$20                  ; $80E3E0
    lda.w #$0222                ; $80E3E2
    sta.w $2123                 ; $80E3E5
    lda.w #$8202                ; $80E3E8
    sta.w $2124                 ; $80E3EB
    lda.w #$0017                ; $80E3EE
    sta.w $212C                 ; $80E3F1
    jmp.w $8934                 ; $80E3F4
    rep.b #$30                  ; $80E3F7
    phb                         ; $80E3F9
    pha                         ; $80E3FA
    phx                         ; $80E3FB
    phy                         ; $80E3FC
    phk                         ; $80E3FD
    plb                         ; $80E3FE
    lda.w $4211                 ; $80E3FF
    lda.w $017A                 ; $80E402
    cmp.w #$0064                ; $80E405
    bcs CODE_80E46B             ; $80E408
    sep.b #$10                  ; $80E40A
    ldx.w $0120                 ; $80E40C
    ldy.w $0121                 ; $80E40F
    lda.w #$0100                ; $80E412
    sta.w $4207                 ; $80E415
    lda.w #$1017                ; $80E418
    wai                         ; $80E41B
    stx.w $210F                 ; $80E41C
    sty.w $210F                 ; $80E41F
    stx.w $210D                 ; $80E422
    sty.w $210D                 ; $80E425
    stx.w $2111                 ; $80E428
    sty.w $2111                 ; $80E42B
    sta.w $212C                 ; $80E42E
    ldx.b #$05                  ; $80E431
    stx.w $2131                 ; $80E433
    ldx.b #$00                  ; $80E436
    stx.w $2121                 ; $80E438
    ldx.b #$C6                  ; $80E43B
    stx.w $2122                 ; $80E43D
    ldx.b #$7C                  ; $80E440
    stx.w $2122                 ; $80E442
    rep.b #$10                  ; $80E445
    lda.w $4211                 ; $80E447
    sep.b #$20                  ; $80E44A
    lda.b #$B1                  ; $80E44C
    sta.w $4200                 ; $80E44E
    rep.b #$20                  ; $80E451
    lda.w #$00B3                ; $80E453
    sta.w $4209                 ; $80E456
    lda.w #$0010                ; $80E459
    sta.w $4207                 ; $80E45C
    lda.w #$8000                ; $80E45F
    sta.w $0006                 ; $80E462
    lda.w #$E48A                ; $80E465
    sta.w $0005                 ; $80E468
CODE_80E46B:
    sep.b #$20                  ; $80E46B
    lda.b #$E0                  ; $80E46D
    sta.w $2132                 ; $80E46F
    lda.b #$02                  ; $80E472
    sta.w $2130                 ; $80E474
    rep.b #$20                  ; $80E477
    lda.w #$0AAA                ; $80E479
    sta.w $2123                 ; $80E47C
    lda.w #$0A0A                ; $80E47F
    sta.w $2124                 ; $80E482
    ply                         ; $80E485
    plx                         ; $80E486
    pla                         ; $80E487
    plb                         ; $80E488
    rti                         ; $80E489
    rep.b #$30                  ; $80E48A
    phb                         ; $80E48C
    pha                         ; $80E48D
    phx                         ; $80E48E
    phy                         ; $80E48F
    phk                         ; $80E490
    plb                         ; $80E491
    lda.w $4211                 ; $80E492
    sep.b #$20                  ; $80E495
    ldx.w #$0100                ; $80E497
    stx.w $4207                 ; $80E49A
    ldx.w #$0017                ; $80E49D
    wai                         ; $80E4A0
    stz.w $210F                 ; $80E4A1
    stz.w $210F                 ; $80E4A4
    stz.w $210D                 ; $80E4A7
    stz.w $210D                 ; $80E4AA
    stz.w $2111                 ; $80E4AD
    stz.w $2111                 ; $80E4B0
    stx.w $212C                 ; $80E4B3
    lda.b #$05                  ; $80E4B6
    sta.w $2131                 ; $80E4B8
    rep.b #$20                  ; $80E4BB
    lda.w $4211                 ; $80E4BD
    ply                         ; $80E4C0
    plx                         ; $80E4C1
    pla                         ; $80E4C2
    plb                         ; $80E4C3
    rti                         ; $80E4C4
    pha                         ; $80E4C5
    sep.b #$20                  ; $80E4C6
    lda.b #$11                  ; $80E4C8
    sta.b $FB                   ; $80E4CA
    rep.b #$20                  ; $80E4CC
    pla                         ; $80E4CE
    rtl                         ; $80E4CF
    incbin !baserom:$64D0..$64DB
    pha                         ; $80E4DB
    sep.b #$20                  ; $80E4DC
    lda.b #$13                  ; $80E4DE
    sta.b $FB                   ; $80E4E0
    rep.b #$20                  ; $80E4E2
    pla                         ; $80E4E4
    rtl                         ; $80E4E5
    ldx.w #$CB74                ; $80E4E6
    lda.w #$0081                ; $80E4E9
    jsr.w $81CC                 ; $80E4EC
    ldx.w #$8601                ; $80E4EF
    ldy.w #$4000                ; $80E4F2
    lda.w #$0088                ; $80E4F5
    jsr.w $8381                 ; $80E4F8
    ldx.w #$8C7D                ; $80E4FB
    ldy.w #$0000                ; $80E4FE
    lda.w #$7F88                ; $80E501
    jsr.w $FBA0                 ; $80E504
    ldy.w #$1900                ; $80E507
    sty.w $4300                 ; $80E50A
    ldy.w #$0000                ; $80E50D
    sty.w $4302                 ; $80E510
    sta.w $4305                 ; $80E513
    stz.w $2116                 ; $80E516
    sep.b #$20                  ; $80E519
    lda.b #$7F                  ; $80E51B
    sta.w $4304                 ; $80E51D
    lda.b #$01                  ; $80E520
    sta.w $420B                 ; $80E522
    rep.b #$20                  ; $80E525
    ldy.w #$1808                ; $80E527
    sty.w $4300                 ; $80E52A
    ldy.w #$CBF8                ; $80E52D
    sty.w $4302                 ; $80E530
    lda.w #$4000                ; $80E533
    sta.w $4305                 ; $80E536
    stz.w $2116                 ; $80E539
    sep.b #$20                  ; $80E53C
    stz.w $2115                 ; $80E53E
    lda.b #$81                  ; $80E541
    sta.w $4304                 ; $80E543
    lda.b #$01                  ; $80E546
    sta.w $420B                 ; $80E548
    rep.b #$20                  ; $80E54B
    ldx.w #$8000                ; $80E54D
    ldy.w #$0000                ; $80E550
    lda.w #$7F95                ; $80E553
    jsr.w $FBA0                 ; $80E556
    lda.w #$1800                ; $80E559
    sta.w $4300                 ; $80E55C
    lda.w #$0000                ; $80E55F
    sta.w $4302                 ; $80E562
    sep.b #$20                  ; $80E565
    lda.b #$7F                  ; $80E567
    sta.w $4304                 ; $80E569
    rep.b #$20                  ; $80E56C
    ldx.w #$0007                ; $80E56E
    ldy.w #$0011                ; $80E571
    lda.w #$0000                ; $80E574
CODE_80E577:
    sty.w $4305                 ; $80E577
    sta.w $2116                 ; $80E57A
    sep.b #$20                  ; $80E57D
    pha                         ; $80E57F
    lda.b #$01                  ; $80E580
    sta.w $420B                 ; $80E582
    pla                         ; $80E585
    rep.b #$20                  ; $80E586
    clc                         ; $80E588
    adc.w #$0080                ; $80E589
    dex                         ; $80E58C
    bne CODE_80E577             ; $80E58D
    sep.b #$20                  ; $80E58F
    lda.b #$80                  ; $80E591
    sta.w $2115                 ; $80E593
    rep.b #$20                  ; $80E596
    lda.w #$1B43                ; $80E598
    sta.w $4310                 ; $80E59B
    lda.w #$CBF9                ; $80E59E
    sta.w $4312                 ; $80E5A1
    lda.w #$1D43                ; $80E5A4
    sta.w $4320                 ; $80E5A7
    lda.w #$CC00                ; $80E5AA
    sta.w $4322                 ; $80E5AD
    lda.w #$1F43                ; $80E5B0
    sta.w $4330                 ; $80E5B3
    lda.w #$CC07                ; $80E5B6
    sta.w $4332                 ; $80E5B9
    lda.w #$0D43                ; $80E5BC
    sta.w $4340                 ; $80E5BF
    lda.w #$CC0E                ; $80E5C2
    sta.w $4342                 ; $80E5C5
    sep.b #$20                  ; $80E5C8
    lda.b #$81                  ; $80E5CA
    sta.w $4314                 ; $80E5CC
    stz.w $4317                 ; $80E5CF
    sta.w $4324                 ; $80E5D2
    stz.w $4327                 ; $80E5D5
    sta.w $4334                 ; $80E5D8
    stz.w $4337                 ; $80E5DB
    sta.w $4344                 ; $80E5DE
    stz.w $4347                 ; $80E5E1
    lda.b #$1E                  ; $80E5E4
    sta.w $420C                 ; $80E5E6
    rep.b #$20                  ; $80E5E9
    lda.w $03AB                 ; $80E5EB
    sta.w $0148                 ; $80E5EE
    lda.w $03DA                 ; $80E5F1
    sta.w $0248                 ; $80E5F4
    jsr.w $B7E7                 ; $80E5F7
    lda.w #$FFFE                ; $80E5FA
    sta.b $4A                   ; $80E5FD
    ldx.b $92                   ; $80E5FF
    lda.w $8B40,x               ; $80E601
    sta.b $B8                   ; $80E604
    lda.w $8B43,x               ; $80E606
    clc                         ; $80E609
    adc.w #$0004                ; $80E60A
    sta.b $B4                   ; $80E60D
    lda.w $8B45,x               ; $80E60F
    sta.b $5C                   ; $80E612
    sep.b #$20                  ; $80E614
    lda.w $8B42,x               ; $80E616
    sta.b $AD                   ; $80E619
    lda.b #$11                  ; $80E61B
    sta.b $BC                   ; $80E61D
    lda.b #$07                  ; $80E61F
    sta.b $A8                   ; $80E621
    rep.b #$20                  ; $80E623
    jsr.w $EBCE                 ; $80E625
    stz.b $7A                   ; $80E628
    stz.b $7C                   ; $80E62A
    lda.w #$0000                ; $80E62C
    sta.b $7E                   ; $80E62F
    jsr.w $EB66                 ; $80E631
    jsr.w $89D6                 ; $80E634
    phb                         ; $80E637
    lda.b $C4                   ; $80E638
    clc                         ; $80E63A
    adc.w #$0100                ; $80E63B
    tay                         ; $80E63E
    ldx.w #$9073                ; $80E63F
    lda.w #$001F                ; $80E642
    mvn $81,$9C                 ; $80E645
    ldy.b $94                   ; $80E648
    lda.w $0392,y               ; $80E64A
    and.w #$00FF                ; $80E64D
    sta.b $7A                   ; $80E650
    asl a                       ; $80E652
    asl a                       ; $80E653
    asl a                       ; $80E654
    asl a                       ; $80E655
    sec                         ; $80E656
    sbc.b $7A                   ; $80E657
    asl a                       ; $80E659
    clc                         ; $80E65A
    adc.w #$8380                ; $80E65B
    tax                         ; $80E65E
    ldy.b $C4                   ; $80E65F
    lda.l $9D9F0A               ; $80E661
    sta.w $0000,y               ; $80E665
    iny                         ; $80E668
    iny                         ; $80E669
    lda.w #$001D                ; $80E66A
    mvn $81,$89                 ; $80E66D
    lda.b $C4                   ; $80E670
    clc                         ; $80E672
    adc.w #$0120                ; $80E673
    tay                         ; $80E676
    ldx.w #$8BFB                ; $80E677
    lda.w #$001F                ; $80E67A
    mvn $81,$9F                 ; $80E67D
    plb                         ; $80E680
    ldx.w #$0000                ; $80E681
    ldy.b $C4                   ; $80E684
CODE_80E686:
    lda.l $81D2A9,x             ; $80E686
    sta.w $0122,y               ; $80E68A
    lda.l $81D2B7,x             ; $80E68D
    sta.w $0124,y               ; $80E691
    lda.w #$0000                ; $80E694
    sta.w $0126,y               ; $80E697
    tya                         ; $80E69A
    clc                         ; $80E69B
    adc.w #$0020                ; $80E69C
    tay                         ; $80E69F
    inx                         ; $80E6A0
    inx                         ; $80E6A1
    cpx.w #$000E                ; $80E6A2
    bne CODE_80E686             ; $80E6A5
    lda.b $76                   ; $80E6A7
    bit.w #$0020                ; $80E6A9
    beq CODE_80E6B1             ; $80E6AC
    jmp.w $E87A                 ; $80E6AE
CODE_80E6B1:
    lda.w #$0F00                ; $80E6B1
    sta.b $98                   ; $80E6B4
    lda.w #$0080                ; $80E6B6
    sta.b $96                   ; $80E6B9
    stz.b $80                   ; $80E6BB
    stz.b $82                   ; $80E6BD
    stz.b $84                   ; $80E6BF
    stz.b $86                   ; $80E6C1
    stz.b $22                   ; $80E6C3
    stz.b $24                   ; $80E6C5
    stz.b $26                   ; $80E6C7
    stz.b $28                   ; $80E6C9
    stz.b $2A                   ; $80E6CB
    lda.w #$0040                ; $80E6CD
    sta.b $2C                   ; $80E6D0
    lda.w #$F400                ; $80E6D2
    sta.b $80                   ; $80E6D5
    ldx.b $5E                   ; $80E6D7
    ldy.b $94                   ; $80E6D9
    lda.w $03A8,y               ; $80E6DB
    cmp.w #$0003                ; $80E6DE
    bcs CODE_80E6F7             ; $80E6E1
    asl a                       ; $80E6E3
    tay                         ; $80E6E4
    lda.w $D3C3,y               ; $80E6E5
    tay                         ; $80E6E8
    jsr.w $E8B5                 ; $80E6E9
    jsr.w $E8CE                 ; $80E6EC
    jsr.w $E8F4                 ; $80E6EF
    jsr.w $E911                 ; $80E6F2
    bra CODE_80E71C             ; $80E6F5
CODE_80E6F7:
    cmp.w #$000A                ; $80E6F7
    db $B0,$11 ; bcs            ; $80E6FA
    jsr.w $E8CE                 ; $80E6FC
    ldy.w #$018D                ; $80E6FF
    jsr.w $E8B5                 ; $80E702
    jsr.w $E8F4                 ; $80E705
    jsr.w $E911                 ; $80E708
    bra CODE_80E71C             ; $80E70B
    incbin !baserom:$670D..$671C
CODE_80E71C:
    lda.w #$0000                ; $80E71C
    sta.l $7F2000,x             ; $80E71F
    lda.w #$0110                ; $80E723
    sta.b $46                   ; $80E726
    ldx.b $94                   ; $80E728
    lda.w $03A8,x               ; $80E72A
    beq CODE_80E73A             ; $80E72D
    cmp.w #$000A                ; $80E72F
    bcc CODE_80E737             ; $80E732
    incbin !baserom:$6734..$6737
CODE_80E737:
    jmp.w $E7F9                 ; $80E737
CODE_80E73A:
    ldx.b $5E                   ; $80E73A
    lda.w #$0000                ; $80E73C
    ldy.w #$0020                ; $80E73F
CODE_80E742:
    sta.l $7F1000,x             ; $80E742
    inx                         ; $80E746
    inx                         ; $80E747
    inx                         ; $80E748
    inx                         ; $80E749
    dey                         ; $80E74A
    bne CODE_80E742             ; $80E74B
    stz.b $3E                   ; $80E74D
    lda.w #$0001                ; $80E74F
    jsr.w $8373                 ; $80E752
    ldx.b $26                   ; $80E755
    lda.b $86                   ; $80E757
    cmp.w $CC17,x               ; $80E759
    bcc CODE_80E764             ; $80E75C
    inx                         ; $80E75E
    inx                         ; $80E75F
    inx                         ; $80E760
    inx                         ; $80E761
    stx.b $26                   ; $80E762
CODE_80E764:
    lda.w $CC15,x               ; $80E764
    clc                         ; $80E767
    adc.b $22                   ; $80E768
    sta.b $22                   ; $80E76A
    ldx.b $28                   ; $80E76C
    lda.b $86                   ; $80E76E
    cmp.w $CC33,x               ; $80E770
    bcc CODE_80E77B             ; $80E773
    inx                         ; $80E775
    inx                         ; $80E776
    inx                         ; $80E777
    inx                         ; $80E778
    stx.b $28                   ; $80E779
CODE_80E77B:
    lda.w $CC31,x               ; $80E77B
    clc                         ; $80E77E
    adc.b $24                   ; $80E77F
    sta.b $24                   ; $80E781
    lda.b $86                   ; $80E783
    cmp.w #$0060                ; $80E785
    bcc CODE_80E7E9             ; $80E788
    cmp.w #$0080                ; $80E78A
    bcs CODE_80E797             ; $80E78D
    lda.b $82                   ; $80E78F
    clc                         ; $80E791
    adc.w #$0018                ; $80E792
    sta.b $82                   ; $80E795
CODE_80E797:
    lda.b $80                   ; $80E797
    bpl CODE_80E7A8             ; $80E799
    clc                         ; $80E79B
    adc.w #$0020                ; $80E79C
    bmi CODE_80E7A4             ; $80E79F
    lda.w #$0000                ; $80E7A1
CODE_80E7A4:
    sta.b $80                   ; $80E7A4
    bra CODE_80E7B3             ; $80E7A6
CODE_80E7A8:
    sec                         ; $80E7A8
    sbc.w #$0020                ; $80E7A9
    bpl CODE_80E7B1             ; $80E7AC
    lda.w #$0000                ; $80E7AE
CODE_80E7B1:
    sta.b $80                   ; $80E7B1
CODE_80E7B3:
    lda.b $2A                   ; $80E7B3
    cmp.w #$0040                ; $80E7B5
    beq CODE_80E7C0             ; $80E7B8
    clc                         ; $80E7BA
    adc.w #$0010                ; $80E7BB
    sta.b $2A                   ; $80E7BE
CODE_80E7C0:
    lda.b $2C                   ; $80E7C0
    cmp.w #$0066                ; $80E7C2
    bcs CODE_80E7CD             ; $80E7C5
    clc                         ; $80E7C7
    adc.w #$0002                ; $80E7C8
    sta.b $2C                   ; $80E7CB
CODE_80E7CD:
    lda.b $84                   ; $80E7CD
    sec                         ; $80E7CF
    sbc.w #$FE3E                ; $80E7D0
    bpl CODE_80E7D8             ; $80E7D3
    incbin !baserom:$67D5..$67D8
CODE_80E7D8:
    cmp.w #$0040                ; $80E7D8
    bmi CODE_80E7DF             ; $80E7DB
    lda.b $2A                   ; $80E7DD
CODE_80E7DF:
    lsr a                       ; $80E7DF
    lsr a                       ; $80E7E0
    eor.w #$FFFF                ; $80E7E1
    sec                         ; $80E7E4
    adc.b $84                   ; $80E7E5
    sta.b $84                   ; $80E7E7
CODE_80E7E9:
    jsr.w $E925                 ; $80E7E9
    lda.b $9A                   ; $80E7EC
    bit.w #$7FFF                ; $80E7EE
    bne CODE_80E7F6             ; $80E7F1
    jmp.w $E87A                 ; $80E7F3
CODE_80E7F6:
    jmp.w $E74F                 ; $80E7F6
    lda.w #$F000                ; $80E7F9
    sta.b $80                   ; $80E7FC
    lda.w #$0001                ; $80E7FE
    jsr.w $8373                 ; $80E801
    ldx.b $26                   ; $80E804
    lda.b $86                   ; $80E806
    cmp.w $CC73,x               ; $80E808
    bcc CODE_80E813             ; $80E80B
    inx                         ; $80E80D
    inx                         ; $80E80E
    inx                         ; $80E80F
    inx                         ; $80E810
    stx.b $26                   ; $80E811
CODE_80E813:
    lda.w $CC71,x               ; $80E813
    clc                         ; $80E816
    adc.b $22                   ; $80E817
    sta.b $22                   ; $80E819
    ldx.b $28                   ; $80E81B
    lda.b $86                   ; $80E81D
    cmp.w $CC93,x               ; $80E81F
    bcc CODE_80E82A             ; $80E822
    inx                         ; $80E824
    inx                         ; $80E825
    inx                         ; $80E826
    inx                         ; $80E827
    stx.b $28                   ; $80E828
CODE_80E82A:
    lda.w $CC91,x               ; $80E82A
    clc                         ; $80E82D
    adc.b $24                   ; $80E82E
    sta.b $24                   ; $80E830
    lda.b $86                   ; $80E832
    cmp.w #$0018                ; $80E834
    bcc CODE_80E86A             ; $80E837
    cmp.w #$00A0                ; $80E839
    bcc CODE_80E85A             ; $80E83C
    lda.b $80                   ; $80E83E
    cmp.w #$FF01                ; $80E840
    bpl CODE_80E848             ; $80E843
    lda.w #$FF01                ; $80E845
CODE_80E848:
    dec a                       ; $80E848
    eor.w #$FFFF                ; $80E849
    sec                         ; $80E84C
    adc.b $80                   ; $80E84D
    bmi CODE_80E856             ; $80E84F
    lda.w #$0000                ; $80E851
    stz.b $20                   ; $80E854
CODE_80E856:
    sta.b $80                   ; $80E856
    bra CODE_80E86A             ; $80E858
CODE_80E85A:
    lda.b $80                   ; $80E85A
    sec                         ; $80E85C
    sbc.w #$0080                ; $80E85D
    cmp.w #$E800                ; $80E860
    bpl CODE_80E868             ; $80E863
    lda.w #$E800                ; $80E865
CODE_80E868:
    sta.b $80                   ; $80E868
CODE_80E86A:
    jsr.w $E925                 ; $80E86A
    lda.b $9A                   ; $80E86D
    bit.w #$7FFF                ; $80E86F
    bne CODE_80E877             ; $80E872
    jmp.w $E87A                 ; $80E874
CODE_80E877:
    jmp.w $E7FE                 ; $80E877
    jsr.w $B950                 ; $80E87A
    sei                         ; $80E87D
    lda.w #$8934                ; $80E87E
    sta.w $0008                 ; $80E881
    lda.w #$89B2                ; $80E884
    sta.w $0005                 ; $80E887
    stz.w $420B                 ; $80E88A
    rts                         ; $80E88D
    incbin !baserom:$688E..$68B5
    lda.w $D419,y               ; $80E8B5
    sta.b $14                   ; $80E8B8
    iny                         ; $80E8BA
    iny                         ; $80E8BB
    sep.b #$20                  ; $80E8BC
CODE_80E8BE:
    lda.w $D419,y               ; $80E8BE
    sta.l $7F2000,x             ; $80E8C1
    inx                         ; $80E8C5
    iny                         ; $80E8C6
    dec.b $14                   ; $80E8C7
    bne CODE_80E8BE             ; $80E8C9
    rep.b #$20                  ; $80E8CB
    rts                         ; $80E8CD
    ldy.b $94                   ; $80E8CE
    sep.b #$20                  ; $80E8D0
CODE_80E8D2:
    lda.w $037E,y               ; $80E8D2
    beq CODE_80E8DF             ; $80E8D5
    sta.l $7F2000,x             ; $80E8D7
    inx                         ; $80E8DB
    iny                         ; $80E8DC
    bra CODE_80E8D2             ; $80E8DD
CODE_80E8DF:
    rep.b #$20                  ; $80E8DF
    lda.w #$0100                ; $80E8E1
    sta.l $7F2000,x             ; $80E8E4
    lda.w #$1400                ; $80E8E8
    sta.l $7F2002,x             ; $80E8EB
    inx                         ; $80E8EF
    inx                         ; $80E8F0
    inx                         ; $80E8F1
    inx                         ; $80E8F2
    rts                         ; $80E8F3
    ldy.b $94                   ; $80E8F4
    lda.w $03A8,y               ; $80E8F6
    bpl CODE_80E901             ; $80E8F9
    incbin !baserom:$68FB..$6901
CODE_80E901:
    asl a                       ; $80E901
    tay                         ; $80E902
    lda.w $D3C9,y               ; $80E903
    tay                         ; $80E906
    jsr.w $E8B5                 ; $80E907
    ldy.w #$00BE                ; $80E90A
    jsr.w $E8B5                 ; $80E90D
    rts                         ; $80E910
    ldy.b $94                   ; $80E911
    lda.w $03A8,y               ; $80E913
    bpl CODE_80E91B             ; $80E916
    incbin !baserom:$6918..$691B
CODE_80E91B:
    asl a                       ; $80E91B
    tay                         ; $80E91C
    lda.w $D3F1,y               ; $80E91D
    tay                         ; $80E920
    jsr.w $E8B5                 ; $80E921
    rts                         ; $80E924
    lda.b $82                   ; $80E925
    bpl CODE_80E936             ; $80E927
    incbin !baserom:$6929..$6936
CODE_80E936:
    sec                         ; $80E936
    sbc.w #$0012                ; $80E937
    bpl CODE_80E93F             ; $80E93A
    lda.w #$0000                ; $80E93C
CODE_80E93F:
    sta.b $82                   ; $80E93F
    ldx.w #$0000                ; $80E941
    lda.b $80                   ; $80E944
    bpl CODE_80E949             ; $80E946
    dex                         ; $80E948
CODE_80E949:
    clc                         ; $80E949
    adc.b $7B                   ; $80E94A
    sta.b $7B                   ; $80E94C
    sep.b #$20                  ; $80E94E
    txa                         ; $80E950
    adc.b $7D                   ; $80E951
    sta.b $7D                   ; $80E953
    rep.b #$20                  ; $80E955
    lda.b $82                   ; $80E957
    clc                         ; $80E959
    adc.b $7E                   ; $80E95A
    sta.b $7E                   ; $80E95C
    lda.w #$0002                ; $80E95E
    tsb.w $0026                 ; $80E961
    lda.b $84                   ; $80E964
    and.w #$03FE                ; $80E966
    tax                         ; $80E969
    lda.b $7C                   ; $80E96A
    cmp.w #$8000                ; $80E96C
    ror a                       ; $80E96F
    cmp.w #$8000                ; $80E970
    ror a                       ; $80E973
    cmp.w #$8000                ; $80E974
    ror a                       ; $80E977
    clc                         ; $80E978
    adc.w #$0088                ; $80E979
    sta.b $2E                   ; $80E97C
    lda.b $22                   ; $80E97E
    xba                         ; $80E980
    and.w #$00FF                ; $80E981
    bit.w #$0080                ; $80E984
    beq CODE_80E98C             ; $80E987
    ora.w #$FF00                ; $80E989
CODE_80E98C:
    clc                         ; $80E98C
    adc.b $2E                   ; $80E98D
    sta.b $2E                   ; $80E98F
    lda.b $7E                   ; $80E991
    xba                         ; $80E993
    cmp.w #$8000                ; $80E994
    rol a                       ; $80E997
    and.w #$01FF                ; $80E998
    bit.w #$0100                ; $80E99B
    beq CODE_80E9A3             ; $80E99E
    incbin !baserom:$69A0..$69A3
CODE_80E9A3:
    sec                         ; $80E9A3
    sbc.w #$002E                ; $80E9A4
    sec                         ; $80E9A7
    sbc.b $E8                   ; $80E9A8
    sta.b $30                   ; $80E9AA
    lda.b $24                   ; $80E9AC
    xba                         ; $80E9AE
    and.w #$00FF                ; $80E9AF
    bit.w #$0080                ; $80E9B2
    beq CODE_80E9BA             ; $80E9B5
    ora.w #$FF00                ; $80E9B7
CODE_80E9BA:
    clc                         ; $80E9BA
    adc.b $30                   ; $80E9BB
    sta.b $30                   ; $80E9BD
    lda.w $CEA9,x               ; $80E9BF
    sta.b $32                   ; $80E9C2
    sta.b $38                   ; $80E9C4
    lda.w $CDA9,x               ; $80E9C6
    sta.b $34                   ; $80E9C9
    eor.w #$FFFF                ; $80E9CB
    inc a                       ; $80E9CE
    sta.b $36                   ; $80E9CF
    lda.b $2C                   ; $80E9D1
    sta.b $3A                   ; $80E9D3
    lda.w #$001C                ; $80E9D5
    sta.b $3C                   ; $80E9D8
    lda.b $46                   ; $80E9DA
    bmi CODE_80E9E1             ; $80E9DC
    jmp.w $EA26                 ; $80E9DE
CODE_80E9E1:
    and.w #$0001                ; $80E9E1
    bne CODE_80EA09             ; $80E9E4
    lda.b $48                   ; $80E9E6
    bne CODE_80E9F3             ; $80E9E8
    stz.b $98                   ; $80E9EA
    lda.w #$FF80                ; $80E9EC
    sta.b $96                   ; $80E9EF
    bra CODE_80EA09             ; $80E9F1
CODE_80E9F3:
    lda.b $4A                   ; $80E9F3
    inc a                       ; $80E9F5
    inc a                       ; $80E9F6
    sta.b $4A                   ; $80E9F7
    lsr.b $48                   ; $80E9F9
    bcs CODE_80E9FF             ; $80E9FB
    bra CODE_80E9F3             ; $80E9FD
CODE_80E9FF:
    inc.b $46                   ; $80E9FF
    jsr.w $EBCE                 ; $80EA01
    lda.w #$00A0                ; $80EA04
    sta.b $4C                   ; $80EA07
CODE_80EA09:
    ldx.b $4A                   ; $80EA09
    lda.l $81D5CF,x             ; $80EA0B
    tay                         ; $80EA0F
    lda.w #$0A10                ; $80EA10
    clc                         ; $80EA13
    adc.b $E7                   ; $80EA14
    tax                         ; $80EA16
    lda.w #$0081                ; $80EA17
    jsr.w $EBF7                 ; $80EA1A
    dec.b $4C                   ; $80EA1D
    bne CODE_80EA3A             ; $80EA1F
    inc.b $46                   ; $80EA21
    jmp.w $EA3A                 ; $80EA23
    lda.w #$0A10                ; $80EA26
    clc                         ; $80EA29
    adc.b $E7                   ; $80EA2A
    tax                         ; $80EA2C
    lda.w #$2000                ; $80EA2D
    clc                         ; $80EA30
    adc.b $5E                   ; $80EA31
    tay                         ; $80EA33
    lda.w #$007F                ; $80EA34
    jsr.w $EBF7                 ; $80EA37
CODE_80EA3A:
    ldx.b $94                   ; $80EA3A
    lda.w $03A8,x               ; $80EA3C
    bne CODE_80EA44             ; $80EA3F
    jsr.w $EA77                 ; $80EA41
CODE_80EA44:
    jsr.w $EB66                 ; $80EA44
    jsr.w $89D6                 ; $80EA47
    inc.b $86                   ; $80EA4A
    lda.b $46                   ; $80EA4C
    bpl CODE_80EA62             ; $80EA4E
    cmp.w #$FFFF                ; $80EA50
    bne CODE_80EA76             ; $80EA53
    incbin !baserom:$6A55..$6A62
CODE_80EA62:
    dec.b $46                   ; $80EA62
    bne CODE_80EA76             ; $80EA64
    lda.b $48                   ; $80EA66
    db $F0,$F4 ; beq            ; $80EA68
    ldy.b $94                   ; $80EA6A
    lda.w $03A8,y               ; $80EA6C
    db $30,$ED ; bmi            ; $80EA6F
    lda.w #$8000                ; $80EA71
    sta.b $46                   ; $80EA74
CODE_80EA76:
    rts                         ; $80EA76
    lda.b $3E                   ; $80EA77
    clc                         ; $80EA79
    adc.b $5E                   ; $80EA7A
    tax                         ; $80EA7C
    lda.w #$0020                ; $80EA7D
    sta.b $14                   ; $80EA80
CODE_80EA82:
    lda.l $7F1000,x             ; $80EA82
    clc                         ; $80EA86
    lsr a                       ; $80EA87
    and.w #$FFFE                ; $80EA88
    bit.w #$000F                ; $80EA8B
    beq CODE_80EAB8             ; $80EA8E
    tay                         ; $80EA90
    lda.l $7F1000,x             ; $80EA91
    dec a                       ; $80EA95
    sta.l $7F1000,x             ; $80EA96
    lda.w $D2C3,y               ; $80EA9A
    ldy.b $74                   ; $80EA9D
    sta.w $0902,y               ; $80EA9F
    lda.l $7F1002,x             ; $80EAA2
    clc                         ; $80EAA6
    adc.b $E7                   ; $80EAA7
    sta.w $0900,y               ; $80EAA9
    lda.w #$0100                ; $80EAAC
    sta.w $0B01,y               ; $80EAAF
    iny                         ; $80EAB2
    iny                         ; $80EAB3
    iny                         ; $80EAB4
    iny                         ; $80EAB5
    sty.b $74                   ; $80EAB6
CODE_80EAB8:
    txa                         ; $80EAB8
    clc                         ; $80EAB9
    adc.w #$0004                ; $80EABA
    and.w #$807F                ; $80EABD
    tax                         ; $80EAC0
    dec.b $14                   ; $80EAC1
    bne CODE_80EA82             ; $80EAC3
    lda.b $86                   ; $80EAC5
    sec                         ; $80EAC7
    sbc.w #$0064                ; $80EAC8
    bcs CODE_80EAD0             ; $80EACB
CODE_80EACD:
    jmp.w $EB65                 ; $80EACD
CODE_80EAD0:
    cmp.w #$001E                ; $80EAD0
    bcs CODE_80EACD             ; $80EAD3
    asl a                       ; $80EAD5
    tay                         ; $80EAD6
    lda.b $3E                   ; $80EAD7
    clc                         ; $80EAD9
    adc.b $5E                   ; $80EADA
    tax                         ; $80EADC
    lda.w #$001C                ; $80EADD
    sta.l $7F1000,x             ; $80EAE0
    lda.w $D2E3,y               ; $80EAE4
    sta.l $7F1002,x             ; $80EAE7
    lda.b $3E                   ; $80EAEB
    clc                         ; $80EAED
    adc.w #$0004                ; $80EAEE
    and.w #$007F                ; $80EAF1
    sta.b $3E                   ; $80EAF4
    lda.b $3E                   ; $80EAF6
    clc                         ; $80EAF8
    adc.b $5E                   ; $80EAF9
    tax                         ; $80EAFB
    lda.w #$001C                ; $80EAFC
    sta.l $7F1000,x             ; $80EAFF
    lda.w $D2E3,y               ; $80EB03
    sec                         ; $80EB06
    sbc.w #$0808                ; $80EB07
    sta.l $7F1002,x             ; $80EB0A
    lda.b $3E                   ; $80EB0E
    clc                         ; $80EB10
    adc.w #$0004                ; $80EB11
    and.w #$007F                ; $80EB14
    sta.b $3E                   ; $80EB17
    lda.b $86                   ; $80EB19
    bit.w #$0003                ; $80EB1B
    bne CODE_80EB65             ; $80EB1E
    lda.b $3E                   ; $80EB20
    clc                         ; $80EB22
    adc.b $5E                   ; $80EB23
    tax                         ; $80EB25
    lda.w #$001C                ; $80EB26
    sta.l $7F1000,x             ; $80EB29
    lda.w $D323,y               ; $80EB2D
    sta.l $7F1002,x             ; $80EB30
    lda.b $3E                   ; $80EB34
    clc                         ; $80EB36
    adc.w #$0004                ; $80EB37
    and.w #$007F                ; $80EB3A
    sta.b $3E                   ; $80EB3D
    lda.b $86                   ; $80EB3F
    bit.w #$0003                ; $80EB41
    bne CODE_80EB65             ; $80EB44
    lda.b $3E                   ; $80EB46
    clc                         ; $80EB48
    adc.b $5E                   ; $80EB49
    tax                         ; $80EB4B
    lda.w #$003C                ; $80EB4C
    sta.l $7F1000,x             ; $80EB4F
    lda.w $D367,y               ; $80EB53
    sta.l $7F1002,x             ; $80EB56
    lda.b $3E                   ; $80EB5A
    clc                         ; $80EB5C
    adc.w #$0004                ; $80EB5D
    and.w #$007F                ; $80EB60
    sta.b $3E                   ; $80EB63
CODE_80EB65:
    rts                         ; $80EB65
    lda.b $22                   ; $80EB66
    xba                         ; $80EB68
    and.w #$00FF                ; $80EB69
    bit.w #$0080                ; $80EB6C
    beq CODE_80EB74             ; $80EB6F
    ora.w #$FF00                ; $80EB71
CODE_80EB74:
    eor.w #$FFFF                ; $80EB74
    sec                         ; $80EB77
    adc.b $7C                   ; $80EB78
    and.w #$00FF                ; $80EB7A
    sec                         ; $80EB7D
    sbc.w #$0100                ; $80EB7E
    sta.b $14                   ; $80EB81
    lda.b $7E                   ; $80EB83
    xba                         ; $80EB85
    cmp.w #$8000                ; $80EB86
    rol a                       ; $80EB89
    clc                         ; $80EB8A
    adc.w #$0024                ; $80EB8B
    and.w #$00FF                ; $80EB8E
    clc                         ; $80EB91
    adc.b $E8                   ; $80EB92
    sta.b $16                   ; $80EB94
    lda.b $24                   ; $80EB96
    xba                         ; $80EB98
    and.w #$00FF                ; $80EB99
    bit.w #$0080                ; $80EB9C
    beq CODE_80EBA4             ; $80EB9F
    ora.w #$FF00                ; $80EBA1
CODE_80EBA4:
    eor.w #$FFFF                ; $80EBA4
    sec                         ; $80EBA7
    adc.b $16                   ; $80EBA8
    sta.b $16                   ; $80EBAA
    lda.w #$F000                ; $80EBAC
    sta.b $18                   ; $80EBAF
    ldx.w #$CBB8                ; $80EBB1
    lda.w #$0081                ; $80EBB4
    jsr.w $B49E                 ; $80EBB7
    rts                         ; $80EBBA
    incbin !baserom:$6BBB..$6BCE
    stz.b $CC                   ; $80EBCE
    stz.b $C6                   ; $80EBD0
    lda.w #$0080                ; $80EBD2
    sta.b $E5                   ; $80EBD5
    lda.w #$0040                ; $80EBD7
    sta.b $E1                   ; $80EBDA
    stz.b $D1                   ; $80EBDC
    ldx.b $5E                   ; $80EBDE
    lda.w #$FFFF                ; $80EBE0
    ldy.w #$001C                ; $80EBE3
CODE_80EBE6:
    sta.l $7F0000,x             ; $80EBE6
    inx                         ; $80EBEA
    inx                         ; $80EBEB
    dey                         ; $80EBEC
    bne CODE_80EBE6             ; $80EBED
    lda.w #$FFFE                ; $80EBEF
    sta.l $7F0000,x             ; $80EBF2
    rts                         ; $80EBF6
    phb                         ; $80EBF7
    sep.b #$20                  ; $80EBF8
    pha                         ; $80EBFA
    plb                         ; $80EBFB
    rep.b #$20                  ; $80EBFC
    phy                         ; $80EBFE
    stx.b $C8                   ; $80EBFF
    stx.b $CA                   ; $80EC01
    lda.b $C6                   ; $80EC03
    clc                         ; $80EC05
    adc.b $E5                   ; $80EC06
    sta.b $C6                   ; $80EC08
    stz.b $D3                   ; $80EC0A
    ldx.b $5E                   ; $80EC0C
    ldy.w #$001B                ; $80EC0E
CODE_80EC11:
    lda.l $7F0002,x             ; $80EC11
    sta.l $7F0000,x             ; $80EC15
    inx                         ; $80EC19
    inx                         ; $80EC1A
    dey                         ; $80EC1B
    bne CODE_80EC11             ; $80EC1C
    lda.w #$FFFF                ; $80EC1E
    sta.l $7F0000,x             ; $80EC21
    lda.b $C6                   ; $80EC25
CODE_80EC27:
    sec                         ; $80EC27
    sbc.w #$0100                ; $80EC28
    bcc CODE_80EC39             ; $80EC2B
    sta.b $C6                   ; $80EC2D
    lda.b $CC                   ; $80EC2F
    sta.l $7F0000,x             ; $80EC31
    inc.b $CC                   ; $80EC35
    bra CODE_80EC27             ; $80EC37
CODE_80EC39:
    stz.b $DF                   ; $80EC39
    ldx.b $5E                   ; $80EC3B
    ldy.w #$001C                ; $80EC3D
CODE_80EC40:
    lda.l $7F0000,x             ; $80EC40
    cmp.w #$FFFF                ; $80EC44
    bne CODE_80EC51             ; $80EC47
    inx                         ; $80EC49
    inx                         ; $80EC4A
    dey                         ; $80EC4B
    bne CODE_80EC40             ; $80EC4C
    ldx.w #$0000                ; $80EC4E
CODE_80EC51:
    stx.b $CF                   ; $80EC51
    ply                         ; $80EC53
    lda.b $DF                   ; $80EC54
    cmp.b $CC                   ; $80EC56
    bne CODE_80EC5D             ; $80EC58
    jmp.w $ED6B                 ; $80EC5A
CODE_80EC5D:
    lda.w $0000,y               ; $80EC5D
    bit.w #$00FF                ; $80EC60
    bne CODE_80EC6C             ; $80EC63
    iny                         ; $80EC65
    xba                         ; $80EC66
    asl a                       ; $80EC67
    tax                         ; $80EC68
    jmp.w ($EDDB,x)             ; $80EC69
CODE_80EC6C:
    pha                         ; $80EC6C
    lda.b $D3                   ; $80EC6D
    beq CODE_80ECAE             ; $80EC6F
    ldx.b $CF                   ; $80EC71
    lda.b $DF                   ; $80EC73
    inc a                       ; $80EC75
    cmp.l $7F0000,x             ; $80EC76
    bcs CODE_80ECAE             ; $80EC7A
    ldx.b $D3                   ; $80EC7C
    lda.l $81D2E3,x             ; $80EC7E
    inx                         ; $80EC82
    inx                         ; $80EC83
    stx.b $D3                   ; $80EC84
    ldx.b $74                   ; $80EC86
    sta.l $810902,x             ; $80EC88
    lda.b $CA                   ; $80EC8C
    sta.l $810900,x             ; $80EC8E
    clc                         ; $80EC92
    adc.w #$0010                ; $80EC93
    sta.b $CA                   ; $80EC96
    lda.w #$0100                ; $80EC98
    sta.l $810B01,x             ; $80EC9B
    inx                         ; $80EC9F
    inx                         ; $80ECA0
    inx                         ; $80ECA1
    inx                         ; $80ECA2
    stx.b $74                   ; $80ECA3
    inc.b $DF                   ; $80ECA5
    inc.b $DF                   ; $80ECA7
    iny                         ; $80ECA9
    iny                         ; $80ECAA
    pla                         ; $80ECAB
    bra CODE_80EC5D             ; $80ECAC
CODE_80ECAE:
    lda.w #$3200                ; $80ECAE
    sta.b $16                   ; $80ECB1
    ldx.b $CF                   ; $80ECB3
    lda.b $DF                   ; $80ECB5
    cmp.l $7F0000,x             ; $80ECB7
    bne CODE_80ECDA             ; $80ECBB
    txa                         ; $80ECBD
    sec                         ; $80ECBE
    sbc.b $5E                   ; $80ECBF
    lsr a                       ; $80ECC1
    lsr a                       ; $80ECC2
    xba                         ; $80ECC3
    and.w #$0E00                ; $80ECC4
    clc                         ; $80ECC7
    adc.w #$3200                ; $80ECC8
    sta.b $16                   ; $80ECCB
CODE_80ECCD:
    inx                         ; $80ECCD
    inx                         ; $80ECCE
    lda.l $7F0000,x             ; $80ECCF
    cmp.w #$FFFF                ; $80ECD3
    beq CODE_80ECCD             ; $80ECD6
    stx.b $CF                   ; $80ECD8
CODE_80ECDA:
    pla                         ; $80ECDA
    and.w #$00FF                ; $80ECDB
    cmp.w #$0030                ; $80ECDE
    bcc CODE_80ECFC             ; $80ECE1
    cmp.w #$003A                ; $80ECE3
    bcs CODE_80ECED             ; $80ECE6
    sbc.w #$0015                ; $80ECE8
    bra CODE_80ED18             ; $80ECEB
CODE_80ECED:
    cmp.w #$0041                ; $80ECED
    bcc CODE_80ECFC             ; $80ECF0
    cmp.w #$005B                ; $80ECF2
    bcs CODE_80ECFC             ; $80ECF5
    sbc.w #$0040                ; $80ECF7
    bra CODE_80ED18             ; $80ECFA
CODE_80ECFC:
    cmp.w #$0020                ; $80ECFC
    db $D0,$03 ; bne            ; $80ECFF
    jmp.w $ED57                 ; $80ED01
    incbin !baserom:$6D04..$6D18
CODE_80ED18:
    asl a                       ; $80ED18
    asl a                       ; $80ED19
    tax                         ; $80ED1A
    sta.b $14                   ; $80ED1B
    lda.l $81CCD9,x             ; $80ED1D
    ldx.b $74                   ; $80ED21
    ora.b $16                   ; $80ED23
    sta.l $810902,x             ; $80ED25
    ldx.b $14                   ; $80ED29
    lda.l $81CCDB,x             ; $80ED2B
    ldx.b $74                   ; $80ED2F
    ora.b $16                   ; $80ED31
    sta.l $810906,x             ; $80ED33
    lda.b $CA                   ; $80ED37
    sta.l $810900,x             ; $80ED39
    clc                         ; $80ED3D
    adc.w #$0800                ; $80ED3E
    sta.l $810904,x             ; $80ED41
    lda.w #$0000                ; $80ED45
    sta.l $810B01,x             ; $80ED48
    sta.l $810B05,x             ; $80ED4C
    txa                         ; $80ED50
    clc                         ; $80ED51
    adc.w #$0008                ; $80ED52
    sta.b $74                   ; $80ED55
    lda.b $CA                   ; $80ED57
    clc                         ; $80ED59
    adc.w #$0008                ; $80ED5A
    sta.b $CA                   ; $80ED5D
    iny                         ; $80ED5F
    inc.b $DF                   ; $80ED60
    lda.b $DF                   ; $80ED62
    cmp.b $CC                   ; $80ED64
    beq CODE_80ED6B             ; $80ED66
    jmp.w $EC5D                 ; $80ED68
CODE_80ED6B:
    lda.b $E1                   ; $80ED6B
    bmi CODE_80EDD9             ; $80ED6D
    ldx.b $5E                   ; $80ED6F
    lda.l $7F001A,x             ; $80ED71
    cmp.w #$FFFF                ; $80ED75
    beq CODE_80ED7F             ; $80ED78
    lda.w #$003B                ; $80ED7A
    sta.b $E1                   ; $80ED7D
CODE_80ED7F:
    lda.b $E1                   ; $80ED7F
    cmp.w #$001E                ; $80ED81
    bcc CODE_80EDCF             ; $80ED84
    ldx.b $74                   ; $80ED86
    sbc.w #$0020                ; $80ED88
    bpl CODE_80ED90             ; $80ED8B
    lda.w #$0000                ; $80ED8D
CODE_80ED90:
    lsr a                       ; $80ED90
    and.w #$000E                ; $80ED91
    xba                         ; $80ED94
    clc                         ; $80ED95
    adc.w #$3200                ; $80ED96
    sta.b $16                   ; $80ED99
    clc                         ; $80ED9B
    adc.l $81CDA1               ; $80ED9C
    sta.l $810902,x             ; $80EDA0
    lda.l $81CDA3               ; $80EDA4
    clc                         ; $80EDA8
    adc.b $16                   ; $80EDA9
    sta.l $810906,x             ; $80EDAB
    lda.b $CA                   ; $80EDAF
    sta.l $810900,x             ; $80EDB1
    clc                         ; $80EDB5
    adc.w #$0800                ; $80EDB6
    sta.l $810904,x             ; $80EDB9
    lda.w #$0000                ; $80EDBD
    sta.l $810B01,x             ; $80EDC0
    sta.l $810B05,x             ; $80EDC4
    txa                         ; $80EDC8
    clc                         ; $80EDC9
    adc.w #$0008                ; $80EDCA
    sta.b $74                   ; $80EDCD
CODE_80EDCF:
    lda.b $E1                   ; $80EDCF
    dec a                       ; $80EDD1
    bpl CODE_80EDD7             ; $80EDD2
    lda.w #$003B                ; $80EDD4
CODE_80EDD7:
    sta.b $E1                   ; $80EDD7
CODE_80EDD9:
    plb                         ; $80EDD9
    rts                         ; $80EDDA
    incbin !baserom:$6DDB..$6DF6
    dey                         ; $80EDF6
    dey                         ; $80EDF7
    stz.b $C6                   ; $80EDF8
    jmp.w $ED5F                 ; $80EDFA
    lda.w $0001,y               ; $80EDFD
    clc                         ; $80EE00
    adc.b $C8                   ; $80EE01
    sta.b $CA                   ; $80EE03
    sta.b $C8                   ; $80EE05
    iny                         ; $80EE07
    iny                         ; $80EE08
    iny                         ; $80EE09
    jmp.w $EC5D                 ; $80EE0A
    incbin !baserom:$6E0D..$6E45
    lda.w $0001,y               ; $80EE45
    sta.b $D3                   ; $80EE48
    iny                         ; $80EE4A
    iny                         ; $80EE4B
    iny                         ; $80EE4C
    jmp.w $EC5D                 ; $80EE4D
    stz.b $D3                   ; $80EE50
    iny                         ; $80EE52
    jmp.w $EC5D                 ; $80EE53
    stz.w $0196                 ; $80EE56
    stz.w $0198                 ; $80EE59
    stz.w $019A                 ; $80EE5C
    ldx.w #$FB5C                ; $80EE5F
    lda.w #$0080                ; $80EE62
    jsr.w $81CC                 ; $80EE65
    ldx.w #$8000                ; $80EE68
    ldy.w #$5010                ; $80EE6B
    lda.w #$008D                ; $80EE6E
    jsr.w $8381                 ; $80EE71
    ldx.w #$AADF                ; $80EE74
    ldy.w #$7400                ; $80EE77
    lda.w #$0091                ; $80EE7A
    jsr.w $8381                 ; $80EE7D
    ldx.w #$AB17                ; $80EE80
    ldy.w #$2000                ; $80EE83
    lda.w #$7E91                ; $80EE86
    jsr.w $FBA0                 ; $80EE89
    ldx.w #$8020                ; $80EE8C
    ldy.w #$0000                ; $80EE8F
    lda.w #$7F94                ; $80EE92
    jsr.w $FBA0                 ; $80EE95
    ldy.w #$1902                ; $80EE98
    sty.w $4300                 ; $80EE9B
    ldy.w #$0000                ; $80EE9E
    sty.w $4302                 ; $80EEA1
    sta.w $4305                 ; $80EEA4
    stz.w $2116                 ; $80EEA7
    sep.b #$20                  ; $80EEAA
    lda.b #$7F                  ; $80EEAC
    sta.w $4304                 ; $80EEAE
    lda.b #$01                  ; $80EEB1
    sta.w $420B                 ; $80EEB3
    rep.b #$20                  ; $80EEB6
    ldx.w #$0000                ; $80EEB8
    ldy.w #$4000                ; $80EEBB
    lda.w #$0000                ; $80EEBE
CODE_80EEC1:
    sta.l $7F0000,x             ; $80EEC1
    inx                         ; $80EEC5
    inx                         ; $80EEC6
    dey                         ; $80EEC7
    bne CODE_80EEC1             ; $80EEC8
    ldx.w #$C343                ; $80EECA
    ldy.w #$4000                ; $80EECD
    lda.w #$7F90                ; $80EED0
    jsr.w $FBA0                 ; $80EED3
    ldx.w #$050B                ; $80EED6
    ldy.w #$4000                ; $80EED9
    pea.w $817F                 ; $80EEDC
    plb                         ; $80EEDF
    lda.w #$0008                ; $80EEE0
    sta.b $14                   ; $80EEE3
CODE_80EEE5:
    lda.w $0000,y               ; $80EEE5
    sta.w $0000,x               ; $80EEE8
    lda.w $0002,y               ; $80EEEB
    sta.w $0001,x               ; $80EEEE
    lda.w $0004,y               ; $80EEF1
    sta.w $0002,x               ; $80EEF4
    lda.w $0006,y               ; $80EEF7
    sta.w $0003,x               ; $80EEFA
    lda.w $0008,y               ; $80EEFD
    sta.w $0004,x               ; $80EF00
    lda.w $000A,y               ; $80EF03
    sta.w $0005,x               ; $80EF06
    lda.w $000C,y               ; $80EF09
    sta.w $0006,x               ; $80EF0C
    lda.w $000E,y               ; $80EF0F
    sta.w $0007,x               ; $80EF12
    lda.w $0010,y               ; $80EF15
    sta.w $0008,x               ; $80EF18
    lda.w $0012,y               ; $80EF1B
    sta.w $0009,x               ; $80EF1E
    txa                         ; $80EF21
    clc                         ; $80EF22
    adc.w #$0080                ; $80EF23
    tax                         ; $80EF26
    tya                         ; $80EF27
    clc                         ; $80EF28
    adc.w #$0014                ; $80EF29
    tay                         ; $80EF2C
    dec.b $14                   ; $80EF2D
    bne CODE_80EEE5             ; $80EF2F
    plb                         ; $80EF31
    lda.w #$1802                ; $80EF32
    sta.w $4300                 ; $80EF35
    lda.w #$0000                ; $80EF38
    sta.w $4302                 ; $80EF3B
    lda.w #$4000                ; $80EF3E
    sta.w $4305                 ; $80EF41
    stz.w $2116                 ; $80EF44
    sep.b #$20                  ; $80EF47
    lda.b #$7F                  ; $80EF49
    sta.w $4304                 ; $80EF4B
    lda.b #$00                  ; $80EF4E
    sta.w $2115                 ; $80EF50
    lda.b #$01                  ; $80EF53
    sta.w $420B                 ; $80EF55
    lda.b #$80                  ; $80EF58
    sta.w $2115                 ; $80EF5A
    rep.b #$20                  ; $80EF5D
    lda.w #$4400                ; $80EF5F
    sta.w $2116                 ; $80EF62
    ldx.w #$0140                ; $80EF65
    lda.w #$00A8                ; $80EF68
CODE_80EF6B:
    sta.w $2118                 ; $80EF6B
    dex                         ; $80EF6E
    bne CODE_80EF6B             ; $80EF6F
    ldx.w #$0008                ; $80EF71
    lda.w #$00FF                ; $80EF74
CODE_80EF77:
    sta.w $2118                 ; $80EF77
    dex                         ; $80EF7A
    bne CODE_80EF77             ; $80EF7B
    ldx.w #$07FE                ; $80EF7D
    lda.w #$0000                ; $80EF80
CODE_80EF83:
    sta.l $7F0000,x             ; $80EF83
    dex                         ; $80EF87
    dex                         ; $80EF88
    bpl CODE_80EF83             ; $80EF89
    lda.w #$5000                ; $80EF8B
    sta.w $2116                 ; $80EF8E
    ldx.w #$0010                ; $80EF91
CODE_80EF94:
    stz.w $2118                 ; $80EF94
    dex                         ; $80EF97
    bne CODE_80EF94             ; $80EF98
    jsr.w $83E8                 ; $80EF9A
    phb                         ; $80EF9D
    ldx.w #$95C3                ; $80EF9E
    ldy.w #$0600                ; $80EFA1
    lda.w #$001F                ; $80EFA4
    mvn $81,$8C                 ; $80EFA7
    ldx.w #$84F2                ; $80EFAA
    ldy.w #$0502                ; $80EFAD
    lda.w #$001F                ; $80EFB0
    mvn $81,$89                 ; $80EFB3
    ldy.w #$0540                ; $80EFB6
    ldx.w #$9ECA                ; $80EFB9
    lda.w #$003F                ; $80EFBC
    mvn $81,$9D                 ; $80EFBF
    ldy.w #$0520                ; $80EFC2
    ldx.w #$84AE                ; $80EFC5
    lda.w #$001F                ; $80EFC8
    mvn $81,$8F                 ; $80EFCB
    ldy.w #$0580                ; $80EFCE
    ldx.w #$84CE                ; $80EFD1
    lda.w #$001F                ; $80EFD4
    mvn $81,$8F                 ; $80EFD7
    ldy.w #$05A0                ; $80EFDA
    ldx.w #$84EE                ; $80EFDD
    lda.w #$001F                ; $80EFE0
    mvn $81,$8F                 ; $80EFE3
    ldy.w #$05C0                ; $80EFE6
    ldx.w #$850E                ; $80EFE9
    lda.w #$001F                ; $80EFEC
    mvn $81,$8F                 ; $80EFEF
    plb                         ; $80EFF2
    lda.w #$17E5                ; $80EFF3
    sta.w $0500                 ; $80EFF6
    jsr.w $F8F8                 ; $80EFF9
    ldx.w #$D750                ; $80EFFC
    lda.w #$0081                ; $80EFFF
    ldy.w #$05C4                ; $80F002
    jsr.w $B556                 ; $80F005
    lda.w #$0080                ; $80F008
    sta.b $E5                   ; $80F00B
    lda.w #$001C                ; $80F00D
    sta.b $F9                   ; $80F010
    sep.b #$20                  ; $80F012
    lda.b #$07                  ; $80F014
    sta.w $2105                 ; $80F016
    lda.b #$01                  ; $80F019
    stz.w $211B                 ; $80F01B
    sta.w $211B                 ; $80F01E
    stz.w $211C                 ; $80F021
    stz.w $211C                 ; $80F024
    stz.w $211D                 ; $80F027
    stz.w $211D                 ; $80F02A
    stz.w $211E                 ; $80F02D
    sta.w $211E                 ; $80F030
    lda.b #$C0                  ; $80F033
    sta.w $211A                 ; $80F035
    rep.b #$20                  ; $80F038
    lda.w #$0000                ; $80F03A
    ldx.w #$0040                ; $80F03D
    jsr.w $F950                 ; $80F040
    jsr.w $89D1                 ; $80F043
    lda.w #$0003                ; $80F046
    tsb.w $0026                 ; $80F049
    lda.w #$0500                ; $80F04C
    sta.w $4310                 ; $80F04F
    lda.w #$D712                ; $80F052
    sta.w $4312                 ; $80F055
    lda.w #$2C01                ; $80F058
    sta.w $4320                 ; $80F05B
    lda.w #$D72E                ; $80F05E
    sta.w $4322                 ; $80F061
    lda.w #$0B00                ; $80F064
    sta.w $4330                 ; $80F067
    lda.w #$D720                ; $80F06A
    sta.w $4332                 ; $80F06D
    lda.w #$1042                ; $80F070
    sta.w $4340                 ; $80F073
    lda.w #$D738                ; $80F076
    sta.w $4342                 ; $80F079
    lda.w #$2601                ; $80F07C
    sta.w $4370                 ; $80F07F
    lda.w #$0D00                ; $80F082
    sta.w $4372                 ; $80F085
    stz.w $0D00                 ; $80F088
    lda.w #$2801                ; $80F08B
    sta.w $4360                 ; $80F08E
    lda.w #$0D40                ; $80F091
    sta.w $4362                 ; $80F094
    stz.w $0D40                 ; $80F097
    sep.b #$20                  ; $80F09A
    lda.b #$81                  ; $80F09C
    sta.w $4314                 ; $80F09E
    sta.w $4324                 ; $80F0A1
    sta.w $4334                 ; $80F0A4
    sta.w $4344                 ; $80F0A7
    sta.w $4347                 ; $80F0AA
    lda.b #$81                  ; $80F0AD
    sta.w $4374                 ; $80F0AF
    lda.b #$81                  ; $80F0B2
    sta.w $4364                 ; $80F0B4
    lda.b #$DE                  ; $80F0B7
    sta.w $420C                 ; $80F0B9
    rep.b #$20                  ; $80F0BC
    stz.b $3E                   ; $80F0BE
    lda.w #$27FF                ; $80F0C0
    sta.b $7A                   ; $80F0C3
    stz.b $7C                   ; $80F0C5
    stz.b $80                   ; $80F0C7
    stz.b $82                   ; $80F0C9
    lda.w #$4000                ; $80F0CB
    sta.b $86                   ; $80F0CE
    lda.w #$001B                ; $80F0D0
    sta.b $7E                   ; $80F0D3
    lda.w #$C000                ; $80F0D5
    sta.b $3C                   ; $80F0D8
    lda.w #$1C00                ; $80F0DA
    sta.b $3A                   ; $80F0DD
    lda.w #$0800                ; $80F0DF
    sta.b $38                   ; $80F0E2
    lda.w #$0100                ; $80F0E4
    sta.b $36                   ; $80F0E7
    stz.b $34                   ; $80F0E9
    stz.b $32                   ; $80F0EB
    jsr.w $F745                 ; $80F0ED
    stz.b $30                   ; $80F0F0
    lda.w #$0F00                ; $80F0F2
    sta.w $0198                 ; $80F0F5
    lda.w #$0080                ; $80F0F8
    sta.w $0196                 ; $80F0FB
    stz.w $000E                 ; $80F0FE
    stz.w $0012                 ; $80F101
    stz.b $40                   ; $80F104
    lda.w #$0200                ; $80F106
    sta.b $7C                   ; $80F109
    stz.b $86                   ; $80F10B
    stz.b $3A                   ; $80F10D
    lda.w #$0004                ; $80F10F
    tsb.w $0026                 ; $80F112
    lda.w #$0001                ; $80F115
    jsr.w $8373                 ; $80F118
    lda.b $30                   ; $80F11B
    asl a                       ; $80F11D
    tax                         ; $80F11E
    lda.w #$1801                ; $80F11F
    sta.w $4300                 ; $80F122
    lda.w #$0000                ; $80F125
    sta.w $4302                 ; $80F128
    lda.w #$0800                ; $80F12B
    sta.w $4305                 ; $80F12E
    lda.w #$4000                ; $80F131
    sta.w $2116                 ; $80F134
    sep.b #$20                  ; $80F137
    lda.b #$7F                  ; $80F139
    sta.w $4304                 ; $80F13B
    lda.b #$01                  ; $80F13E
    sta.w $420B                 ; $80F140
    stz.w $2121                 ; $80F143
    lda.w $D9AC,x               ; $80F146
    sta.w $2122                 ; $80F149
    lda.w $D9AD,x               ; $80F14C
    sta.w $2122                 ; $80F14F
    lda.b $82                   ; $80F152
    sta.w $2110                 ; $80F154
    lda.b $83                   ; $80F157
    sta.w $2110                 ; $80F159
    rep.b #$20                  ; $80F15C
    lda.b $36                   ; $80F15E
    clc                         ; $80F160
    adc.w #$0D00                ; $80F161
    sta.w $4372                 ; $80F164
    clc                         ; $80F167
    adc.w #$0040                ; $80F168
    sta.w $4362                 ; $80F16B
    lda.b $82                   ; $80F16E
    sta.b $84                   ; $80F170
    lda.b $3E                   ; $80F172
    asl a                       ; $80F174
    tax                         ; $80F175
    jsr.w ($F1E3,x)             ; $80F176
    bcs CODE_80F190             ; $80F179
    lda.b $86                   ; $80F17B
    and.w #$FF00                ; $80F17D
    xba                         ; $80F180
    tax                         ; $80F181
    lda.b $7C                   ; $80F182
    jsr.w $F950                 ; $80F184
    jsr.w $89D1                 ; $80F187
    lda.w #$0002                ; $80F18A
    tsb.w $0026                 ; $80F18D
CODE_80F190:
    jsr.w $B59D                 ; $80F190
    lda.b $F6                   ; $80F193
    sec                         ; $80F195
    sbc.w #$0003                ; $80F196
    bpl CODE_80F19E             ; $80F199
    lda.w #$0000                ; $80F19B
CODE_80F19E:
    asl a                       ; $80F19E
    asl a                       ; $80F19F
    asl a                       ; $80F1A0
    clc                         ; $80F1A1
    adc.w #$FFFF                ; $80F1A2
    sta.b $82                   ; $80F1A5
    lda.w #$0001                ; $80F1A7
    trb.b $F8                   ; $80F1AA
    beq CODE_80F1B0             ; $80F1AC
    inc.b $3E                   ; $80F1AE
CODE_80F1B0:
    lda.w #$C0C0                ; $80F1B0
    trb.w $000E                 ; $80F1B3
    db $D0,$05 ; bne            ; $80F1B6
    trb.w $0012                 ; $80F1B8
    beq CODE_80F1CA             ; $80F1BB
    incbin !baserom:$71BD..$71CA
CODE_80F1CA:
    phd                         ; $80F1CA
    lda.w #$0000                ; $80F1CB
    tcd                         ; $80F1CE
    jsl.l $8386FB               ; $80F1CF
    pld                         ; $80F1D3
    lda.w $019A                 ; $80F1D4
    beq CODE_80F1DC             ; $80F1D7
    jmp.w $F115                 ; $80F1D9
CODE_80F1DC:
    lda.w #$0004                ; $80F1DC
    trb.w $0026                 ; $80F1DF
    rts                         ; $80F1E2
    incbin !baserom:$71E3..$7225
    rts                         ; $80F225
    jsr.w $F38D                 ; $80F226
    inc.b $40                   ; $80F229
    lda.b $40                   ; $80F22B
    cmp.w #$0320                ; $80F22D
    bcc CODE_80F23F             ; $80F230
    lda.b $7C                   ; $80F232
    sec                         ; $80F234
    sbc.w #$0010                ; $80F235
    bpl CODE_80F23D             ; $80F238
    lda.w #$0000                ; $80F23A
CODE_80F23D:
    sta.b $7C                   ; $80F23D
CODE_80F23F:
    jsr.w $F709                 ; $80F23F
    jsr.w $F745                 ; $80F242
    clc                         ; $80F245
    rts                         ; $80F246
    stz.b $7C                   ; $80F247
    lda.w #$4000                ; $80F249
    sta.b $86                   ; $80F24C
    lda.w #$0040                ; $80F24E
    sta.b $18                   ; $80F251
    ldx.w #$0080                ; $80F253
    ldy.w #$0000                ; $80F256
CODE_80F259:
    sep.b #$20                  ; $80F259
    lda.w $0F05,y               ; $80F25B
    sta.w $4202                 ; $80F25E
    lda.b $18                   ; $80F261
    sta.w $4203                 ; $80F263
    rep.b #$20                  ; $80F266
    xba                         ; $80F268
    nop                         ; $80F269
    lda.w $4216                 ; $80F26A
    sta.b $19                   ; $80F26D
    adc.w $0F02,y               ; $80F26F
    sta.w $0F02,y               ; $80F272
    lda.w $0F00,y               ; $80F275
    sec                         ; $80F278
    sbc.b $19                   ; $80F279
    sta.w $0F00,y               ; $80F27B
    tya                         ; $80F27E
    clc                         ; $80F27F
    adc.w #$0006                ; $80F280
    tay                         ; $80F283
    dex                         ; $80F284
    bne CODE_80F259             ; $80F285
    inc.b $3E                   ; $80F287
    lda.w #$9B00                ; $80F289
    sta.b $3A                   ; $80F28C
    lda.w #$1000                ; $80F28E
    sta.b $3C                   ; $80F291
    lda.w #$0800                ; $80F293
    sta.b $40                   ; $80F296
    jsr.w $F709                 ; $80F298
    jsr.w $F745                 ; $80F29B
    clc                         ; $80F29E
    rts                         ; $80F29F
    lda.b $30                   ; $80F2A0
    cmp.w #$000F                ; $80F2A2
    beq CODE_80F2A9             ; $80F2A5
    inc.b $30                   ; $80F2A7
CODE_80F2A9:
    lda.b $3C                   ; $80F2A9
    and.w #$FF00                ; $80F2AB
    cmp.w #$C000                ; $80F2AE
    bne CODE_80F2C1             ; $80F2B1
    lda.b $3A                   ; $80F2B3
    and.w #$FF00                ; $80F2B5
    cmp.w #$1C00                ; $80F2B8
    bne CODE_80F2C1             ; $80F2BB
    inc.b $3E                   ; $80F2BD
    clc                         ; $80F2BF
    rts                         ; $80F2C0
CODE_80F2C1:
    lda.b $3C                   ; $80F2C1
    clc                         ; $80F2C3
    adc.b $40                   ; $80F2C4
    sta.b $3C                   ; $80F2C6
    cmp.w #$F000                ; $80F2C8
    bcc CODE_80F2D4             ; $80F2CB
    lda.w #$F800                ; $80F2CD
    sta.b $40                   ; $80F2D0
    bra CODE_80F2DE             ; $80F2D2
CODE_80F2D4:
    cmp.w #$1000                ; $80F2D4
    bcs CODE_80F2DE             ; $80F2D7
    lda.w #$0800                ; $80F2D9
    sta.b $40                   ; $80F2DC
CODE_80F2DE:
    lda.b $3A                   ; $80F2DE
    sec                         ; $80F2E0
    sbc.w #$0080                ; $80F2E1
    sta.b $3A                   ; $80F2E4
    jsr.w $F709                 ; $80F2E6
    jsr.w $F745                 ; $80F2E9
    clc                         ; $80F2EC
    rts                         ; $80F2ED
    lda.b $30                   ; $80F2EE
    cmp.w #$001F                ; $80F2F0
    bne CODE_80F2F8             ; $80F2F3
    lda.w #$000E                ; $80F2F5
CODE_80F2F8:
    inc a                       ; $80F2F8
    sta.b $30                   ; $80F2F9
    jsr.w $F709                 ; $80F2FB
    jsr.w $F745                 ; $80F2FE
    clc                         ; $80F301
    rts                         ; $80F302
    jsr.w $F38D                 ; $80F303
    lda.b $7A                   ; $80F306
    sec                         ; $80F308
    sbc.w #$00E2                ; $80F309
    cmp.w #$8000                ; $80F30C
    ror a                       ; $80F30F
    cmp.w #$0200                ; $80F310
    bcc CODE_80F318             ; $80F313
    lda.w #$0200                ; $80F315
CODE_80F318:
    sta.b $7C                   ; $80F318
    lda.b $7C                   ; $80F31A
    cmp.w #$8000                ; $80F31C
    ror a                       ; $80F31F
    cmp.w #$8000                ; $80F320
    ror a                       ; $80F323
    eor.w #$FFFF                ; $80F324
    sec                         ; $80F327
    adc.b $7A                   ; $80F328
    cmp.w #$0100                ; $80F32A
    bpl CODE_80F334             ; $80F32D
    stz.b $7C                   ; $80F32F
    lda.w #$0100                ; $80F331
CODE_80F334:
    sta.b $7A                   ; $80F334
    lda.b $86                   ; $80F336
    lsr a                       ; $80F338
    lsr a                       ; $80F339
    lsr a                       ; $80F33A
    cmp.w #$00D0                ; $80F33B
    bcc CODE_80F343             ; $80F33E
    lda.w #$00D0                ; $80F340
CODE_80F343:
    inc a                       ; $80F343
    eor.w #$FFFF                ; $80F344
    sec                         ; $80F347
    adc.b $86                   ; $80F348
    bpl CODE_80F34F             ; $80F34A
    lda.w #$0000                ; $80F34C
CODE_80F34F:
    sta.b $86                   ; $80F34F
    sep.b #$20                  ; $80F351
    lda.b #$80                  ; $80F353
    sta.w $4202                 ; $80F355
    lda.b $87                   ; $80F358
    sta.w $4203                 ; $80F35A
    rep.b #$20                  ; $80F35D
    lda.w #$4000                ; $80F35F
    clc                         ; $80F362
    adc.w $4216                 ; $80F363
    asl a                       ; $80F366
    sta.b $3C                   ; $80F367
    lda.w #$2E00                ; $80F369
    sec                         ; $80F36C
    sbc.w $4216                 ; $80F36D
    asl a                       ; $80F370
    sta.b $3A                   ; $80F371
    lda.b $30                   ; $80F373
    cmp.w #$000F                ; $80F375
    beq CODE_80F385             ; $80F378
    cmp.w #$001F                ; $80F37A
    bne CODE_80F382             ; $80F37D
    lda.w #$000E                ; $80F37F
CODE_80F382:
    inc a                       ; $80F382
    sta.b $30                   ; $80F383
CODE_80F385:
    jsr.w $F709                 ; $80F385
    jsr.w $F745                 ; $80F388
    clc                         ; $80F38B
    rts                         ; $80F38C
    sep.b #$20                  ; $80F38D
    lda.b $7A                   ; $80F38F
    sta.w $211B                 ; $80F391
    lda.b $7B                   ; $80F394
    sta.w $211B                 ; $80F396
    lda.b $7A                   ; $80F399
    sta.w $211E                 ; $80F39B
    lda.b $7B                   ; $80F39E
    sta.w $211E                 ; $80F3A0
    lda.b #$80                  ; $80F3A3
    sec                         ; $80F3A5
    sbc.b $3D                   ; $80F3A6
    sta.w $210D                 ; $80F3A8
    lda.b #$00                  ; $80F3AB
    sbc.b #$00                  ; $80F3AD
    sta.w $210D                 ; $80F3AF
    lda.b #$6F                  ; $80F3B2
    sec                         ; $80F3B4
    sbc.b $3B                   ; $80F3B5
    sta.w $210E                 ; $80F3B7
    lda.b #$00                  ; $80F3BA
    sbc.b #$00                  ; $80F3BC
    sta.w $210E                 ; $80F3BE
    rep.b #$20                  ; $80F3C1
    rts                         ; $80F3C3
    lda.w #$1801                ; $80F3C4
    sta.w $4300                 ; $80F3C7
    lda.w #$2000                ; $80F3CA
    sta.w $4302                 ; $80F3CD
    lda.w #$04A0                ; $80F3D0
    sta.w $4305                 ; $80F3D3
    lda.w #$3100                ; $80F3D6
    sta.w $2116                 ; $80F3D9
    sep.b #$20                  ; $80F3DC
    lda.b #$7E                  ; $80F3DE
    sta.w $4304                 ; $80F3E0
    lda.b #$01                  ; $80F3E3
    sta.w $420B                 ; $80F3E5
    rep.b #$20                  ; $80F3E8
    inc.b $3E                   ; $80F3EA
    jsr.w $F709                 ; $80F3EC
    jsr.w $F745                 ; $80F3EF
    clc                         ; $80F3F2
    rts                         ; $80F3F3
    lda.w #$1801                ; $80F3F4
    sta.w $4300                 ; $80F3F7
    lda.w #$24A0                ; $80F3FA
    sta.w $4302                 ; $80F3FD
    lda.w #$04A0                ; $80F400
    sta.w $4305                 ; $80F403
    lda.w #$3350                ; $80F406
    sta.w $2116                 ; $80F409
    sep.b #$20                  ; $80F40C
    lda.b #$7E                  ; $80F40E
    sta.w $4304                 ; $80F410
    lda.b #$01                  ; $80F413
    sta.w $420B                 ; $80F415
    rep.b #$20                  ; $80F418
    inc.b $3E                   ; $80F41A
    jsr.w $F709                 ; $80F41C
    jsr.w $F745                 ; $80F41F
    clc                         ; $80F422
    rts                         ; $80F423
    stz.w $2116                 ; $80F424
    stz.w $2118                 ; $80F427
    stz.w $2118                 ; $80F42A
    stz.w $2118                 ; $80F42D
    stz.w $2118                 ; $80F430
    stz.w $2118                 ; $80F433
    stz.w $2118                 ; $80F436
    stz.w $2118                 ; $80F439
    stz.w $2118                 ; $80F43C
    stz.w $2118                 ; $80F43F
    stz.w $2118                 ; $80F442
    stz.w $2118                 ; $80F445
    stz.w $2118                 ; $80F448
    stz.w $2118                 ; $80F44B
    stz.w $2118                 ; $80F44E
    stz.w $2118                 ; $80F451
    stz.w $2118                 ; $80F454
    lda.w #$1801                ; $80F457
    sta.w $4300                 ; $80F45A
    lda.w #$2940                ; $80F45D
    sta.w $4302                 ; $80F460
    lda.w #$0800                ; $80F463
    sta.w $4305                 ; $80F466
    lda.w #$3C00                ; $80F469
    sta.w $2116                 ; $80F46C
    sep.b #$20                  ; $80F46F
    lda.b #$7E                  ; $80F471
    sta.w $4304                 ; $80F473
    lda.b #$01                  ; $80F476
    sta.w $420B                 ; $80F478
    lda.b #$3C                  ; $80F47B
    sta.w $2107                 ; $80F47D
    stz.w $210D                 ; $80F480
    stz.w $210D                 ; $80F483
    lda.b #$03                  ; $80F486
    sta.w $210E                 ; $80F488
    lda.b #$00                  ; $80F48B
    sta.w $210E                 ; $80F48D
    rep.b #$20                  ; $80F490
    lda.w #$D719                ; $80F492
    sta.w $4312                 ; $80F495
    lda.w #$D727                ; $80F498
    sta.w $4332                 ; $80F49B
    lda.w #$2000                ; $80F49E
    sta.b $80                   ; $80F4A1
    inc.b $3E                   ; $80F4A3
    jsr.w $F709                 ; $80F4A5
    jsr.w $F745                 ; $80F4A8
    clc                         ; $80F4AB
    rts                         ; $80F4AC
    ldx.w #$3040                ; $80F4AD
    ldy.w #$0000                ; $80F4B0
    lda.w #$007E                ; $80F4B3
    jmp.w $F56D                 ; $80F4B6
    ldx.w #$3654                ; $80F4B9
    ldy.w #$030A                ; $80F4BC
    lda.w #$007E                ; $80F4BF
    jmp.w $F56D                 ; $80F4C2
    ldx.w #$3C68                ; $80F4C5
    ldy.w #$0614                ; $80F4C8
    lda.w #$007E                ; $80F4CB
    jmp.w $F56D                 ; $80F4CE
    ldx.w #$427C                ; $80F4D1
    ldy.w #$091E                ; $80F4D4
    lda.w #$007E                ; $80F4D7
    jmp.w $F56D                 ; $80F4DA
    ldx.w #$4890                ; $80F4DD
    ldy.w #$0C28                ; $80F4E0
    lda.w #$007E                ; $80F4E3
    jmp.w $F56D                 ; $80F4E6
    ldx.w #$4EA4                ; $80F4E9
    ldy.w #$0F32                ; $80F4EC
    lda.w #$007E                ; $80F4EF
    jmp.w $F56D                 ; $80F4F2
    ldx.w #$54B8                ; $80F4F5
    ldy.w #$123C                ; $80F4F8
    lda.w #$007E                ; $80F4FB
    jmp.w $F56D                 ; $80F4FE
    ldx.w #$5ACC                ; $80F501
    ldy.w #$1546                ; $80F504
    lda.w #$007E                ; $80F507
    jmp.w $F56D                 ; $80F50A
    ldx.w #$60E0                ; $80F50D
    ldy.w #$1850                ; $80F510
    lda.w #$007E                ; $80F513
    jmp.w $F56D                 ; $80F516
    ldx.w #$66F4                ; $80F519
    ldy.w #$1B5A                ; $80F51C
    lda.w #$007E                ; $80F51F
    jmp.w $F56D                 ; $80F522
    ldx.w #$6D08                ; $80F525
    ldy.w #$1E64                ; $80F528
    lda.w #$007E                ; $80F52B
    jmp.w $F56D                 ; $80F52E
    ldx.w #$731C                ; $80F531
    ldy.w #$216E                ; $80F534
    lda.w #$007E                ; $80F537
    jmp.w $F56D                 ; $80F53A
    ldx.w #$7930                ; $80F53D
    ldy.w #$2478                ; $80F540
    lda.w #$007E                ; $80F543
    jmp.w $F56D                 ; $80F546
    ldx.w #$7F44                ; $80F549
    ldy.w #$2782                ; $80F54C
    lda.w #$007E                ; $80F54F
    jmp.w $F56D                 ; $80F552
    ldx.w #$8558                ; $80F555
    ldy.w #$2A8C                ; $80F558
    lda.w #$007E                ; $80F55B
    jmp.w $F56D                 ; $80F55E
    ldx.w #$8B6C                ; $80F561
    ldy.w #$2D96                ; $80F564
    lda.w #$007E                ; $80F567
    jmp.w $F56D                 ; $80F56A
    stx.w $4302                 ; $80F56D
    sty.w $2116                 ; $80F570
    ldx.w #$1801                ; $80F573
    stx.w $4300                 ; $80F576
    ldx.w #$0614                ; $80F579
    stx.w $4305                 ; $80F57C
    sep.b #$20                  ; $80F57F
    sta.w $4304                 ; $80F581
    lda.b #$01                  ; $80F584
    sta.w $420B                 ; $80F586
    rep.b #$20                  ; $80F589
    inc.b $3E                   ; $80F58B
    jsr.w $F709                 ; $80F58D
    jsr.w $F745                 ; $80F590
    clc                         ; $80F593
    rts                         ; $80F594
    lda.b $7E                   ; $80F595
    dec a                       ; $80F597
    cmp.w #$FFB8                ; $80F598
    bpl CODE_80F5A2             ; $80F59B
    inc.b $3E                   ; $80F59D
    lda.w #$FFB8                ; $80F59F
CODE_80F5A2:
    sta.b $7E                   ; $80F5A2
    ldx.w #$0016                ; $80F5A4
CODE_80F5A7:
    lda.w #$1001                ; $80F5A7
    sta.l $7F0214,x             ; $80F5AA
    inc a                       ; $80F5AE
    sta.l $7F0254,x             ; $80F5AF
    dex                         ; $80F5B3
    dex                         ; $80F5B4
    bpl CODE_80F5A7             ; $80F5B5
    jsr.w $F709                 ; $80F5B7
    jsr.w $F745                 ; $80F5BA
    clc                         ; $80F5BD
    rts                         ; $80F5BE
    dec.b $30                   ; $80F5BF
    bne CODE_80F5C5             ; $80F5C1
    inc.b $3E                   ; $80F5C3
CODE_80F5C5:
    jsr.w $F709                 ; $80F5C5
    jsr.w $F745                 ; $80F5C8
    clc                         ; $80F5CB
    rts                         ; $80F5CC
    stz.b $40                   ; $80F5CD
    inc.b $3E                   ; $80F5CF
    lda.w #$0001                ; $80F5D1
    sta.b $42                   ; $80F5D4
    lda.w #$003C                ; $80F5D6
    sta.b $1E                   ; $80F5D9
    jmp.w $F670                 ; $80F5DB
    lda.b $40                   ; $80F5DE
    cmp.w #$013A                ; $80F5E0
    bne CODE_80F5F3             ; $80F5E3
    lda.w #$9180                ; $80F5E5
    sta.w $4302                 ; $80F5E8
    lda.w #$0000                ; $80F5EB
    sta.w $2116                 ; $80F5EE
    bra CODE_80F62A             ; $80F5F1
CODE_80F5F3:
    cmp.w #$013C                ; $80F5F3
    bne CODE_80F606             ; $80F5F6
    lda.w #$9768                ; $80F5F8
    sta.w $4302                 ; $80F5FB
    lda.w #$02F4                ; $80F5FE
    sta.w $2116                 ; $80F601
    bra CODE_80F62A             ; $80F604
CODE_80F606:
    cmp.w #$013E                ; $80F606
    bne CODE_80F619             ; $80F609
    lda.w #$9D50                ; $80F60B
    sta.w $4302                 ; $80F60E
    lda.w #$05E8                ; $80F611
    sta.w $2116                 ; $80F614
    bra CODE_80F62A             ; $80F617
CODE_80F619:
    cmp.w #$0140                ; $80F619
    bne CODE_80F644             ; $80F61C
    lda.w #$A338                ; $80F61E
    sta.w $4302                 ; $80F621
    lda.w #$08DC                ; $80F624
    sta.w $2116                 ; $80F627
CODE_80F62A:
    lda.w #$1801                ; $80F62A
    sta.w $4300                 ; $80F62D
    lda.w #$0614                ; $80F630
    sta.w $4305                 ; $80F633
    sep.b #$20                  ; $80F636
    lda.b #$7E                  ; $80F638
    sta.w $4304                 ; $80F63A
    lda.b #$01                  ; $80F63D
    sta.w $420B                 ; $80F63F
    rep.b #$20                  ; $80F642
CODE_80F644:
    lda.w #$FFFF                ; $80F644
    sta.b $7E                   ; $80F647
    sep.b #$20                  ; $80F649
    lda.b #$0A                  ; $80F64B
    sta.w $2123                 ; $80F64D
    rep.b #$20                  ; $80F650
    stz.w $0D00                 ; $80F652
    stz.w $0D40                 ; $80F655
    stz.w $0E00                 ; $80F658
    stz.w $0E40                 ; $80F65B
    lda.w #$00FF                ; $80F65E
    sta.w $2126                 ; $80F661
    sta.w $2128                 ; $80F664
    lda.w #$0002                ; $80F667
    sta.w $2123                 ; $80F66A
    stz.w $212A                 ; $80F66D
    lda.b $40                   ; $80F670
    cmp.w #$015C                ; $80F672
    bcc CODE_80F68A             ; $80F675
    inc.b $42                   ; $80F677
    lda.b $42                   ; $80F679
    lsr a                       ; $80F67B
    clc                         ; $80F67C
    adc.w #$0040                ; $80F67D
    sta.b $30                   ; $80F680
    lda.w #$0800                ; $80F682
    sta.b $36                   ; $80F685
    jsr.w $FA5B                 ; $80F687
CODE_80F68A:
    lda.b $40                   ; $80F68A
    and.w #$FFFE                ; $80F68C
    tax                         ; $80F68F
    lda.l $7EA920,x             ; $80F690
    tax                         ; $80F694
    ldy.w #$0000                ; $80F695
    lda.w #$7F7E                ; $80F698
    jsr.w $FBA0                 ; $80F69B
    inc.b $40                   ; $80F69E
    inc.b $40                   ; $80F6A0
    lda.b $40                   ; $80F6A2
    cmp.w #$017C                ; $80F6A4
    bne CODE_80F6AB             ; $80F6A7
    inc.b $3E                   ; $80F6A9
CODE_80F6AB:
    sec                         ; $80F6AB
    rts                         ; $80F6AC
    lda.b $42                   ; $80F6AD
    inc a                       ; $80F6AF
    cmp.w #$007F                ; $80F6B0
    bne CODE_80F6B7             ; $80F6B3
    inc.b $3E                   ; $80F6B5
CODE_80F6B7:
    sta.b $42                   ; $80F6B7
    cmp.w #$0020                ; $80F6B9
    bcs CODE_80F6C5             ; $80F6BC
    lsr a                       ; $80F6BE
    clc                         ; $80F6BF
    adc.w #$0040                ; $80F6C0
    bra CODE_80F6DB             ; $80F6C3
CODE_80F6C5:
    sec                         ; $80F6C5
    sbc.w #$003F                ; $80F6C6
    bcs CODE_80F6CE             ; $80F6C9
    lda.w #$0000                ; $80F6CB
CODE_80F6CE:
    lsr a                       ; $80F6CE
    clc                         ; $80F6CF
    adc.w #$0020                ; $80F6D0
    cmp.w #$0040                ; $80F6D3
    bcc CODE_80F6DB             ; $80F6D6
    lda.w #$003F                ; $80F6D8
CODE_80F6DB:
    sta.b $30                   ; $80F6DB
    jsr.w $FAEE                 ; $80F6DD
    lda.w #$0800                ; $80F6E0
    sta.b $36                   ; $80F6E3
    jsr.w $FA5B                 ; $80F6E5
    lda.b $42                   ; $80F6E8
    cmp.w #$003F                ; $80F6EA
    bne CODE_80F6FB             ; $80F6ED
    ldx.w #$C730                ; $80F6EF
    ldy.w #$0000                ; $80F6F2
    lda.w #$7F7E                ; $80F6F5
    jsr.w $FBA0                 ; $80F6F8
CODE_80F6FB:
    sec                         ; $80F6FB
    rts                         ; $80F6FC
    stz.w $0198                 ; $80F6FD
    lda.w #$FF80                ; $80F700
    sta.w $0196                 ; $80F703
    inc.b $3E                   ; $80F706
    rts                         ; $80F708
    lda.w #$0100                ; $80F709
    sta.b $14                   ; $80F70C
    lda.b $7A                   ; $80F70E
    cmp.w #$0100                ; $80F710
    beq CODE_80F73F             ; $80F713
    ldx.w #$0008                ; $80F715
    lda.w #$0200                ; $80F718
    stz.b $14                   ; $80F71B
CODE_80F71D:
    cmp.b $7A                   ; $80F71D
    bcc CODE_80F723             ; $80F71F
    sbc.b $7A                   ; $80F721
CODE_80F723:
    rol.b $14                   ; $80F723
    asl a                       ; $80F725
    dex                         ; $80F726
    bne CODE_80F71D             ; $80F727
    sep.b #$20                  ; $80F729
    lda.b $14                   ; $80F72B
    sta.w $4202                 ; $80F72D
    lda.b #$2A                  ; $80F730
    sta.w $4203                 ; $80F732
    rep.b #$20                  ; $80F735
    xba                         ; $80F737
    nop                         ; $80F738
    lda.w $4216                 ; $80F739
    sta.b $38                   ; $80F73C
    rts                         ; $80F73E
CODE_80F73F:
    lda.w #$2A00                ; $80F73F
    sta.b $38                   ; $80F742
    rts                         ; $80F744
    ldx.b $34                   ; $80F745
    lda.b $3A                   ; $80F747
    bne CODE_80F766             ; $80F749
    lda.w #$0001                ; $80F74B
    sta.w $0D00,x               ; $80F74E
    sta.w $0D40,x               ; $80F751
    lda.w #$00FF                ; $80F754
    sta.w $0D01,x               ; $80F757
    sta.w $0D41,x               ; $80F75A
    stz.w $0D03,x               ; $80F75D
    stz.w $0D43,x               ; $80F760
    jmp.w $F8EF                 ; $80F763
CODE_80F766:
    lda.b $38                   ; $80F766
    and.w #$FF00                ; $80F768
    sta.b $18                   ; $80F76B
    lda.b $3A                   ; $80F76D
    and.w #$FF00                ; $80F76F
    sec                         ; $80F772
    sbc.b $18                   ; $80F773
    beq CODE_80F7B1             ; $80F775
    xba                         ; $80F777
    cmp.w #$0080                ; $80F778
    bcc CODE_80F799             ; $80F77B
    sbc.w #$007F                ; $80F77D
    sta.w $0D00,x               ; $80F780
    lda.b $3C                   ; $80F783
    and.w #$FF00                ; $80F785
    sta.b $14                   ; $80F788
    xba                         ; $80F78A
    ora.b $14                   ; $80F78B
    sta.w $0D01,x               ; $80F78D
    txa                         ; $80F790
    clc                         ; $80F791
    adc.w #$0003                ; $80F792
    tax                         ; $80F795
    lda.w #$007F                ; $80F796
CODE_80F799:
    sta.w $0D00,x               ; $80F799
    lda.b $3C                   ; $80F79C
    and.w #$FF00                ; $80F79E
    sta.b $14                   ; $80F7A1
    xba                         ; $80F7A3
    ora.b $14                   ; $80F7A4
    sta.b $16                   ; $80F7A6
    sta.w $0D01,x               ; $80F7A8
    txa                         ; $80F7AB
    clc                         ; $80F7AC
    adc.w #$0003                ; $80F7AD
    tax                         ; $80F7B0
CODE_80F7B1:
    lda.w #$8081                ; $80F7B1
    sta.b $14                   ; $80F7B4
    lda.b $38                   ; $80F7B6
    and.w #$FF00                ; $80F7B8
    beq CODE_80F801             ; $80F7BB
    clc                         ; $80F7BD
    adc.b $3C                   ; $80F7BE
    and.w #$FF00                ; $80F7C0
    sta.b $14                   ; $80F7C3
    lda.b $38                   ; $80F7C5
    and.w #$FF00                ; $80F7C7
    asl a                       ; $80F7CA
    eor.w #$FFFF                ; $80F7CB
    sec                         ; $80F7CE
    adc.b $14                   ; $80F7CF
    and.w #$FF00                ; $80F7D1
    xba                         ; $80F7D4
    ora.b $14                   ; $80F7D5
    sta.b $14                   ; $80F7D7
    lda.w #$0001                ; $80F7D9
    sta.w $0D00,x               ; $80F7DC
    lda.b $14                   ; $80F7DF
    sta.w $0D01,x               ; $80F7E1
    txa                         ; $80F7E4
    clc                         ; $80F7E5
    adc.w #$0003                ; $80F7E6
    tax                         ; $80F7E9
    lda.b $38                   ; $80F7EA
    and.w #$FF00                ; $80F7EC
    xba                         ; $80F7EF
    dec a                       ; $80F7F0
    beq CODE_80F801             ; $80F7F1
    sta.w $0D00,x               ; $80F7F3
    lda.b $14                   ; $80F7F6
    sta.w $0D01,x               ; $80F7F8
    txa                         ; $80F7FB
    clc                         ; $80F7FC
    adc.w #$0003                ; $80F7FD
    tax                         ; $80F800
CODE_80F801:
    lda.w #$0001                ; $80F801
    sta.w $0D00,x               ; $80F804
    lda.w #$FF00                ; $80F807
    sta.w $0D01,x               ; $80F80A
    lda.b $14                   ; $80F80D
    txa                         ; $80F80F
    clc                         ; $80F810
    adc.w #$0003                ; $80F811
    tax                         ; $80F814
    lda.b $38                   ; $80F815
    and.w #$FF00                ; $80F817
    beq CODE_80F842             ; $80F81A
    sec                         ; $80F81C
    sbc.w #$0100                ; $80F81D
    beq CODE_80F831             ; $80F820
    xba                         ; $80F822
    sta.w $0D00,x               ; $80F823
    lda.b $14                   ; $80F826
    sta.w $0D01,x               ; $80F828
    txa                         ; $80F82B
    clc                         ; $80F82C
    adc.w #$0003                ; $80F82D
    tax                         ; $80F830
CODE_80F831:
    lda.w #$0001                ; $80F831
    sta.w $0D00,x               ; $80F834
    lda.b $14                   ; $80F837
    sta.w $0D01,x               ; $80F839
    txa                         ; $80F83C
    clc                         ; $80F83D
    adc.w #$0003                ; $80F83E
    tax                         ; $80F841
CODE_80F842:
    lda.w #$0001                ; $80F842
    sta.w $0D00,x               ; $80F845
    lda.b $16                   ; $80F848
    sta.w $0D01,x               ; $80F84A
    txa                         ; $80F84D
    clc                         ; $80F84E
    adc.w #$0003                ; $80F84F
    tax                         ; $80F852
    stz.w $0D00,x               ; $80F853
    ldx.b $34                   ; $80F856
    lda.b $38                   ; $80F858
    and.w #$FF00                ; $80F85A
    sta.b $16                   ; $80F85D
    lda.b $3A                   ; $80F85F
    and.w #$FF00                ; $80F861
    sec                         ; $80F864
    sbc.b $16                   ; $80F865
    xba                         ; $80F867
    and.w #$00FF                ; $80F868
    inc a                       ; $80F86B
    cmp.w #$0080                ; $80F86C
    bcc CODE_80F883             ; $80F86F
    sbc.w #$007F                ; $80F871
    sta.w $0D40,x               ; $80F874
    lda.w #$00FF                ; $80F877
    sta.w $0D41,x               ; $80F87A
    inx                         ; $80F87D
    inx                         ; $80F87E
    inx                         ; $80F87F
    lda.w #$007F                ; $80F880
CODE_80F883:
    sta.w $0D40,x               ; $80F883
    lda.w #$00FF                ; $80F886
    sta.w $0D41,x               ; $80F889
    inx                         ; $80F88C
    inx                         ; $80F88D
    inx                         ; $80F88E
    lda.b $32                   ; $80F88F
    and.w #$FF00                ; $80F891
    sta.b $16                   ; $80F894
    beq CODE_80F8B9             ; $80F896
    incbin !baserom:$7898..$78B9
CODE_80F8B9:
    lda.b $16                   ; $80F8B9
    xba                         ; $80F8BB
    sta.b $16                   ; $80F8BC
    lda.b $38                   ; $80F8BE
    xba                         ; $80F8C0
    and.w #$00FF                ; $80F8C1
    beq CODE_80F8EC             ; $80F8C4
    dec a                       ; $80F8C6
    asl a                       ; $80F8C7
    inc a                       ; $80F8C8
    sec                         ; $80F8C9
    sbc.b $16                   ; $80F8CA
    beq CODE_80F8EC             ; $80F8CC
    sta.w $0D40,x               ; $80F8CE
    lda.b $14                   ; $80F8D1
    clc                         ; $80F8D3
    adc.w #$FF01                ; $80F8D4
    sta.w $0D41,x               ; $80F8D7
    inx                         ; $80F8DA
    inx                         ; $80F8DB
    inx                         ; $80F8DC
    lda.w #$0001                ; $80F8DD
    sta.w $0D40,x               ; $80F8E0
    lda.w #$00FF                ; $80F8E3
    sta.w $0D41,x               ; $80F8E6
    inx                         ; $80F8E9
    inx                         ; $80F8EA
    inx                         ; $80F8EB
CODE_80F8EC:
    stz.w $0D40,x               ; $80F8EC
    lda.b $34                   ; $80F8EF
    ldx.b $36                   ; $80F8F1
    stx.b $34                   ; $80F8F3
    sta.b $36                   ; $80F8F5
    rts                         ; $80F8F7
    ldx.w #$0000                ; $80F8F8
    lda.w #$2000                ; $80F8FB
    sta.b $14                   ; $80F8FE
CODE_80F900:
    jsr.w $87E9                 ; $80F900
    sep.b #$20                  ; $80F903
    pha                         ; $80F905
    lda.b $14                   ; $80F906
    sta.w $211B                 ; $80F908
    lda.b $15                   ; $80F90B
    sta.w $211B                 ; $80F90D
    pla                         ; $80F910
    sta.w $211C                 ; $80F911
    rep.b #$20                  ; $80F914
    lda.w $2135                 ; $80F916
    sta.w $0F00,x               ; $80F919
    jsr.w $87E9                 ; $80F91C
    xba                         ; $80F91F
    sep.b #$20                  ; $80F920
    pha                         ; $80F922
    lda.b $14                   ; $80F923
    sta.w $211B                 ; $80F925
    lda.b $15                   ; $80F928
    sta.w $211B                 ; $80F92A
    pla                         ; $80F92D
    sta.w $211C                 ; $80F92E
    rep.b #$20                  ; $80F931
    lda.w $2135                 ; $80F933
    sta.w $0F02,x               ; $80F936
    lda.b $14                   ; $80F939
    sta.w $0F04,x               ; $80F93B
    clc                         ; $80F93E
    adc.w #$00BE                ; $80F93F
    sta.b $14                   ; $80F942
    txa                         ; $80F944
    clc                         ; $80F945
    adc.w #$0006                ; $80F946
    tax                         ; $80F949
    cpx.w #$0300                ; $80F94A
    bne CODE_80F900             ; $80F94D
    rts                         ; $80F94F
    stx.b $18                   ; $80F950
    sta.b $16                   ; $80F952
    ldy.w #$0000                ; $80F954
    ldx.b $74                   ; $80F957
    sep.b #$20                  ; $80F959
    lda.w $0F05,y               ; $80F95B
    sta.w $4202                 ; $80F95E
    lda.b $18                   ; $80F961
    sta.w $4203                 ; $80F963
    rep.b #$20                  ; $80F966
    xba                         ; $80F968
    nop                         ; $80F969
    lda.w $4216                 ; $80F96A
    sta.b $19                   ; $80F96D
    clc                         ; $80F96F
    adc.w $0F00,y               ; $80F970
    bpl CODE_80F99A             ; $80F973
    eor.w #$FFFF                ; $80F975
    inc a                       ; $80F978
    sta.w $4204                 ; $80F979
    sep.b #$20                  ; $80F97C
    lda.w $0F05,y               ; $80F97E
    sta.w $4206                 ; $80F981
    rep.b #$20                  ; $80F984
    xba                         ; $80F986
    xba                         ; $80F987
    nop                         ; $80F988
    lda.w #$0080                ; $80F989
    sec                         ; $80F98C
    sbc.w $4214                 ; $80F98D
    bit.w #$FF00                ; $80F990
    bne CODE_80F997             ; $80F993
    bra CODE_80F9B6             ; $80F995
CODE_80F997:
    jmp.w $FA24                 ; $80F997
CODE_80F99A:
    sta.w $4204                 ; $80F99A
    sep.b #$20                  ; $80F99D
    lda.w $0F05,y               ; $80F99F
    sta.w $4206                 ; $80F9A2
    rep.b #$20                  ; $80F9A5
    xba                         ; $80F9A7
    xba                         ; $80F9A8
    nop                         ; $80F9A9
    lda.w #$0080                ; $80F9AA
    clc                         ; $80F9AD
    adc.w $4214                 ; $80F9AE
    bit.w #$FF00                ; $80F9B1
    bne CODE_80FA24             ; $80F9B4
CODE_80F9B6:
    sta.b $14                   ; $80F9B6
    lda.b $19                   ; $80F9B8
    eor.w #$FFFF                ; $80F9BA
    sec                         ; $80F9BD
    adc.w $0F02,y               ; $80F9BE
    bmi CODE_80F9DC             ; $80F9C1
    sta.w $4204                 ; $80F9C3
    sep.b #$20                  ; $80F9C6
    lda.w $0F05,y               ; $80F9C8
    sta.w $4206                 ; $80F9CB
    rep.b #$20                  ; $80F9CE
    xba                         ; $80F9D0
    xba                         ; $80F9D1
    nop                         ; $80F9D2
    lda.w #$005C                ; $80F9D3
    clc                         ; $80F9D6
    adc.w $4214                 ; $80F9D7
    bra CODE_80F9F7             ; $80F9DA
CODE_80F9DC:
    eor.w #$FFFF                ; $80F9DC
    inc a                       ; $80F9DF
    sta.w $4204                 ; $80F9E0
    sep.b #$20                  ; $80F9E3
    lda.w $0F05,y               ; $80F9E5
    sta.w $4206                 ; $80F9E8
    rep.b #$20                  ; $80F9EB
    xba                         ; $80F9ED
    xba                         ; $80F9EE
    nop                         ; $80F9EF
    lda.w #$005C                ; $80F9F0
    sec                         ; $80F9F3
    sbc.w $4214                 ; $80F9F4
CODE_80F9F7:
    cmp.w #$00E0                ; $80F9F7
    bcs CODE_80FA24             ; $80F9FA
    and.w #$00FF                ; $80F9FC
    xba                         ; $80F9FF
    ora.b $14                   ; $80FA00
    sta.w $0900,x               ; $80FA02
    lda.w $0F05,y               ; $80FA05
    and.w #$007C                ; $80FA08
    phx                         ; $80FA0B
    tax                         ; $80FA0C
    lda.w $D692,x               ; $80FA0D
    ora.b $80                   ; $80FA10
    plx                         ; $80FA12
    sta.w $0902,x               ; $80FA13
    stz.w $0B01,x               ; $80FA16
    lda.w $0F04,y               ; $80FA19
    sec                         ; $80FA1C
    sbc.b $16                   ; $80FA1D
    cmp.w #$0100                ; $80FA1F
    bcs CODE_80FA2A             ; $80FA22
CODE_80FA24:
    jsr.w $FA3F                 ; $80FA24
CODE_80FA27:
    jmp.w $F959                 ; $80FA27
CODE_80FA2A:
    sta.w $0F04,y               ; $80FA2A
    inx                         ; $80FA2D
    inx                         ; $80FA2E
    inx                         ; $80FA2F
    inx                         ; $80FA30
    tya                         ; $80FA31
    clc                         ; $80FA32
    adc.w #$0006                ; $80FA33
    tay                         ; $80FA36
    cpy.w #$0300                ; $80FA37
    bne CODE_80FA27             ; $80FA3A
    stx.b $74                   ; $80FA3C
    rts                         ; $80FA3E
    jsr.w $87E9                 ; $80FA3F
    cmp.w #$8000                ; $80FA42
    ror a                       ; $80FA45
    sta.w $0F00,y               ; $80FA46
    jsr.w $87E9                 ; $80FA49
    xba                         ; $80FA4C
    cmp.w #$8000                ; $80FA4D
    ror a                       ; $80FA50
    sta.w $0F02,y               ; $80FA51
    lda.w #$7F00                ; $80FA54
    sta.w $0F04,y               ; $80FA57
    rts                         ; $80FA5A
    stz.b $14                   ; $80FA5B
    stz.b $16                   ; $80FA5D
    lda.w #$A000                ; $80FA5F
    sta.w $4204                 ; $80FA62
    sep.b #$20                  ; $80FA65
    lda.b $42                   ; $80FA67
    sta.w $4206                 ; $80FA69
    rep.b #$20                  ; $80FA6C
    xba                         ; $80FA6E
    xba                         ; $80FA6F
    xba                         ; $80FA70
    nop                         ; $80FA71
    nop                         ; $80FA72
    lda.w $4214                 ; $80FA73
    sta.b $17                   ; $80FA76
    ldy.w #$00B8                ; $80FA78
CODE_80FA7B:
    ldx.b $15                   ; $80FA7B
    sep.b #$20                  ; $80FA7D
    lda.w $DA4C,x               ; $80FA7F
    sta.w $211B                 ; $80FA82
    lda.w $DACC,x               ; $80FA85
    sta.w $211B                 ; $80FA88
    lda.b $42                   ; $80FA8B
    sta.w $211C                 ; $80FA8D
    lda.b #$7F                  ; $80FA90
    sec                         ; $80FA92
    sbc.w $2135                 ; $80FA93
    bpl CODE_80FA9A             ; $80FA96
    lda.b #$00                  ; $80FA98
CODE_80FA9A:
    sta.w $1501,y               ; $80FA9A
    lda.b #$7E                  ; $80FA9D
    clc                         ; $80FA9F
    adc.w $2135                 ; $80FAA0
    bcc CODE_80FAA7             ; $80FAA3
    lda.b #$FF                  ; $80FAA5
CODE_80FAA7:
    sta.w $1502,y               ; $80FAA7
    rep.b #$20                  ; $80FAAA
    lda.b $14                   ; $80FAAC
    clc                         ; $80FAAE
    adc.b $17                   ; $80FAAF
    sta.b $14                   ; $80FAB1
    cmp.w #$8000                ; $80FAB3
    bcs CODE_80FABE             ; $80FAB6
    dey                         ; $80FAB8
    dey                         ; $80FAB9
    bne CODE_80FA7B             ; $80FABA
    bra CODE_80FACC             ; $80FABC
CODE_80FABE:
    dey                         ; $80FABE
    dey                         ; $80FABF
    beq CODE_80FACC             ; $80FAC0
    lda.w #$00FF                ; $80FAC2
CODE_80FAC5:
    sta.w $1501,y               ; $80FAC5
    dey                         ; $80FAC8
    dey                         ; $80FAC9
    bne CODE_80FAC5             ; $80FACA
CODE_80FACC:
    ldx.w #$00B6                ; $80FACC
    ldy.w #$00BA                ; $80FACF
CODE_80FAD2:
    lda.w $1501,x               ; $80FAD2
    sta.w $1502,y               ; $80FAD5
    iny                         ; $80FAD8
    iny                         ; $80FAD9
    dex                         ; $80FADA
    dex                         ; $80FADB
    bne CODE_80FAD2             ; $80FADC
    sep.b #$20                  ; $80FADE
    lda.b #$DD                  ; $80FAE0
    sta.w $1500                 ; $80FAE2
    sta.w $15BB                 ; $80FAE5
    stz.w $1674                 ; $80FAE8
    rep.b #$20                  ; $80FAEB
    rts                         ; $80FAED
    asl a                       ; $80FAEE
    tax                         ; $80FAEF
    lda.w $D9AC,x               ; $80FAF0
    and.w #$001F                ; $80FAF3
    sta.b $14                   ; $80FAF6
    ldy.w #$0010                ; $80FAF8
    ldx.w #$0000                ; $80FAFB
CODE_80FAFE:
    lda.w $DB4C,x               ; $80FAFE
    sec                         ; $80FB01
    sbc.b $14                   ; $80FB02
    bcs CODE_80FB09             ; $80FB04
    lda.w #$0000                ; $80FB06
CODE_80FB09:
    sta.b $16                   ; $80FB09
    lda.w $DB6C,x               ; $80FB0B
    sec                         ; $80FB0E
    sbc.b $14                   ; $80FB0F
    bcs CODE_80FB16             ; $80FB11
    lda.w #$0000                ; $80FB13
CODE_80FB16:
    xba                         ; $80FB16
    lsr a                       ; $80FB17
    lsr a                       ; $80FB18
    lsr a                       ; $80FB19
    ora.b $16                   ; $80FB1A
    sta.b $16                   ; $80FB1C
    lda.w $DB8C,x               ; $80FB1E
    sec                         ; $80FB21
    sbc.b $14                   ; $80FB22
    bcs CODE_80FB29             ; $80FB24
    lda.w #$0000                ; $80FB26
CODE_80FB29:
    xba                         ; $80FB29
    asl a                       ; $80FB2A
    asl a                       ; $80FB2B
    ora.b $16                   ; $80FB2C
    sta.w $0500,x               ; $80FB2E
    inx                         ; $80FB31
    inx                         ; $80FB32
    dey                         ; $80FB33
    bne CODE_80FAFE             ; $80FB34
    lda.w #$2202                ; $80FB36
    sta.w $4300                 ; $80FB39
    lda.w #$0500                ; $80FB3C
    sta.w $4302                 ; $80FB3F
    lda.w #$0020                ; $80FB42
    sta.w $4305                 ; $80FB45
    sep.b #$20                  ; $80FB48
    lda.b #$60                  ; $80FB4A
    sta.w $2121                 ; $80FB4C
    lda.b #$81                  ; $80FB4F
    sta.w $4304                 ; $80FB51
    lda.b #$01                  ; $80FB54
    sta.w $420B                 ; $80FB56
    rep.b #$20                  ; $80FB59
    rts                         ; $80FB5B
    incbin !baserom:$7B5C..$7BA0
    jsl.l $82FB70               ; $80FBA0
    rts                         ; $80FBA4
    sta.w $1536                 ; $80FBA5
    phb                         ; $80FBA8
    pea.w $8A8A                 ; $80FBA9
    plb                         ; $80FBAC
    plb                         ; $80FBAD
    pha                         ; $80FBAE
    lda.w #$0100                ; $80FBAF
    tcd                         ; $80FBB2
    pla                         ; $80FBB3
    ldx.w #$EB44                ; $80FBB4
    ldy.w #$8000                ; $80FBB7
    lda.w #$7E9A                ; $80FBBA
    jsl.l $82FB70               ; $80FBBD
    jsl.l $8A800B               ; $80FBC1
    sep.b #$20                  ; $80FBC5
    stz.w $420C                 ; $80FBC7
    stz.w $420B                 ; $80FBCA
    rep.b #$20                  ; $80FBCD
    plb                         ; $80FBCF
    rtl                         ; $80FBD0
    jsl.l $8A8000               ; $80FBD1
    jmp.w $8934                 ; $80FBD5
    jsr.w $83E8                 ; $80FBD8
    rtl                         ; $80FBDB
    rep.b #$30                  ; $80FBDC
    phb                         ; $80FBDE
    pea.w $8A8A                 ; $80FBDF
    plb                         ; $80FBE2
    plb                         ; $80FBE3
    lda.w #$0100                ; $80FBE4
    tcd                         ; $80FBE7
    jsl.l $8A94C3               ; $80FBE8
    plb                         ; $80FBEC
    sep.b #$20                  ; $80FBED
    stz.w $420C                 ; $80FBEF
    stz.w $420B                 ; $80FBF2
    rep.b #$20                  ; $80FBF5
    rtl                         ; $80FBF7
    incbin !baserom:$7BF8..$7C14
    jml.l $8A968E               ; $80FC14
    jmp.w $8934                 ; $80FC18
    incbin !baserom:$7C1B..$7C20
    ldx.w #$0002                ; $80FC20
    bra CODE_80FC2D             ; $80FC23
    ldx.w #$0001                ; $80FC25
    bra CODE_80FC2D             ; $80FC28
    ldx.w #$0000                ; $80FC2A
CODE_80FC2D:
    sep.b #$20                  ; $80FC2D
    stz.w $420C                 ; $80FC2F
    stz.w $420B                 ; $80FC32
    rep.b #$30                  ; $80FC35
    phb                         ; $80FC37
    pea.w $8A8A                 ; $80FC38
    plb                         ; $80FC3B
    plb                         ; $80FC3C
    lda.w #$0100                ; $80FC3D
    tcd                         ; $80FC40
    jsl.l $8A9E5B               ; $80FC41
    plb                         ; $80FC45
    jsl.l $8A8001               ; $80FC46
    ldx.w #$00FE                ; $80FC4A
CODE_80FC4D:
    stz.w $0D00,x               ; $80FC4D
    dex                         ; $80FC50
    dex                         ; $80FC51
    bpl CODE_80FC4D             ; $80FC52
    sep.b #$20                  ; $80FC54
    stz.w $420C                 ; $80FC56
    stz.w $420B                 ; $80FC59
    rep.b #$20                  ; $80FC5C
    rtl                         ; $80FC5E
    lda.w #$0002                ; $80FC5F
    tsb.w $0026                 ; $80FC62
    rep.b #$30                  ; $80FC65
    phb                         ; $80FC67
    pea.w $8A8A                 ; $80FC68
    plb                         ; $80FC6B
    plb                         ; $80FC6C
    lda.w #$0100                ; $80FC6D
    tcd                         ; $80FC70
    jsl.l $8AA050               ; $80FC71
    plb                         ; $80FC75
    lda.w #$0000                ; $80FC76
    tcd                         ; $80FC79
    jmp.w $8934                 ; $80FC7A
    incbin !baserom:$7C7D..$8000
