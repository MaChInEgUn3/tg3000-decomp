; bank $82 (file offset $010000)
org $828000

    incbin !baserom:$10000..$10342
    sei                         ; $828342
    phd                         ; $828343
    phb                         ; $828344
    pea.w $8383                 ; $828345
    plb                         ; $828348
    plb                         ; $828349
    sep.b #$20                  ; $82834A
    lda.b #$81                  ; $82834C
    sta.w $4200                 ; $82834E
    rep.b #$20                  ; $828351
    stz.w $420B                 ; $828353
    lda.w #$0001                ; $828356
    jsr.w $841E                 ; $828359
    lda.w #$8623                ; $82835C
    sta.w $0001                 ; $82835F
    sep.b #$20                  ; $828362
    lda.b #$83                  ; $828364
    sta.w $0003                 ; $828366
    rep.b #$20                  ; $828369
    ldx.w #$000C                ; $82836B
    lda.w #$0000                ; $82836E
CODE_828371:
    sta.w $0000,x               ; $828371
    inx                         ; $828374
    inx                         ; $828375
    cpx.w #$0100                ; $828376
    bne CODE_828371             ; $828379
    ldx.w #$0000                ; $82837B
    lda.w #$0000                ; $82837E
CODE_828381:
    sta.w $0200,x               ; $828381
    inx                         ; $828384
    inx                         ; $828385
    cpx.w #$0100                ; $828386
    bne CODE_828381             ; $828389
    jsl.l $85831C               ; $82838B
    stz.w $0196                 ; $82838F
    stz.w $019A                 ; $828392
    lda.w #$0000                ; $828395
    sta.l $7E4CD9               ; $828398
    dec a                       ; $82839C
    sta.l $7E4CDB               ; $82839D
    ldx.w #$0020                ; $8283A1
    lda.w #$0000                ; $8283A4
CODE_8283A7:
    sta.l $7E4CDB,x             ; $8283A7
    dex                         ; $8283AB
    dex                         ; $8283AC
    bne CODE_8283A7             ; $8283AD
    lda.l $8103DC               ; $8283AF
    and.w #$0003                ; $8283B3
    asl a                       ; $8283B6
    tax                         ; $8283B7
    jsr.w ($8418,x)             ; $8283B8
    incbin !baserom:$103BB..$1041E
    clc                         ; $82841E
    adc.w $0024                 ; $82841F
CODE_828422:
    cmp.w $0024                 ; $828422
    bne CODE_828422             ; $828425
    rts                         ; $828427
    ldx.w #$96DD                ; $828428
    lda.w #$0083                ; $82842B
    jsl.l $8081C8               ; $82842E
    lda.w #$838A                ; $828432
    sta.w $0008                 ; $828435
    sep.b #$20                  ; $828438
    ldx.w #$95BD                ; $82843A
    stx.w $0005                 ; $82843D
    lda.b #$85                  ; $828440
    sta.w $0007                 ; $828442
    ldx.w #$8377                ; $828445
    stx.w $0001                 ; $828448
    lda.b #$85                  ; $82844B
    sta.w $0003                 ; $82844D
    rep.b #$20                  ; $828450
    sep.b #$20                  ; $828452
    lda.b #$25                  ; $828454
    sta.w $2132                 ; $828456
    lda.b #$4B                  ; $828459
    sta.w $2132                 ; $82845B
    lda.b #$8A                  ; $82845E
    sta.w $2132                 ; $828460
    rep.b #$20                  ; $828463
    jsl.l $85831C               ; $828465
    lda.w $047B                 ; $828469
    jsl.l $858000               ; $82846C
    jsr.w $A28B                 ; $828470
    lda.w #$0000                ; $828473
    sta.l $7EFAFA               ; $828476
    lda.w #$FF7D                ; $82847A
    and.l $7EFB13               ; $82847D
    sta.l $7EFB13               ; $828481
    phb                         ; $828485
    ldx.w #$8AAE                ; $828486
    ldy.w #$4CCC                ; $828489
    lda.w #$000C                ; $82848C
    mvn $7E,$85                 ; $82848F
    plb                         ; $828492
    jsr.w $A954                 ; $828493
    ldx.w #$85F5                ; $828496
    ldy.w #$5AC0                ; $828499
    lda.w #$008F                ; $82849C
    jsl.l $80837D               ; $82849F
    lda.w $01E4                 ; $8284A3
    bmi CODE_8284C8             ; $8284A6
    incbin !baserom:$104A8..$104C8
CODE_8284C8:
    ldx.w #$9F2A                ; $8284C8
    ldy.w #$0000                ; $8284CB
    lda.w #$009D                ; $8284CE
    jsl.l $80837D               ; $8284D1
    ldx.w #$8B3D                ; $8284D5
    ldy.w #$0000                ; $8284D8
    lda.w #$7F87                ; $8284DB
    jsl.l $82FB70               ; $8284DE
    lda.w $1F06                 ; $8284E2
    asl a                       ; $8284E5
    tax                         ; $8284E6
    lda.l $83A484,x             ; $8284E7
    sta.l $7F0078               ; $8284EB
    lda.w #$30F4                ; $8284EF
    jsr.w $AD99                 ; $8284F2
    lda.w #$007F                ; $8284F5
    sta.b $14                   ; $8284F8
    ldx.w #$0000                ; $8284FA
    ldy.w #$7C00                ; $8284FD
    lda.w #$0700                ; $828500
    jsl.l $8083AD               ; $828503
    jsr.w $A1D3                 ; $828507
    jsr.w $ADC2                 ; $82850A
    lda.w #$007E                ; $82850D
    sta.b $14                   ; $828510
    ldx.w #$217C                ; $828512
    ldy.w #$57A0                ; $828515
    lda.w #$0190                ; $828518
    jsl.l $8083AD               ; $82851B
    lda.w #$007E                ; $82851F
    sta.b $14                   ; $828522
    ldx.w #$217C                ; $828524
    ldy.w #$5868                ; $828527
    lda.w #$0190                ; $82852A
    jsl.l $8083AD               ; $82852D
    lda.w #$8D00                ; $828531
    ldx.w #$8B70                ; $828534
    ldy.w #$0040                ; $828537
    jsl.l $8083CB               ; $82853A
    ldx.w #$B3D5                ; $82853E
    ldy.w #$0EE0                ; $828541
    lda.w #$0099                ; $828544
    jsl.l $80837D               ; $828547
    lda.w $047C                 ; $82854B
    bmi CODE_82856D             ; $82854E
    jsl.l $84A4CD               ; $828550
    jsl.l $84A487               ; $828554
    sep.b #$20                  ; $828558
    lda.b #$7F                  ; $82855A
    sta.w $045C                 ; $82855C
    lda.w $03DD                 ; $82855F
    and.b #$04                  ; $828562
    lsr a                       ; $828564
    lsr a                       ; $828565
    eor.b #$01                  ; $828566
    sta.w $045F                 ; $828568
    rep.b #$20                  ; $82856B
CODE_82856D:
    lda.w #$0010                ; $82856D
    sta.w $002E                 ; $828570
    lda.w #$0000                ; $828573
    sta.w $0030                 ; $828576
    stz.w $0212                 ; $828579
    stz.w $02C5                 ; $82857C
    stz.w $003E                 ; $82857F
    stz.w $0040                 ; $828582
    stz.b $3E                   ; $828585
    stz.b $40                   ; $828587
    stz.w $005A                 ; $828589
    stz.w $005C                 ; $82858C
    stz.w $005D                 ; $82858F
    stz.w $005F                 ; $828592
    lda.w #$FFFF                ; $828595
    sta.w $0235                 ; $828598
    sta.w $02E8                 ; $82859B
    lda.w #$0003                ; $82859E
    sta.w $0060                 ; $8285A1
    sta.w $0062                 ; $8285A4
    lda.w #$000C                ; $8285A7
    sta.w $0142                 ; $8285AA
    lda.w #$000B                ; $8285AD
    sta.w $0144                 ; $8285B0
    stz.b $5A                   ; $8285B3
    stz.b $0D                   ; $8285B5
    stz.b $4C                   ; $8285B7
    stz.b $4E                   ; $8285B9
    stz.b $50                   ; $8285BB
    stz.b $52                   ; $8285BD
    stz.b $54                   ; $8285BF
    stz.b $56                   ; $8285C1
    stz.b $58                   ; $8285C3
    jsl.l $859D7B               ; $8285C5
    jsr.w $A7F9                 ; $8285C9
    jsl.l $81E631               ; $8285CC
    jsr.w $AF33                 ; $8285D0
    jsr.w $8961                 ; $8285D3
    lda.w #$0001                ; $8285D6
    jsr.w $841E                 ; $8285D9
    stz.w $4315                 ; $8285DC
    stz.w $4325                 ; $8285DF
    stz.w $4335                 ; $8285E2
    lda.w #$83CE                ; $8285E5
    sta.w $0008                 ; $8285E8
    lda.w #$8377                ; $8285EB
    sta.w $0001                 ; $8285EE
    lda.w #$95BD                ; $8285F1
    sta.w $0005                 ; $8285F4
    sep.b #$20                  ; $8285F7
    lda.b #$85                  ; $8285F9
    sta.w $0003                 ; $8285FB
    lda.b #$85                  ; $8285FE
    sta.w $0007                 ; $828600
    rep.b #$20                  ; $828603
    jsr.w $DFA9                 ; $828605
    jsl.l $85B36F               ; $828608
    lda.w #$0F00                ; $82860C
    sta.w $0148                 ; $82860F
    lda.w #$0080                ; $828612
    sta.w $0146                 ; $828615
    lda.w #$8001                ; $828618
    sta.w $1ECE                 ; $82861B
    stz.w $00BA                 ; $82861E
    lda.w #$0001                ; $828621
    sta.w $1E9E                 ; $828624
    lda.w #$0001                ; $828627
    jsr.w $841E                 ; $82862A
    lda.w #$0012                ; $82862D
    ldx.w #$4000                ; $828630
    jsr.w $AD12                 ; $828633
    cli                         ; $828636
    lda.w #$003D                ; $828637
    bit.w $047C                 ; $82863A
    bpl CODE_828642             ; $82863D
    incbin !baserom:$1063F..$10642
CODE_828642:
    sta.w $0233                 ; $828642
    jsr.w $AB9D                 ; $828645
    jsl.l $83E241               ; $828648
    jsl.l $81E812               ; $82864C
    jsr.w $AC52                 ; $828650
    lda.w #$0001                ; $828653
CODE_828656:
    bit.w $00B8                 ; $828656
    bne CODE_828656             ; $828659
    jsr.w $F2B3                 ; $82865B
    jsr.w $DFA9                 ; $82865E
    jsr.w $EC32                 ; $828661
    jsr.w $AD2C                 ; $828664
    lda.w $00B8                 ; $828667
    bit.w #$0004                ; $82866A
    bne CODE_828678             ; $82866D
    jsr.w $B4ED                 ; $82866F
    lda.w #$0004                ; $828672
    tsb.w $00B8                 ; $828675
CODE_828678:
    jsr.w $8923                 ; $828678
    lda.w #$0001                ; $82867B
    tsb.w $00B8                 ; $82867E
    ldx.w $00BA                 ; $828681
    bit.w $047C                 ; $828684
    bpl CODE_8286AB             ; $828687
    incbin !baserom:$10689..$106AB
CODE_8286AB:
    lda.w #$1000                ; $8286AB
    trb.w $000E                 ; $8286AE
    beq CODE_8286BE             ; $8286B1
    incbin !baserom:$106B3..$106BE
CODE_8286BE:
    lda.w $03E2                 ; $8286BE
    bit.w #$0004                ; $8286C1
    beq CODE_8286D1             ; $8286C4
    incbin !baserom:$106C6..$106D1
CODE_8286D1:
    stz.w $00BA                 ; $8286D1
    stx.w $1E9E                 ; $8286D4
    lda.w $0244                 ; $8286D7
    bit.w #$0004                ; $8286DA
    db $D0,$08 ; bne            ; $8286DD
    lda.w $1F08                 ; $8286DF
    cmp.w #$000A                ; $8286E2
    bcc CODE_828766             ; $8286E5
    incbin !baserom:$106E7..$10766
CODE_828766:
    lda.w $0146                 ; $828766
    bne CODE_828773             ; $828769
    lda.w $014A                 ; $82876B
    and.w #$0F00                ; $82876E
    db $F0,$03 ; beq            ; $828771
CODE_828773:
    jmp.w $8645                 ; $828773
    incbin !baserom:$10776..$10923
    stz.b $86                   ; $828923
    stz.b $8E                   ; $828925
    lda.w $00B4                 ; $828927
    beq CODE_828933             ; $82892A
    lda.w #$0220                ; $82892C
    sta.b $86                   ; $82892F
    sta.b $8E                   ; $828931
CODE_828933:
    jsl.l $85B3A9               ; $828933
    jsl.l $818564               ; $828937
    jsr.w $8F8F                 ; $82893B
    jsr.w $CBD4                 ; $82893E
    jsr.w $A9B7                 ; $828941
    jmp.w $AE15                 ; $828944
    incbin !baserom:$10947..$10961
    lda.w #$7C57                ; $828961
    sta.w $2116                 ; $828964
    ldx.w #$0000                ; $828967
    ldy.w #$0157                ; $82896A
CODE_82896D:
    lda.l $810398,x             ; $82896D
    and.w #$00FF                ; $828971
    beq CODE_82897B             ; $828974
    incbin !baserom:$10976..$1097B
CODE_82897B:
    sty.w $2118                 ; $82897B
    inx                         ; $82897E
    cpx.w #$0004                ; $82897F
    bne CODE_82896D             ; $828982
    lda.w $03A7                 ; $828984
    and.w #$00FF                ; $828987
    beq CODE_8289B2             ; $82898A
    incbin !baserom:$1098C..$109B2
CODE_8289B2:
    jmp.w $B4ED                 ; $8289B2
    incbin !baserom:$109B5..$10F8F
    lda.w $0231                 ; $828F8F
    beq CODE_828FAE             ; $828F92
    incbin !baserom:$10F94..$10FAE
CODE_828FAE:
    lda.w #$8000                ; $828FAE
    sta.w $1F18                 ; $828FB1
    rts                         ; $828FB4
    incbin !baserom:$10FB5..$11BC9
    pha                         ; $829BC9
    asl a                       ; $829BCA
    pha                         ; $829BCB
    lda.w $0233                 ; $829BCC
    asl a                       ; $829BCF
    tax                         ; $829BD0
    lda.l $83A0A4,x             ; $829BD1
    plx                         ; $829BD5
    cmp.l $83A0A4,x             ; $829BD6
    beq CODE_829BDE             ; $829BDA
    incbin !baserom:$11BDC..$11BDE
CODE_829BDE:
    pla                         ; $829BDE
    sta.w $0233                 ; $829BDF
    rts                         ; $829BE2
    incbin !baserom:$11BE3..$11BE5
    pha                         ; $829BE5
    asl a                       ; $829BE6
    pha                         ; $829BE7
    lda.w $02E6                 ; $829BE8
    asl a                       ; $829BEB
    tax                         ; $829BEC
    lda.l $83A0A4,x             ; $829BED
    plx                         ; $829BF1
    cmp.l $83A0A4,x             ; $829BF2
    beq CODE_829BFA             ; $829BF6
    db $B0,$05 ; bcs            ; $829BF8
CODE_829BFA:
    pla                         ; $829BFA
    sta.w $02E6                 ; $829BFB
    rts                         ; $829BFE
    incbin !baserom:$11BFF..$121D3
    ldx.w #$9FEB                ; $82A1D3
    ldy.w #$0000                ; $82A1D6
    lda.w #$7F98                ; $82A1D9
    jsl.l $82FB70               ; $82A1DC
    ldx.w $047B                 ; $82A1E0
    lda.l $84A66C,x             ; $82A1E3
    and.w #$00FF                ; $82A1E7
    asl a                       ; $82A1EA
    asl a                       ; $82A1EB
    tax                         ; $82A1EC
    lda.l $85A33A,x             ; $82A1ED
    sta.w $1F28                 ; $82A1F1
    lda.l $85A33C,x             ; $82A1F4
    sta.w $1F2A                 ; $82A1F8
    txa                         ; $82A1FB
    asl a                       ; $82A1FC
    tax                         ; $82A1FD
    phx                         ; $82A1FE
    lda.w #$007F                ; $82A1FF
    sta.b $14                   ; $82A202
    lda.l $85A346,x             ; $82A204
    pha                         ; $82A208
    lda.l $85A348,x             ; $82A209
    ldy.w #$3800                ; $82A20D
    plx                         ; $82A210
    jsl.l $8083AD               ; $82A211
    plx                         ; $82A215
    phx                         ; $82A216
    lda.w #$007F                ; $82A217
    sta.b $14                   ; $82A21A
    lda.l $85A34A,x             ; $82A21C
    tax                         ; $82A220
    ldy.w #$3000                ; $82A221
    lda.w #$0600                ; $82A224
    jsl.l $8083AD               ; $82A227
    plx                         ; $82A22B
    phx                         ; $82A22C
    lda.w #$007F                ; $82A22D
    sta.b $14                   ; $82A230
    lda.l $85A34C,x             ; $82A232
    tax                         ; $82A236
    ldy.w #$3400                ; $82A237
    lda.w #$0600                ; $82A23A
    jsl.l $8083AD               ; $82A23D
    plx                         ; $82A241
    phx                         ; $82A242
    lda.l $85A34A,x             ; $82A243
    tax                         ; $82A247
    ldy.w #$0600                ; $82A248
CODE_82A24B:
    lda.l $7F0000,x             ; $82A24B
    and.w #$DFFF                ; $82A24F
    clc                         ; $82A252
    adc.w #$0400                ; $82A253
    sta.l $7F0000,x             ; $82A256
    inx                         ; $82A25A
    inx                         ; $82A25B
    dey                         ; $82A25C
    bne CODE_82A24B             ; $82A25D
    plx                         ; $82A25F
    phx                         ; $82A260
    lda.w #$007F                ; $82A261
    sta.b $14                   ; $82A264
    lda.l $85A34A,x             ; $82A266
    tax                         ; $82A26A
    ldy.w #$7400                ; $82A26B
    lda.w #$0600                ; $82A26E
    jsl.l $8083AD               ; $82A271
    plx                         ; $82A275
    lda.w #$007F                ; $82A276
    sta.b $14                   ; $82A279
    lda.l $85A34C,x             ; $82A27B
    tax                         ; $82A27F
    ldy.w #$7800                ; $82A280
    lda.w #$0600                ; $82A283
    jsl.l $8083AD               ; $82A286
    rts                         ; $82A28A
    stz.w $00B4                 ; $82A28B
    stz.w $00B6                 ; $82A28E
    stz.w $00B8                 ; $82A291
    lda.w #$0040                ; $82A294
    sta.w $028B                 ; $82A297
    lda.w #$FFC0                ; $82A29A
    sta.w $028D                 ; $82A29D
    lda.w $03DC                 ; $82A2A0
    and.w #$0030                ; $82A2A3
    cmp.w #$0030                ; $82A2A6
    beq CODE_82A2C5             ; $82A2A9
    lda.w $0393                 ; $82A2AB
    sta.w $03C2                 ; $82A2AE
    lda.w $0395                 ; $82A2B1
    sta.w $03C4                 ; $82A2B4
    lda.w $0393                 ; $82A2B7
    and.w #$00FF                ; $82A2BA
    cmp.w #$0005                ; $82A2BD
    beq CODE_82A2C5             ; $82A2C0
    inc.w $03C2                 ; $82A2C2
CODE_82A2C5:
    stz.w $1ED0                 ; $82A2C5
    stz.w $025C                 ; $82A2C8
    stz.w $030F                 ; $82A2CB
    lda.w #$001C                ; $82A2CE
    sta.w $027C                 ; $82A2D1
    lda.w #$001C                ; $82A2D4
    sta.w $032F                 ; $82A2D7
    lda.w #$FFFF                ; $82A2DA
    sta.w $1F18                 ; $82A2DD
    sta.w $1F1A                 ; $82A2E0
    sta.w $1F24                 ; $82A2E3
    sta.w $1F26                 ; $82A2E6
    stz.w $0216                 ; $82A2E9
    stz.w $0218                 ; $82A2EC
    stz.w $02C9                 ; $82A2EF
    stz.w $02CB                 ; $82A2F2
    stz.w $0278                 ; $82A2F5
    stz.w $027A                 ; $82A2F8
    stz.w $032B                 ; $82A2FB
    stz.w $032D                 ; $82A2FE
    stz.w $0253                 ; $82A301
    stz.w $0306                 ; $82A304
    stz.w $0223                 ; $82A307
    stz.w $02D6                 ; $82A30A
    stz.w $0285                 ; $82A30D
    stz.w $0338                 ; $82A310
    stz.w $1F08                 ; $82A313
    stz.w $1F0A                 ; $82A316
    stz.w $1F0C                 ; $82A319
    lda.w #$0000                ; $82A31C
    sta.l $7E4E3D               ; $82A31F
    sta.l $7E4E3F               ; $82A323
    sta.l $7E4E41               ; $82A327
    sta.l $7E4E43               ; $82A32B
    lda.w #$0001                ; $82A32F
    sta.w $0233                 ; $82A332
    sta.w $02E6                 ; $82A335
    sta.w $0295                 ; $82A338
    sta.w $0348                 ; $82A33B
    stz.w $1F0E                 ; $82A33E
    stz.w $1F10                 ; $82A341
    stz.w $1F12                 ; $82A344
    stz.w $1F14                 ; $82A347
    stz.w $0231                 ; $82A34A
    stz.w $02E4                 ; $82A34D
    lda.w #$0028                ; $82A350
    sta.w $0258                 ; $82A353
    sta.w $030B                 ; $82A356
    lda.w #$FFFF                ; $82A359
    sta.w $0256                 ; $82A35C
    sta.w $0309                 ; $82A35F
    stz.w $025A                 ; $82A362
    stz.w $030D                 ; $82A365
    lda.w #$00FF                ; $82A368
    trb.w $1F32                 ; $82A36B
    lda.w #$8000                ; $82A36E
    sta.w $1E94                 ; $82A371
    stz.w $1ECE                 ; $82A374
    stz.w $021F                 ; $82A377
    stz.w $02D2                 ; $82A37A
    lda.w #$0001                ; $82A37D
    sta.w $0244                 ; $82A380
    sta.w $02F7                 ; $82A383
    stz.w $0221                 ; $82A386
    stz.w $02D4                 ; $82A389
    stz.w $021A                 ; $82A38C
    stz.w $02CD                 ; $82A38F
    stz.w $0212                 ; $82A392
    stz.w $0214                 ; $82A395
    stz.w $02C5                 ; $82A398
    stz.w $02C7                 ; $82A39B
    stz.w $1E5E                 ; $82A39E
    stz.w $1E62                 ; $82A3A1
    stz.w $1E60                 ; $82A3A4
    stz.w $1E64                 ; $82A3A7
    stz.w $1E54                 ; $82A3AA
    stz.w $1E5A                 ; $82A3AD
    lda.w #$0300                ; $82A3B0
    sta.w $023A                 ; $82A3B3
    sta.w $02ED                 ; $82A3B6
    lda.w #$000C                ; $82A3B9
    sta.w $00E8                 ; $82A3BC
    lda.w #$0008                ; $82A3BF
    sta.w $00E6                 ; $82A3C2
    lda.w #$0004                ; $82A3C5
    sta.w $00E4                 ; $82A3C8
    lda.w #$0000                ; $82A3CB
    sta.w $00E0                 ; $82A3CE
    lda.w #$06C2                ; $82A3D1
    sta.w $1E2E                 ; $82A3D4
    lda.w #$06E0                ; $82A3D7
    sta.w $1E32                 ; $82A3DA
    lda.w #$06FE                ; $82A3DD
    sta.w $1E36                 ; $82A3E0
    lda.w #$071C                ; $82A3E3
    sta.w $1E3A                 ; $82A3E6
    lda.w #$073A                ; $82A3E9
    sta.w $1E30                 ; $82A3EC
    lda.w #$0758                ; $82A3EF
    sta.w $1E34                 ; $82A3F2
    lda.w #$0776                ; $82A3F5
    sta.w $1E38                 ; $82A3F8
    lda.w #$0794                ; $82A3FB
    sta.w $1E3C                 ; $82A3FE
    stz.w $00BE                 ; $82A401
    stz.w $00C0                 ; $82A404
    stz.w $00BC                 ; $82A407
    stz.w $0486                 ; $82A40A
    stz.w $00C4                 ; $82A40D
    stz.w $00C6                 ; $82A410
    stz.w $00C2                 ; $82A413
    stz.w $050E                 ; $82A416
    stz.w $0176                 ; $82A419
    stz.w $017A                 ; $82A41C
    stz.w $0208                 ; $82A41F
    stz.w $02BB                 ; $82A422
    stz.w $026A                 ; $82A425
    stz.w $031D                 ; $82A428
    stz.w $0200                 ; $82A42B
    stz.w $0202                 ; $82A42E
    stz.w $02B3                 ; $82A431
    stz.w $02B5                 ; $82A434
    stz.w $0262                 ; $82A437
    stz.w $0264                 ; $82A43A
    stz.w $0315                 ; $82A43D
    stz.w $0317                 ; $82A440
    stz.w $023E                 ; $82A443
    stz.w $02F1                 ; $82A446
    stz.w $0210                 ; $82A449
    stz.w $02C3                 ; $82A44C
    stz.w $0272                 ; $82A44F
    stz.w $0325                 ; $82A452
    stz.w $0225                 ; $82A455
    stz.w $0227                 ; $82A458
    stz.w $02D8                 ; $82A45B
    stz.w $02DA                 ; $82A45E
    stz.w $0242                 ; $82A461
    stz.w $02F5                 ; $82A464
    lda.w #$8000                ; $82A467
    sta.w $027E                 ; $82A46A
    sta.w $0331                 ; $82A46D
    sep.b #$20                  ; $82A470
    lda.b #$04                  ; $82A472
    sta.w $024E                 ; $82A474
    sta.w $0301                 ; $82A477
    stz.w $024F                 ; $82A47A
    stz.w $0302                 ; $82A47D
    sta.w $0251                 ; $82A480
    sta.w $0304                 ; $82A483
    sta.w $0252                 ; $82A486
    sta.w $0305                 ; $82A489
    lda.b #$96                  ; $82A48C
    sta.w $0250                 ; $82A48E
    sta.w $0303                 ; $82A491
    stz.w $0280                 ; $82A494
    stz.w $0333                 ; $82A497
    rep.b #$20                  ; $82A49A
    stz.w $014C                 ; $82A49C
    stz.w $014E                 ; $82A49F
    lda.w $024E                 ; $82A4A2
    and.w #$00FF                ; $82A4A5
    sta.w $025C                 ; $82A4A8
    lda.w $0301                 ; $82A4AB
    and.w #$00FF                ; $82A4AE
    sta.w $030F                 ; $82A4B1
    stz.w $0240                 ; $82A4B4
    stz.w $02F3                 ; $82A4B7
    stz.w $003E                 ; $82A4BA
    stz.w $0040                 ; $82A4BD
    stz.w $00FE                 ; $82A4C0
    stz.w $0100                 ; $82A4C3
    stz.w $01A0                 ; $82A4C6
    stz.w $01A2                 ; $82A4C9
    stz.w $01A4                 ; $82A4CC
    stz.w $01A6                 ; $82A4CF
    stz.w $1E02                 ; $82A4D2
    stz.w $1E0A                 ; $82A4D5
    stz.w $0204                 ; $82A4D8
    stz.w $0206                 ; $82A4DB
    stz.w $02B7                 ; $82A4DE
    stz.w $02B9                 ; $82A4E1
    stz.w $020A                 ; $82A4E4
    stz.w $02BD                 ; $82A4E7
    stz.w $020C                 ; $82A4EA
    stz.w $02BF                 ; $82A4ED
    lda.w #$18A6                ; $82A4F0
    ldx.w #$0000                ; $82A4F3
    ldy.w #$0004                ; $82A4F6
CODE_82A4F9:
    sta.l $7E272C,x             ; $82A4F9
    sta.l $7E282C,x             ; $82A4FD
    sta.l $7E2B2C,x             ; $82A501
    sta.l $7E2C2C,x             ; $82A505
    sta.l $7E292C,x             ; $82A509
    sta.l $7E2A2C,x             ; $82A50D
    sta.l $7E2D2C,x             ; $82A511
    sta.l $7E2E2C,x             ; $82A515
    inx                         ; $82A519
    inx                         ; $82A51A
    dey                         ; $82A51B
    bne CODE_82A4F9             ; $82A51C
    lda.l $7E6A33               ; $82A51E
    tax                         ; $82A522
    and.w #$001F                ; $82A523
    sec                         ; $82A526
    sbc.w #$0006                ; $82A527
    ora.w #$0020                ; $82A52A
    sta.l $7E4E55               ; $82A52D
    txa                         ; $82A531
    and.w #$03E0                ; $82A532
    lsr a                       ; $82A535
    lsr a                       ; $82A536
    lsr a                       ; $82A537
    lsr a                       ; $82A538
    lsr a                       ; $82A539
    sec                         ; $82A53A
    sbc.w #$0007                ; $82A53B
    ora.w #$0040                ; $82A53E
    sta.l $7E4E57               ; $82A541
    txa                         ; $82A545
    and.w #$7C00                ; $82A546
    lsr a                       ; $82A549
    lsr a                       ; $82A54A
    xba                         ; $82A54B
    sec                         ; $82A54C
    sbc.w #$0008                ; $82A54D
    ora.w #$0080                ; $82A550
    sta.l $7E4E59               ; $82A553
    lda.l $7E6B33               ; $82A557
    tax                         ; $82A55B
    and.w #$001F                ; $82A55C
    sec                         ; $82A55F
    sbc.w #$0006                ; $82A560
    sta.l $7E4E5B               ; $82A563
    txa                         ; $82A567
    and.w #$03E0                ; $82A568
    lsr a                       ; $82A56B
    lsr a                       ; $82A56C
    lsr a                       ; $82A56D
    lsr a                       ; $82A56E
    lsr a                       ; $82A56F
    sec                         ; $82A570
    sbc.w #$0007                ; $82A571
    sta.l $7E4E5D               ; $82A574
    txa                         ; $82A578
    and.w #$7C00                ; $82A579
    lsr a                       ; $82A57C
    lsr a                       ; $82A57D
    xba                         ; $82A57E
    sec                         ; $82A57F
    sbc.w #$0008                ; $82A580
    sta.l $7E4E5F               ; $82A583
    ldy.w #$0000                ; $82A587
    ldx.w #$0000                ; $82A58A
CODE_82A58D:
    lda.w #$0000                ; $82A58D
    sta.l $308000               ; $82A590
    lda.w $0446,y               ; $82A594
    and.w #$00FF                ; $82A597
    sta.l $308000               ; $82A59A
    lda.w #$0035                ; $82A59E
    sta.l $308000               ; $82A5A1
    nop                         ; $82A5A5
    lda.l $308000               ; $82A5A6
    sta.w $1E6A,x               ; $82A5AA
    lda.l $308000               ; $82A5AD
    inx                         ; $82A5B1
    inx                         ; $82A5B2
    iny                         ; $82A5B3
    cpy.w #$0014                ; $82A5B4
    bne CODE_82A58D             ; $82A5B7
    jsl.l $859ADB               ; $82A5B9
    lda.w #$A411                ; $82A5BD
    sta.w $00EA                 ; $82A5C0
    phb                         ; $82A5C3
    ldx.w #$A411                ; $82A5C4
    ldy.w #$4CCC                ; $82A5C7
    lda.w #$0010                ; $82A5CA
    mvn $7E,$83                 ; $82A5CD
    plb                         ; $82A5D0
    phb                         ; $82A5D1
    lda.w #$0000                ; $82A5D2
    sta.l $308000               ; $82A5D5
    lda.w #$001E                ; $82A5D9
    sta.l $308000               ; $82A5DC
    lda.w $0392                 ; $82A5E0
    and.w #$00FF                ; $82A5E3
    sta.l $308000               ; $82A5E6
    lda.w #$A118                ; $82A5EA
    clc                         ; $82A5ED
    adc.l $308000               ; $82A5EE
    sta.l $308000               ; $82A5F2
    tax                         ; $82A5F6
    ldy.w #$2140                ; $82A5F7
    lda.w #$001D                ; $82A5FA
    mvn $7E,$85                 ; $82A5FD
    lda.w #$0000                ; $82A600
    sta.l $308000               ; $82A603
    lda.w #$001E                ; $82A607
    sta.l $308000               ; $82A60A
    lda.w $03C1                 ; $82A60E
    and.w #$00FF                ; $82A611
    sta.l $308000               ; $82A614
    lda.w #$A118                ; $82A618
    clc                         ; $82A61B
    adc.l $308000               ; $82A61C
    sta.l $308000               ; $82A620
    tax                         ; $82A624
    ldy.w #$215E                ; $82A625
    lda.w #$001D                ; $82A628
    mvn $7E,$85                 ; $82A62B
    plb                         ; $82A62E
    jsr.w $A672                 ; $82A62F
    stz.w $1E66                 ; $82A632
    stz.w $1E68                 ; $82A635
    ldx.w #$0000                ; $82A638
    ldy.w #$0000                ; $82A63B
    pea.w $837E                 ; $82A63E
    plb                         ; $82A641
    lda.w #$0028                ; $82A642
    sta.b $14                   ; $82A645
CODE_82A647:
    lda.l $97A2B8,x             ; $82A647
    sta.w $2000,y               ; $82A64B
    sta.w $2002,y               ; $82A64E
    sta.w $2004,y               ; $82A651
    sta.w $2006,y               ; $82A654
    tya                         ; $82A657
    clc                         ; $82A658
    adc.w #$0008                ; $82A659
    tay                         ; $82A65C
    inx                         ; $82A65D
    inx                         ; $82A65E
    dec.b $14                   ; $82A65F
    bne CODE_82A647             ; $82A661
    plb                         ; $82A663
    ldx.w #$0014                ; $82A664
    lda.w #$FFFF                ; $82A667
CODE_82A66A:
    sta.w $0444,x               ; $82A66A
    dex                         ; $82A66D
    dex                         ; $82A66E
    bne CODE_82A66A             ; $82A66F
    rts                         ; $82A671
    ldx.w #$929A                ; $82A672
    ldy.w #$0000                ; $82A675
    lda.w #$7F88                ; $82A678
    jsl.l $82FB70               ; $82A67B
    pea.w $837E                 ; $82A67F
    plb                         ; $82A682
    lda.w #$0000                ; $82A683
    sta.l $308000               ; $82A686
    lda.w $047B                 ; $82A68A
    sta.l $308000               ; $82A68D
    lda.w #$00E0                ; $82A691
    sta.l $308000               ; $82A694
    nop                         ; $82A698
    lda.l $308000               ; $82A699
    sta.b $16                   ; $82A69D
    sta.b $1A                   ; $82A69F
    lda.l $308000               ; $82A6A1
    stz.b $18                   ; $82A6A5
    stz.b $1C                   ; $82A6A7
    lda.w $01E4                 ; $82A6A9
    bmi CODE_82A6B7             ; $82A6AC
    incbin !baserom:$126AE..$126B7
CODE_82A6B7:
    lda.w $047B                 ; $82A6B7
    asl a                       ; $82A6BA
    tax                         ; $82A6BB
    lda.l $839583,x             ; $82A6BC
    bne CODE_82A6EC             ; $82A6C0
    incbin !baserom:$126C2..$126EC
CODE_82A6EC:
    ldy.w #$0000                ; $82A6EC
    lda.w #$0007                ; $82A6EF
    sta.b $14                   ; $82A6F2
    ldx.b $16                   ; $82A6F4
    lda.l $7F0000,x             ; $82A6F6
    sta.w $6A59,y               ; $82A6FA
    lda.l $7F0002,x             ; $82A6FD
    sta.w $6A5B,y               ; $82A701
    lda.l $7F0004,x             ; $82A704
    sta.w $6A5D,y               ; $82A708
    lda.l $7F0006,x             ; $82A70B
    sta.w $6A5F,y               ; $82A70F
    lda.l $7F0008,x             ; $82A712
    sta.w $6A61,y               ; $82A716
    lda.l $7F000A,x             ; $82A719
    sta.w $6A63,y               ; $82A71D
    lda.l $7F000C,x             ; $82A720
    sta.w $6A65,y               ; $82A724
    lda.l $7F000E,x             ; $82A727
    sta.w $6A67,y               ; $82A72B
    txa                         ; $82A72E
    clc                         ; $82A72F
    adc.w #$0010                ; $82A730
    sta.b $16                   ; $82A733
    ldx.b $1A                   ; $82A735
    lda.l $7F0000,x             ; $82A737
    sta.w $6B59,y               ; $82A73B
    lda.l $7F0002,x             ; $82A73E
    sta.w $6B5B,y               ; $82A742
    lda.l $7F0004,x             ; $82A745
    sta.w $6B5D,y               ; $82A749
    lda.l $7F0006,x             ; $82A74C
    sta.w $6B5F,y               ; $82A750
    lda.l $7F0008,x             ; $82A753
    sta.w $6B61,y               ; $82A757
    lda.l $7F000A,x             ; $82A75A
    sta.w $6B63,y               ; $82A75E
    lda.l $7F000C,x             ; $82A761
    sta.w $6B65,y               ; $82A765
    lda.l $7F000E,x             ; $82A768
    sta.w $6B67,y               ; $82A76C
    txa                         ; $82A76F
    clc                         ; $82A770
    adc.w #$0010                ; $82A771
    sta.b $1A                   ; $82A774
    ldx.b $18                   ; $82A776
    lda.l $85A1EC,x             ; $82A778
    sta.w $6A55,y               ; $82A77C
    lda.l $85A1EE,x             ; $82A77F
    sta.w $6A57,y               ; $82A783
    lda.l $85A1F0,x             ; $82A786
    sta.w $6A69,y               ; $82A78A
    lda.l $85A1F2,x             ; $82A78D
    sta.w $6A6B,y               ; $82A791
    lda.l $85A1F4,x             ; $82A794
    sta.w $6A6D,y               ; $82A798
    lda.l $85A1F6,x             ; $82A79B
    sta.w $6A6F,y               ; $82A79F
    lda.l $85A1F8,x             ; $82A7A2
    sta.w $6A71,y               ; $82A7A6
    txa                         ; $82A7A9
    clc                         ; $82A7AA
    adc.w #$0010                ; $82A7AB
    sta.b $18                   ; $82A7AE
    ldx.b $1C                   ; $82A7B0
    lda.l $85A1EC,x             ; $82A7B2
    sta.w $6B55,y               ; $82A7B6
    lda.l $85A1EE,x             ; $82A7B9
    sta.w $6B57,y               ; $82A7BD
    lda.l $85A1F0,x             ; $82A7C0
    sta.w $6B69,y               ; $82A7C4
    lda.l $85A1F2,x             ; $82A7C7
    sta.w $6B6B,y               ; $82A7CB
    lda.l $85A1F4,x             ; $82A7CE
    sta.w $6B6D,y               ; $82A7D2
    lda.l $85A1F6,x             ; $82A7D5
    sta.w $6B6F,y               ; $82A7D9
    lda.l $85A1F8,x             ; $82A7DC
    sta.w $6B71,y               ; $82A7E0
    txa                         ; $82A7E3
    clc                         ; $82A7E4
    adc.w #$0010                ; $82A7E5
    sta.b $1C                   ; $82A7E8
    tya                         ; $82A7EA
    clc                         ; $82A7EB
    adc.w #$0020                ; $82A7EC
    tay                         ; $82A7EF
    dec.b $14                   ; $82A7F0
    beq CODE_82A7F7             ; $82A7F2
    jmp.w $A6F4                 ; $82A7F4
CODE_82A7F7:
    plb                         ; $82A7F7
    rts                         ; $82A7F8
    lda.w $047B                 ; $82A7F9
    asl a                       ; $82A7FC
    tax                         ; $82A7FD
    lda.l $84FD33,x             ; $82A7FE
    tax                         ; $82A802
    ldy.w #$5391                ; $82A803
    lda.w #$7E9B                ; $82A806
    jsl.l $82FB70               ; $82A809
    lda.w $047B                 ; $82A80D
    asl a                       ; $82A810
    tax                         ; $82A811
    lda.l $84FDA1,x             ; $82A812
    pha                         ; $82A816
    ldy.w #$8CB3                ; $82A817
    lda.l $84FE0F,x             ; $82A81A
    ora.w #$7E00                ; $82A81E
    plx                         ; $82A821
    jsl.l $82FB70               ; $82A822
    phb                         ; $82A826
    lda.w $047B                 ; $82A827
    asl a                       ; $82A82A
    asl a                       ; $82A82B
    tax                         ; $82A82C
    lda.l $84FE7F,x             ; $82A82D
    pha                         ; $82A831
    lda.l $84FE7D,x             ; $82A832
    tax                         ; $82A836
    pla                         ; $82A837
    ldy.w #$5B91                ; $82A838
    mvn $7E,$8B                 ; $82A83B
    plb                         ; $82A83E
    lda.l $8103DC               ; $82A83F
    and.w #$0003                ; $82A843
    cmp.w #$0002                ; $82A846
    bne CODE_82A85A             ; $82A849
    incbin !baserom:$1284B..$1285A
CODE_82A85A:
    lda.w $01E4                 ; $82A85A
    bmi CODE_82A87F             ; $82A85D
    incbin !baserom:$1285F..$1287F
CODE_82A87F:
    ldx.w #$8020                ; $82A87F
    ldy.w #$0000                ; $82A882
    lda.w #$7F96                ; $82A885
    jsl.l $82FB70               ; $82A888
    lda.w $047B                 ; $82A88C
    asl a                       ; $82A88F
    tax                         ; $82A890
    lda.w $039C                 ; $82A891
    and.w #$00FF                ; $82A894
    beq CODE_82A8A1             ; $82A897
    incbin !baserom:$12899..$128A1
CODE_82A8A1:
    lda.l $85A35E,x             ; $82A8A1
    bpl CODE_82A8B4             ; $82A8A5
    lda.l $7E5395               ; $82A8A7
    sec                         ; $82A8AB
    sbc.w #$8007                ; $82A8AC
    tax                         ; $82A8AF
    lda.l $908000,x             ; $82A8B0
CODE_82A8B4:
    sta.w $003A                 ; $82A8B4
    lda.w $047B                 ; $82A8B7
    asl a                       ; $82A8BA
    tax                         ; $82A8BB
    lda.w $03CB                 ; $82A8BC
    and.w #$00FF                ; $82A8BF
    beq CODE_82A8CC             ; $82A8C2
    incbin !baserom:$128C4..$128CC
CODE_82A8CC:
    lda.l $85A35E,x             ; $82A8CC
    bpl CODE_82A8DF             ; $82A8D0
    lda.l $7E5395               ; $82A8D2
    sec                         ; $82A8D6
    sbc.w #$8007                ; $82A8D7
    tax                         ; $82A8DA
    lda.l $908000,x             ; $82A8DB
CODE_82A8DF:
    sta.w $0038                 ; $82A8DF
    pea.w $837E                 ; $82A8E2
    plb                         ; $82A8E5
    lda.w #$00D2                ; $82A8E6
    sta.b $14                   ; $82A8E9
    lda.l $8103DC               ; $82A8EB
    and.w #$0003                ; $82A8EF
    cmp.w #$0002                ; $82A8F2
    bne CODE_82A8FC             ; $82A8F5
    incbin !baserom:$128F7..$128FC
CODE_82A8FC:
    ldy.w #$0200                ; $82A8FC
    lda.w #$FFFF                ; $82A8FF
CODE_82A902:
    sta.w $658F,y               ; $82A902
    dey                         ; $82A905
    dey                         ; $82A906
    bne CODE_82A902             ; $82A907
    lda.w $019A                 ; $82A909
    asl a                       ; $82A90C
    asl a                       ; $82A90D
    tay                         ; $82A90E
CODE_82A90F:
    lda.w $8CB0,y               ; $82A90F
    and.w #$00FF                ; $82A912
    beq CODE_82A94C             ; $82A915
    asl a                       ; $82A917
    tax                         ; $82A918
    lda.l $86AE36,x             ; $82A919
    beq CODE_82A94C             ; $82A91D
    tax                         ; $82A91F
    phy                         ; $82A920
    lda.l $860000,x             ; $82A921
    tay                         ; $82A925
    lda.w $6591,y               ; $82A926
    cmp.w #$FFFF                ; $82A929
    bne CODE_82A94B             ; $82A92C
    lda.b $14                   ; $82A92E
    sta.w $6591,y               ; $82A930
    tay                         ; $82A933
    lda.l $860002,x             ; $82A934
    ora.w #$7F00                ; $82A938
    pha                         ; $82A93B
    lda.l $860004,x             ; $82A93C
    tax                         ; $82A940
    pla                         ; $82A941
    jsl.l $82FB70               ; $82A942
    clc                         ; $82A946
    adc.b $14                   ; $82A947
    sta.b $14                   ; $82A949
CODE_82A94B:
    ply                         ; $82A94B
CODE_82A94C:
    dey                         ; $82A94C
    dey                         ; $82A94D
    dey                         ; $82A94E
    dey                         ; $82A94F
    bne CODE_82A90F             ; $82A950
    plb                         ; $82A952
    rts                         ; $82A953
    ldx.w #$8000                ; $82A954
    ldy.w #$0000                ; $82A957
    lda.w #$7F9A                ; $82A95A
    jsl.l $82FB70               ; $82A95D
    lda.w $047B                 ; $82A961
    asl a                       ; $82A964
    tax                         ; $82A965
    lda.l $859FAA,x             ; $82A966
    pha                         ; $82A96A
    lda.l $85A008,x             ; $82A96B
    pha                         ; $82A96F
    ldy.w #$4F31                ; $82A970
    pla                         ; $82A973
    plx                         ; $82A974
    pha                         ; $82A975
    dec a                       ; $82A976
    phb                         ; $82A977
    mvn $7E,$7F                 ; $82A978
    plb                         ; $82A97B
    pla                         ; $82A97C
    lsr a                       ; $82A97D
    lsr a                       ; $82A97E
    tay                         ; $82A97F
    pha                         ; $82A980
    ldx.w #$0000                ; $82A981
    lda.w #$0000                ; $82A984
CODE_82A987:
    clc                         ; $82A987
    adc.l $7E4F31,x             ; $82A988
    sta.l $7E4F31,x             ; $82A98C
    inx                         ; $82A990
    inx                         ; $82A991
    inx                         ; $82A992
    inx                         ; $82A993
    dey                         ; $82A994
    bne CODE_82A987             ; $82A995
    ply                         ; $82A997
    ldx.w #$0000                ; $82A998
    lda.w #$0000                ; $82A99B
CODE_82A99E:
    clc                         ; $82A99E
    adc.l $7E4F33,x             ; $82A99F
    sta.l $7E4F33,x             ; $82A9A3
    inx                         ; $82A9A7
    inx                         ; $82A9A8
    inx                         ; $82A9A9
    inx                         ; $82A9AA
    dey                         ; $82A9AB
    bne CODE_82A99E             ; $82A9AC
    rts                         ; $82A9AE
    jsr.w $F2B3                 ; $82A9AF
    rtl                         ; $82A9B2
    jsr.w $FA4F                 ; $82A9B3
    rtl                         ; $82A9B6
    pea.w $837E                 ; $82A9B7
    plb                         ; $82A9BA
    lda.w $0281                 ; $82A9BB
    bpl CODE_82A9CA             ; $82A9BE
    lda.w #$FFD0                ; $82A9C0
    sta.b $18                   ; $82A9C3
    stz.b $16                   ; $82A9C5
    jmp.w $AA1F                 ; $82A9C7
CODE_82A9CA:
    ldy.w $0283                 ; $82A9CA
    ldx.w $0281                 ; $82A9CD
    jsr.w $AB3F                 ; $82A9D0
    lda.w #$0000                ; $82A9D3
    sta.l $308000               ; $82A9D6
    lda.b $16                   ; $82A9DA
    asl a                       ; $82A9DC
    sta.l $308000               ; $82A9DD
    lda.w $0293                 ; $82A9E1
    sta.l $308000               ; $82A9E4
    nop                         ; $82A9E8
    nop                         ; $82A9E9
    lda.l $308000               ; $82A9EA
    lda.l $308000               ; $82A9EE
    eor.w #$FFFF                ; $82A9F2
    inc a                       ; $82A9F5
    sta.b $16                   ; $82A9F6
    lda.w #$0000                ; $82A9F8
    sta.l $308000               ; $82A9FB
    lda.b $18                   ; $82A9FF
    asl a                       ; $82AA01
    sta.l $308000               ; $82AA02
    lda.w $0293                 ; $82AA06
    sta.l $308000               ; $82AA09
    lda.w #$0033                ; $82AA0D
    sec                         ; $82AA10
    sta.l $308000               ; $82AA11
    sbc.l $308000               ; $82AA15
    eor.w #$FFFF                ; $82AA19
    inc a                       ; $82AA1C
    sta.b $18                   ; $82AA1D
    lda.b $16                   ; $82AA1F
    cmp.w $028B                 ; $82AA21
    bpl CODE_82AA3A             ; $82AA24
    lda.w $1E9E                 ; $82AA26
    asl a                       ; $82AA29
    asl a                       ; $82AA2A
    eor.w #$FFFF                ; $82AA2B
    sec                         ; $82AA2E
    adc.w $028B                 ; $82AA2F
    cmp.b $16                   ; $82AA32
    bpl CODE_82AA38             ; $82AA34
    lda.b $16                   ; $82AA36
CODE_82AA38:
    bra CODE_82AA49             ; $82AA38
CODE_82AA3A:
    lda.w $1E9E                 ; $82AA3A
    asl a                       ; $82AA3D
    asl a                       ; $82AA3E
    clc                         ; $82AA3F
    adc.w $028B                 ; $82AA40
    cmp.b $16                   ; $82AA43
    bmi CODE_82AA49             ; $82AA45
    lda.b $16                   ; $82AA47
CODE_82AA49:
    sta.w $028B                 ; $82AA49
    lda.b $18                   ; $82AA4C
    cmp.w $028D                 ; $82AA4E
    bpl CODE_82AA67             ; $82AA51
    lda.w $1E9E                 ; $82AA53
    asl a                       ; $82AA56
    asl a                       ; $82AA57
    eor.w #$FFFF                ; $82AA58
    sec                         ; $82AA5B
    adc.w $028D                 ; $82AA5C
    cmp.b $18                   ; $82AA5F
    bpl CODE_82AA65             ; $82AA61
    lda.b $18                   ; $82AA63
CODE_82AA65:
    bra CODE_82AA76             ; $82AA65
CODE_82AA67:
    lda.w $1E9E                 ; $82AA67
    asl a                       ; $82AA6A
    asl a                       ; $82AA6B
    clc                         ; $82AA6C
    adc.w $028D                 ; $82AA6D
    cmp.b $18                   ; $82AA70
    bmi CODE_82AA76             ; $82AA72
    lda.b $18                   ; $82AA74
CODE_82AA76:
    sta.w $028D                 ; $82AA76
    plb                         ; $82AA79
    rts                         ; $82AA7A
    incbin !baserom:$12A7B..$12B3F
    lda.w #$0000                ; $82AB3F
    sta.l $308000               ; $82AB42
    lda.w $FAB4,x               ; $82AB46
    and.w #$FF00                ; $82AB49
    lsr a                       ; $82AB4C
    sta.b $14                   ; $82AB4D
    sta.l $308000               ; $82AB4F
    lda.w $34EE,y               ; $82AB53
    sec                         ; $82AB56
    sbc.w $34EC,y               ; $82AB57
    asl a                       ; $82AB5A
    sta.l $308000               ; $82AB5B
    lda.w $34EC,y               ; $82AB5F
    clc                         ; $82AB62
    sta.l $308000               ; $82AB63
    adc.l $308000               ; $82AB67
    sec                         ; $82AB6B
    sbc.w $FABB,x               ; $82AB6C
    sta.b $16                   ; $82AB6F
    lda.w #$0000                ; $82AB71
    sta.l $308000               ; $82AB74
    lda.b $14                   ; $82AB78
    sta.l $308000               ; $82AB7A
    lda.w $36EE,y               ; $82AB7E
    sec                         ; $82AB81
    sbc.w $36EC,y               ; $82AB82
    asl a                       ; $82AB85
    sta.l $308000               ; $82AB86
    lda.w $36EC,y               ; $82AB8A
    clc                         ; $82AB8D
    sta.l $308000               ; $82AB8E
    adc.l $308000               ; $82AB92
    sec                         ; $82AB96
    sbc.w #$000C                ; $82AB97
    sta.b $18                   ; $82AB9A
    rts                         ; $82AB9C
    lda.w $1ECE                 ; $82AB9D
    bmi CODE_82ABA5             ; $82ABA0
    bne CODE_82ABC6             ; $82ABA2
    rts                         ; $82ABA4
CODE_82ABA5:
    bit.w $047C                 ; $82ABA5
    bmi CODE_82ABC5             ; $82ABA8
    lda.w #$003D                ; $82ABAA
    sta.w $0233                 ; $82ABAD
    inc a                       ; $82ABB0
    sta.w $02E6                 ; $82ABB1
    lda.w $03DC                 ; $82ABB4
    and.w #$00F0                ; $82ABB7
    cmp.w #$0030                ; $82ABBA
    beq CODE_82ABC5             ; $82ABBD
    lda.w #$0045                ; $82ABBF
    sta.w $02E6                 ; $82ABC2
CODE_82ABC5:
    rts                         ; $82ABC5
CODE_82ABC6:
    tax                         ; $82ABC6
    sec                         ; $82ABC7
    sbc.w $1E9E                 ; $82ABC8
    bpl CODE_82ABD0             ; $82ABCB
    lda.w #$0000                ; $82ABCD
CODE_82ABD0:
    sta.w $1ECE                 ; $82ABD0
    tay                         ; $82ABD3
    lda.w #$001E                ; $82ABD4
    sta.b $14                   ; $82ABD7
    lda.w #$0000                ; $82ABD9
    cpx.w #$00D2                ; $82ABDC
    bmi CODE_82ABE9             ; $82ABDF
    cpy.w #$00D2                ; $82ABE1
    bpl CODE_82ABE9             ; $82ABE4
    lda.w #$0002                ; $82ABE6
CODE_82ABE9:
    cpx.w #$008C                ; $82ABE9
    bmi CODE_82ABF6             ; $82ABEC
    cpy.w #$008C                ; $82ABEE
    bpl CODE_82ABF6             ; $82ABF1
    lda.w #$0003                ; $82ABF3
CODE_82ABF6:
    cpx.w #$0046                ; $82ABF6
    bmi CODE_82AC03             ; $82ABF9
    cpy.w #$0046                ; $82ABFB
    bpl CODE_82AC03             ; $82ABFE
    lda.w #$0004                ; $82AC00
CODE_82AC03:
    cpx.w #$0002                ; $82AC03
    bmi CODE_82AC30             ; $82AC06
    cpy.w #$0002                ; $82AC08
    bpl CODE_82AC30             ; $82AC0B
    lda.w #$0022                ; $82AC0D
    sta.b $14                   ; $82AC10
    sep.b #$20                  ; $82AC12
    lda.b #$01                  ; $82AC14
    rep.b #$20                  ; $82AC16
    lda.w #$0001                ; $82AC18
    sta.w $1ECE                 ; $82AC1B
    lda.w #$0008                ; $82AC1E
    tsb.w $0244                 ; $82AC21
    tsb.w $02F7                 ; $82AC24
    tsb.w $02A6                 ; $82AC27
    tsb.w $0359                 ; $82AC2A
    lda.w #$0005                ; $82AC2D
CODE_82AC30:
    sta.w $0233                 ; $82AC30
    sta.w $02E6                 ; $82AC33
    sta.w $0295                 ; $82AC36
    sta.w $0348                 ; $82AC39
    asl a                       ; $82AC3C
    beq CODE_82AC51             ; $82AC3D
    lda.b $14                   ; $82AC3F
    xba                         ; $82AC41
    and.w #$00FF                ; $82AC42
    ora.w #$2000                ; $82AC45
    tax                         ; $82AC48
    lda.b $14                   ; $82AC49
    ora.w #$FF00                ; $82AC4B
    jsr.w $AD12                 ; $82AC4E
CODE_82AC51:
    rts                         ; $82AC51
    lda.w #$0000                ; $82AC52
    sta.l $308000               ; $82AC55
    lda.w $0213                 ; $82AC59
    sta.l $308000               ; $82AC5C
    lda.w #$0300                ; $82AC60
    sta.l $308000               ; $82AC63
    lda.w #$FF00                ; $82AC67
    and.l $308000               ; $82AC6A
    xba                         ; $82AC6E
    sta.b $14                   ; $82AC6F
    lda.l $308000               ; $82AC71
    xba                         ; $82AC75
    and.w #$FF00                ; $82AC76
    ora.b $14                   ; $82AC79
    sec                         ; $82AC7B
    sbc.w #$0B00                ; $82AC7C
    cmp.w #$3000                ; $82AC7F
    bmi CODE_82AC87             ; $82AC82
    incbin !baserom:$12C84..$12C87
CODE_82AC87:
    sta.l $7E4E2D               ; $82AC87
    lda.w $0244                 ; $82AC8B
    lsr a                       ; $82AC8E
    bcc CODE_82AD0D             ; $82AC8F
    lda.l $7EFAB4               ; $82AC91
    cmp.w $0224                 ; $82AC95
    lda.l $7EFAB6               ; $82AC98
    sbc.w $0226                 ; $82AC9C
    bpl CODE_82ACA4             ; $82AC9F
    incbin !baserom:$12CA1..$12CA4
CODE_82ACA4:
    asl a                       ; $82ACA4
    sta.b $14                   ; $82ACA5
    lda.w #$00B0                ; $82ACA7
    sec                         ; $82ACAA
    sbc.b $14                   ; $82ACAB
    bpl CODE_82ACB2             ; $82ACAD
    incbin !baserom:$12CAF..$12CB2
CODE_82ACB2:
    and.w #$00F8                ; $82ACB2
    sta.l $7E4E35               ; $82ACB5
    lda.w $0244                 ; $82ACB9
    bit.w #$0004                ; $82ACBC
    beq CODE_82AD0D             ; $82ACBF
    incbin !baserom:$12CC1..$12D0D
CODE_82AD0D:
    rts                         ; $82AD0D
    incbin !baserom:$12D0E..$12D12
    bit.w $047C                 ; $82AD12
    bmi CODE_82AD20             ; $82AD15
    stz.w $0466                 ; $82AD17
    stx.w $0465                 ; $82AD1A
    sta.w $0463                 ; $82AD1D
CODE_82AD20:
    rts                         ; $82AD20
    incbin !baserom:$12D21..$12D2C
    pea.w $837E                 ; $82AD2C
    plb                         ; $82AD2F
    ldy.w #$0014                ; $82AD30
    ldx.w #$0000                ; $82AD33
    stz.w $0142                 ; $82AD36
    stz.w $0144                 ; $82AD39
CODE_82AD3C:
    lda.w $FAB5,x               ; $82AD3C
    cmp.w $FAB5                 ; $82AD3F
    lda.w $FAB7,x               ; $82AD42
    sbc.w $FAB7                 ; $82AD45
    lda.w $FAD3,x               ; $82AD48
    sbc.w $FAD3                 ; $82AD4B
    bmi CODE_82AD53             ; $82AD4E
    inc.w $0142                 ; $82AD50
CODE_82AD53:
    lda.w $FAB5,x               ; $82AD53
    cmp.w $FAEA                 ; $82AD56
    lda.w $FAB7,x               ; $82AD59
    sbc.w $FAEC                 ; $82AD5C
    lda.w $FAD3,x               ; $82AD5F
    sbc.w $FB08                 ; $82AD62
    bmi CODE_82AD6A             ; $82AD65
    inc.w $0144                 ; $82AD67
CODE_82AD6A:
    txa                         ; $82AD6A
    clc                         ; $82AD6B
    adc.w #$0035                ; $82AD6C
    tax                         ; $82AD6F
    dey                         ; $82AD70
    bne CODE_82AD3C             ; $82AD71
    lda.w $0142                 ; $82AD73
    cmp.w $1ED2                 ; $82AD76
    beq CODE_82AD85             ; $82AD79
    sta.w $1ED2                 ; $82AD7B
    clc                         ; $82AD7E
    adc.w #$0025                ; $82AD7F
    jsr.w $9BC9                 ; $82AD82
CODE_82AD85:
    lda.w $0144                 ; $82AD85
    cmp.w $1ED4                 ; $82AD88
    beq CODE_82AD97             ; $82AD8B
    sta.w $1ED4                 ; $82AD8D
    clc                         ; $82AD90
    adc.w #$0025                ; $82AD91
    jsr.w $9BE5                 ; $82AD94
CODE_82AD97:
    plb                         ; $82AD97
    rts                         ; $82AD98
    sta.b $16                   ; $82AD99
    ldx.w #$0000                ; $82AD9B
    lda.w #$0005                ; $82AD9E
    sta.b $14                   ; $82ADA1
CODE_82ADA3:
    ldy.w #$0005                ; $82ADA3
CODE_82ADA6:
    lda.b $16                   ; $82ADA6
    sta.l $7F0130,x             ; $82ADA8
    inc.b $16                   ; $82ADAC
    txa                         ; $82ADAE
    clc                         ; $82ADAF
    adc.w #$0040                ; $82ADB0
    tax                         ; $82ADB3
    dey                         ; $82ADB4
    bne CODE_82ADA6             ; $82ADB5
    txa                         ; $82ADB7
    sec                         ; $82ADB8
    sbc.w #$013E                ; $82ADB9
    tax                         ; $82ADBC
    dec.b $14                   ; $82ADBD
    bne CODE_82ADA3             ; $82ADBF
    rts                         ; $82ADC1
    ldx.w #$97E4                ; $82ADC2
    ldy.w #$0000                ; $82ADC5
    lda.w #$7F99                ; $82ADC8
    jsl.l $82FB70               ; $82ADCB
    lda.w #$0000                ; $82ADCF
    sta.l $308000               ; $82ADD2
    lda.w $047B                 ; $82ADD6
    sta.l $308000               ; $82ADD9
    lda.w #$0190                ; $82ADDD
    sta.l $308000               ; $82ADE0
    phb                         ; $82ADE4
    lda.w #$0000                ; $82ADE5
    clc                         ; $82ADE8
    adc.l $308000               ; $82ADE9
    sta.l $308000               ; $82ADED
    tax                         ; $82ADF1
    ldy.w #$217C                ; $82ADF2
    lda.w #$018F                ; $82ADF5
    mvn $7E,$7F                 ; $82ADF8
    ldx.w #$217C                ; $82ADFB
    ldy.w #$230C                ; $82ADFE
    lda.w #$018F                ; $82AE01
    mvn $7E,$7E                 ; $82AE04
    ldx.w #$217C                ; $82AE07
    ldy.w #$249C                ; $82AE0A
    lda.w #$018F                ; $82AE0D
    mvn $7E,$7E                 ; $82AE10
    plb                         ; $82AE13
    rts                         ; $82AE14
    pea.w $8330                 ; $82AE15
    plb                         ; $82AE18
    lda.w #$0006                ; $82AE19
    sta.w $8000                 ; $82AE1C
    ldy.b $8E                   ; $82AE1F
    lda.w $8000                 ; $82AE21
    sta.w $1776,y               ; $82AE24
    lda.w $8000                 ; $82AE27
    sta.w $1778,y               ; $82AE2A
    lda.w $8000                 ; $82AE2D
    sta.w $177A,y               ; $82AE30
    lda.w $8000                 ; $82AE33
    sta.w $177C,y               ; $82AE36
    lda.w $8000                 ; $82AE39
    sta.w $177E,y               ; $82AE3C
    lda.w $8000                 ; $82AE3F
    sta.w $1780,y               ; $82AE42
    lda.w $8000                 ; $82AE45
    sta.w $1782,y               ; $82AE48
    lda.w $8000                 ; $82AE4B
    sta.w $1784,y               ; $82AE4E
    lda.w $8000                 ; $82AE51
    sta.w $1786,y               ; $82AE54
    lda.w $8000                 ; $82AE57
    sta.w $1788,y               ; $82AE5A
    lda.w $8000                 ; $82AE5D
    sta.w $178A,y               ; $82AE60
    lda.w $8000                 ; $82AE63
    sta.w $178C,y               ; $82AE66
    lda.w $8000                 ; $82AE69
    sta.w $178E,y               ; $82AE6C
    lda.w $8000                 ; $82AE6F
    sta.w $1790,y               ; $82AE72
    lda.w $8000                 ; $82AE75
    sta.w $1792,y               ; $82AE78
    lda.w $8000                 ; $82AE7B
    sta.w $1794,y               ; $82AE7E
    plb                         ; $82AE81
    lda.b $86                   ; $82AE82
    tay                         ; $82AE84
    sec                         ; $82AE85
    sbc.b $8E                   ; $82AE86
    eor.w #$FFFF                ; $82AE88
    sec                         ; $82AE8B
    adc.w #$0200                ; $82AE8C
    beq CODE_82AEA3             ; $82AE8F
    lsr a                       ; $82AE91
    lsr a                       ; $82AE92
    tax                         ; $82AE93
    ldy.b $86                   ; $82AE94
    lda.w #$E0E0                ; $82AE96
CODE_82AE99:
    sta.w $1576,y               ; $82AE99
    iny                         ; $82AE9C
    iny                         ; $82AE9D
    iny                         ; $82AE9E
    iny                         ; $82AE9F
    dex                         ; $82AEA0
    bne CODE_82AE99             ; $82AEA1
CODE_82AEA3:
    rts                         ; $82AEA3
    incbin !baserom:$12EA4..$12F33
    lda.w #$0140                ; $82AF33
    sta.l $7E45EC               ; $82AF36
    ldx.b $A0                   ; $82AF3A
    lda.w $0226                 ; $82AF3C
    cmp.l $7E5B91,x             ; $82AF3F
    bcs CODE_82AF48             ; $82AF43
    incbin !baserom:$12F45..$12F48
CODE_82AF48:
    lda.l $7E5B94,x             ; $82AF48
    db $F0,$55 ; beq            ; $82AF4C
    bit.w #$FF00                ; $82AF4E
    bne CODE_82AFB1             ; $82AF51
    asl a                       ; $82AF53
    tax                         ; $82AF54
    lda.l $86AE36,x             ; $82AF55
    db $F0,$54 ; beq            ; $82AF59
    tax                         ; $82AF5B
    lda.l $86001E,x             ; $82AF5C
    sta.b $14                   ; $82AF60
    lda.l $860020,x             ; $82AF62
    sta.b $16                   ; $82AF66
    lda.l $860000,x             ; $82AF68
    tax                         ; $82AF6C
    lda.l $7E6591,x             ; $82AF6D
    clc                         ; $82AF71
    adc.b $14                   ; $82AF72
    sta.b $14                   ; $82AF74
    ldx.b $A0                   ; $82AF76
    lda.l $7E5B93,x             ; $82AF78
    and.w #$00FF                ; $82AF7C
    xba                         ; $82AF7F
    lsr a                       ; $82AF80
    lsr a                       ; $82AF81
    lsr a                       ; $82AF82
    sta.b $18                   ; $82AF83
    lda.l $7E5B93,x             ; $82AF85
    and.w #$00FF                ; $82AF89
    asl a                       ; $82AF8C
    sta.b $1A                   ; $82AF8D
    lda.l $7E5B93,x             ; $82AF8F
    xba                         ; $82AF93
    and.w #$00FF                ; $82AF94
    asl a                       ; $82AF97
    txy                         ; $82AF98
    tax                         ; $82AF99
    lda.b $1A                   ; $82AF9A
    sta.l $7E45EC,x             ; $82AF9C
    tyx                         ; $82AFA0
    bra CODE_82B001             ; $82AFA1
    incbin !baserom:$12FA3..$12FB1
CODE_82AFB1:
    asl a                       ; $82AFB1
    tax                         ; $82AFB2
    lda.l $86AC36,x             ; $82AFB3
    beq CODE_82B012             ; $82AFB7
    tax                         ; $82AFB9
    lda.l $860022,x             ; $82AFBA
    sta.b $14                   ; $82AFBE
    lda.l $860024,x             ; $82AFC0
    sta.l $810116               ; $82AFC4
    lda.l $860000,x             ; $82AFC8
    tax                         ; $82AFCC
    lda.l $7E6591,x             ; $82AFCD
    clc                         ; $82AFD1
    adc.b $14                   ; $82AFD2
    sta.b $14                   ; $82AFD4
    ldx.b $A0                   ; $82AFD6
    lda.l $7E5B93,x             ; $82AFD8
    and.w #$00FF                ; $82AFDC
    xba                         ; $82AFDF
    lsr a                       ; $82AFE0
    lsr a                       ; $82AFE1
    lsr a                       ; $82AFE2
    sta.b $18                   ; $82AFE3
    lda.l $7E5B93,x             ; $82AFE5
    and.w #$00FF                ; $82AFE9
    asl a                       ; $82AFEC
    sta.b $1A                   ; $82AFED
    lda.l $7E5B93,x             ; $82AFEF
    xba                         ; $82AFF3
    and.w #$00FF                ; $82AFF4
    asl a                       ; $82AFF7
    txy                         ; $82AFF8
    tax                         ; $82AFF9
    lda.b $1A                   ; $82AFFA
    sta.l $7E495C,x             ; $82AFFC
    tyx                         ; $82B000
CODE_82B001:
    phx                         ; $82B001
    ldx.b $14                   ; $82B002
    ldy.b $18                   ; $82B004
    lda.w #$007F                ; $82B006
    sta.b $14                   ; $82B009
    lda.b $16                   ; $82B00B
    jsl.l $8083AD               ; $82B00D
    plx                         ; $82B011
CODE_82B012:
    inx                         ; $82B012
    inx                         ; $82B013
    inx                         ; $82B014
    inx                         ; $82B015
    inx                         ; $82B016
    stx.b $A0                   ; $82B017
    lda.w $0226                 ; $82B019
    cmp.l $7E5B91,x             ; $82B01C
    bcc CODE_82B025             ; $82B020
    jmp.w $AF48                 ; $82B022
CODE_82B025:
    rts                         ; $82B025
    incbin !baserom:$13026..$1311F
    pea.w $837E                 ; $82B11F
    plb                         ; $82B122
    lda.b $14                   ; $82B123
    cmp.b $16                   ; $82B125
    bne CODE_82B12C             ; $82B127
    jmp.w $B15D                 ; $82B129
CODE_82B12C:
    asl a                       ; $82B12C
    pha                         ; $82B12D
    lda.b $16                   ; $82B12E
    asl a                       ; $82B130
    sta.b $3C                   ; $82B131
    tax                         ; $82B133
    lda.l $83A0A4,x             ; $82B134
    plx                         ; $82B138
    cmp.l $83A0A4,x             ; $82B139
    bcs CODE_82B142             ; $82B13D
    jmp.w $B159                 ; $82B13F
CODE_82B142:
    jsr.w $B18B                 ; $82B142
    ldx.b $3C                   ; $82B145
    lda.l $83A142,x             ; $82B147
    sta.b $1A                   ; $82B14B
    lda.l $83A1E0,x             ; $82B14D
    sta.b $1C                   ; $82B151
    lda.b $16                   ; $82B153
    sta.b $14                   ; $82B155
    bra CODE_82B189             ; $82B157
    lda.b $14                   ; $82B159
    sta.b $16                   ; $82B15B
    lda.b $1A                   ; $82B15D
    beq CODE_82B187             ; $82B15F
    sec                         ; $82B161
    sbc.w $1E9E                 ; $82B162
    beq CODE_82B169             ; $82B165
    bcs CODE_82B187             ; $82B167
CODE_82B169:
    ldx.b $18                   ; $82B169
    ldy.w #$000D                ; $82B16B
    lda.l $83A5A4               ; $82B16E
CODE_82B172:
    sta.w $0000,x               ; $82B172
    sta.w $001A,x               ; $82B175
    inx                         ; $82B178
    inx                         ; $82B179
    dey                         ; $82B17A
    bne CODE_82B172             ; $82B17B
    lda.w #$0000                ; $82B17D
    sta.b $16                   ; $82B180
    sta.b $14                   ; $82B182
    lda.w #$0000                ; $82B184
CODE_82B187:
    sta.b $1A                   ; $82B187
CODE_82B189:
    plb                         ; $82B189
    rts                         ; $82B18A
    ldx.b $18                   ; $82B18B
    ldy.w #$000D                ; $82B18D
    lda.l $83A5A4               ; $82B190
CODE_82B194:
    sta.w $0000,x               ; $82B194
    sta.w $001A,x               ; $82B197
    inx                         ; $82B19A
    inx                         ; $82B19B
    dey                         ; $82B19C
    bne CODE_82B194             ; $82B19D
    ldx.b $3C                   ; $82B19F
    lda.l $83A006,x             ; $82B1A1
    tax                         ; $82B1A5
    ldy.w #$000D                ; $82B1A6
CODE_82B1A9:
    lda.l $830000,x             ; $82B1A9
    and.w #$00FF                ; $82B1AD
    beq CODE_82B1BB             ; $82B1B0
    bit.w #$0080                ; $82B1B2
    bne CODE_82B1B8             ; $82B1B5
    dey                         ; $82B1B7
CODE_82B1B8:
    inx                         ; $82B1B8
    bra CODE_82B1A9             ; $82B1B9
CODE_82B1BB:
    tya                         ; $82B1BB
    and.w #$FFFE                ; $82B1BC
    clc                         ; $82B1BF
    adc.b $18                   ; $82B1C0
    tay                         ; $82B1C2
    ldx.b $3C                   ; $82B1C3
    lda.l $83A006,x             ; $82B1C5
    tax                         ; $82B1C9
CODE_82B1CA:
    lda.l $830000,x             ; $82B1CA
    and.w #$00FF                ; $82B1CE
    beq CODE_82B233             ; $82B1D1
    phx                         ; $82B1D3
    bit.w #$0080                ; $82B1D4
    beq CODE_82B1E3             ; $82B1D7
    incbin !baserom:$131D9..$131E3
CODE_82B1E3:
    cmp.w #$0020                ; $82B1E3
    bne CODE_82B1ED             ; $82B1E6
    incbin !baserom:$131E8..$131ED
CODE_82B1ED:
    cmp.w #$0021                ; $82B1ED
    bne CODE_82B1F7             ; $82B1F0
    lda.w #$0024                ; $82B1F2
    bra CODE_82B219             ; $82B1F5
CODE_82B1F7:
    cmp.w #$002E                ; $82B1F7
    bne CODE_82B201             ; $82B1FA
    incbin !baserom:$131FC..$13201
CODE_82B201:
    cmp.w #$002B                ; $82B201
    bne CODE_82B20B             ; $82B204
    incbin !baserom:$13206..$1320B
CODE_82B20B:
    cmp.w #$003A                ; $82B20B
    bcs CODE_82B216             ; $82B20E
    sec                         ; $82B210
    sbc.w #$0016                ; $82B211
    bra CODE_82B219             ; $82B214
CODE_82B216:
    sbc.w #$0041                ; $82B216
CODE_82B219:
    asl a                       ; $82B219
    tax                         ; $82B21A
    lda.l $83A504,x             ; $82B21B
    ora.b $3A                   ; $82B21F
    sta.w $0000,y               ; $82B221
    lda.l $83A554,x             ; $82B224
    ora.b $3A                   ; $82B228
    sta.w $001A,y               ; $82B22A
    iny                         ; $82B22D
    iny                         ; $82B22E
    plx                         ; $82B22F
    inx                         ; $82B230
    bra CODE_82B1CA             ; $82B231
CODE_82B233:
    rts                         ; $82B233
    incbin !baserom:$13234..$134ED
    lda.w $0244                 ; $82B4ED
    bit.w #$0004                ; $82B4F0
    beq CODE_82B4F8             ; $82B4F3
    incbin !baserom:$134F5..$134F8
CODE_82B4F8:
    lda.w #$0000                ; $82B4F8
    sta.l $308000               ; $82B4FB
    lda.w $0213                 ; $82B4FF
    sta.l $308000               ; $82B502
    lda.w #$0160                ; $82B506
    sta.l $308000               ; $82B509
    nop                         ; $82B50D
    lda.l $308000               ; $82B50E
    lda.l $308000               ; $82B512
    bpl CODE_82B51C             ; $82B516
    incbin !baserom:$13518..$1351C
CODE_82B51C:
    cmp.w #$000C                ; $82B51C
    bcc CODE_82B524             ; $82B51F
    incbin !baserom:$13521..$13524
CODE_82B524:
    sta.w $003E                 ; $82B524
    lda.w #$0157                ; $82B527
    sta.w $0042                 ; $82B52A
    sta.w $0044                 ; $82B52D
    sta.w $0046                 ; $82B530
    sta.w $0048                 ; $82B533
    sta.w $1E96                 ; $82B536
    sta.w $1E98                 ; $82B539
    lda.w #$0000                ; $82B53C
    sta.l $308000               ; $82B53F
    lda.w $0210                 ; $82B543
    bpl CODE_82B54C             ; $82B546
    incbin !baserom:$13548..$1354C
CODE_82B54C:
    asl a                       ; $82B54C
    asl a                       ; $82B54D
    asl a                       ; $82B54E
    sta.l $308000               ; $82B54F
    ldx.w #$3000                ; $82B553
    lda.w $03A7                 ; $82B556
    and.w #$00FF                ; $82B559
    beq CODE_82B561             ; $82B55C
    incbin !baserom:$1355E..$13561
CODE_82B561:
    txa                         ; $82B561
    sta.l $308000               ; $82B562
    nop                         ; $82B566
    lda.l $308000               ; $82B567
    lda.l $308000               ; $82B56B
    bit.w $1E94                 ; $82B56F
    bmi CODE_82B578             ; $82B572
    incbin !baserom:$13574..$13578
CODE_82B578:
    ldx.w #$0000                ; $82B578
    cmp.w #$03E8                ; $82B57B
    bcc CODE_82B59E             ; $82B57E
    incbin !baserom:$13580..$1359E
CODE_82B59E:
    cmp.w #$000A                ; $82B59E
    bcc CODE_82B5F4             ; $82B5A1
    incbin !baserom:$135A3..$135F4
CODE_82B5F4:
    asl a                       ; $82B5F4
    asl a                       ; $82B5F5
    tax                         ; $82B5F6
    lda.w $A4AC,x               ; $82B5F7
    sta.w $004A                 ; $82B5FA
    lda.w $A4AE,x               ; $82B5FD
    sta.w $004C                 ; $82B600
    lda.w #$0157                ; $82B603
    sta.w $0064                 ; $82B606
    lda.b $42                   ; $82B609
    cmp.w #$000A                ; $82B60B
    bcc CODE_82B62A             ; $82B60E
    cmp.w #$0014                ; $82B610
    bcc CODE_82B620             ; $82B613
    ldx.w $A488                 ; $82B615
    stx.w $0064                 ; $82B618
    sbc.w #$0014                ; $82B61B
    bra CODE_82B62A             ; $82B61E
CODE_82B620:
    ldx.w $A486                 ; $82B620
    stx.w $0064                 ; $82B623
    sec                         ; $82B626
    sbc.w #$000A                ; $82B627
CODE_82B62A:
    asl a                       ; $82B62A
    tax                         ; $82B62B
    lda.w $A484,x               ; $82B62C
    sta.w $0066                 ; $82B62F
    lda.w $0258                 ; $82B632
    cmp.w #$0009                ; $82B635
    bcs CODE_82B642             ; $82B638
    incbin !baserom:$1363A..$13642
CODE_82B642:
    ldx.w #$295B                ; $82B642
    stx.w $0074                 ; $82B645
    cmp.w #$0008                ; $82B648
    bcs CODE_82B67F             ; $82B64B
    incbin !baserom:$1364D..$1367F
CODE_82B67F:
    ldx.w #$0006                ; $82B67F
CODE_82B682:
    sec                         ; $82B682
    sbc.w #$0008                ; $82B683
    cmp.w #$0008                ; $82B686
    bcs CODE_82B6B9             ; $82B689
    incbin !baserom:$1368B..$136B9
CODE_82B6B9:
    tay                         ; $82B6B9
    lda.w #$295A                ; $82B6BA
    sta.w $006C,x               ; $82B6BD
    tya                         ; $82B6C0
    dex                         ; $82B6C1
    dex                         ; $82B6C2
    bpl CODE_82B682             ; $82B6C3
    lda.w $025B                 ; $82B6C5
    lsr a                       ; $82B6C8
    lsr a                       ; $82B6C9
    lsr a                       ; $82B6CA
    and.w #$001E                ; $82B6CB
    tax                         ; $82B6CE
    lda.w $8C35,x               ; $82B6CF
    sta.w $00A0                 ; $82B6D2
    lda.b $4C                   ; $82B6D5
    asl a                       ; $82B6D7
    asl a                       ; $82B6D8
    asl a                       ; $82B6D9
    tax                         ; $82B6DA
    lda.w $A4D4,x               ; $82B6DB
    sta.w $00A4                 ; $82B6DE
    lda.w $A4D6,x               ; $82B6E1
    sta.w $00A6                 ; $82B6E4
    lda.w $A4D8,x               ; $82B6E7
    sta.w $00A8                 ; $82B6EA
    lda.w $A4DA,x               ; $82B6ED
    sta.w $00AA                 ; $82B6F0
    lda.w $03DC                 ; $82B6F3
    and.w #$0003                ; $82B6F6
    beq CODE_82B71D             ; $82B6F9
    incbin !baserom:$136FB..$1371D
CODE_82B71D:
    ldx.b $50                   ; $82B71D
    jsr.w $B838                 ; $82B71F
    lda.w $0226                 ; $82B722
    ldy.w $0235                 ; $82B725
    jsr.w $B769                 ; $82B728
    stx.b $50                   ; $82B72B
    lda.l $7E4E3D               ; $82B72D
    sta.b $14                   ; $82B731
    lda.w $0233                 ; $82B733
    sta.b $16                   ; $82B736
    lda.w #$4E61                ; $82B738
    sta.b $18                   ; $82B73B
    lda.l $7E4E45               ; $82B73D
    sta.b $1A                   ; $82B741
    lda.l $7E4E4D               ; $82B743
    sta.b $1C                   ; $82B747
    lda.w #$0C00                ; $82B749
    sta.b $3A                   ; $82B74C
    jsr.w $B11F                 ; $82B74E
    lda.b $14                   ; $82B751
    sta.l $7E4E3D               ; $82B753
    lda.b $16                   ; $82B757
    sta.w $0233                 ; $82B759
    lda.b $1A                   ; $82B75C
    sta.l $7E4E45               ; $82B75E
    lda.b $1C                   ; $82B762
    sta.l $7E4E4D               ; $82B764
    rts                         ; $82B768
    pha                         ; $82B769
    ldx.w $047B                 ; $82B76A
    lda.l $82DC68,x             ; $82B76D
    bpl CODE_82B785             ; $82B771
    incbin !baserom:$13773..$13785
CODE_82B785:
    pla                         ; $82B785
    jsr.w $B78D                 ; $82B786
    jsr.w $B7B0                 ; $82B789
    rts                         ; $82B78C
    lsr a                       ; $82B78D
    lsr a                       ; $82B78E
    and.w #$FFFC                ; $82B78F
    tax                         ; $82B792
    lda.l $7E4F31,x             ; $82B793
    sec                         ; $82B797
    sbc.w #$0002                ; $82B798
    and.w #$00FF                ; $82B79B
    sta.w $0156                 ; $82B79E
    lda.l $7E4F32,x             ; $82B7A1
    sec                         ; $82B7A5
    sbc.w #$0002                ; $82B7A6
    and.w #$00FF                ; $82B7A9
    sta.w $0158                 ; $82B7AC
    rts                         ; $82B7AF
    lda.b $56                   ; $82B7B0
    and.w #$FFFE                ; $82B7B2
    tay                         ; $82B7B5
    lda.b $58                   ; $82B7B6
    asl a                       ; $82B7B8
    clc                         ; $82B7B9
    adc.w $8C65,y               ; $82B7BA
    tax                         ; $82B7BD
    lda.b $56                   ; $82B7BE
    and.w #$0007                ; $82B7C0
    asl a                       ; $82B7C3
    tay                         ; $82B7C4
    lda.w $8C55,y               ; $82B7C5
    tay                         ; $82B7C8
    lda.l $7E230C,x             ; $82B7C9
    and.w $0000,y               ; $82B7CD
    sta.l $7E230C,x             ; $82B7D0
    lda.l $7E230E,x             ; $82B7D4
    and.w $0002,y               ; $82B7D8
    sta.l $7E230E,x             ; $82B7DB
    lda.l $7E2310,x             ; $82B7DF
    and.w $0004,y               ; $82B7E3
    sta.l $7E2310,x             ; $82B7E6
    lda.l $7E2312,x             ; $82B7EA
    and.w $0006,y               ; $82B7EE
    sta.l $7E2312,x             ; $82B7F1
    lda.l $7E2314,x             ; $82B7F5
    and.w $0008,y               ; $82B7F9
    sta.l $7E2314,x             ; $82B7FC
    lda.l $7E235C,x             ; $82B800
    and.w $000A,y               ; $82B804
    sta.l $7E235C,x             ; $82B807
    lda.l $7E235E,x             ; $82B80B
    and.w $000C,y               ; $82B80F
    sta.l $7E235E,x             ; $82B812
    lda.l $7E2360,x             ; $82B816
    and.w $000E,y               ; $82B81A
    sta.l $7E2360,x             ; $82B81D
    lda.l $7E2362,x             ; $82B821
    and.w $0010,y               ; $82B825
    sta.l $7E2362,x             ; $82B828
    lda.l $7E2364,x             ; $82B82C
    and.w $0012,y               ; $82B830
    sta.l $7E2364,x             ; $82B833
    rts                         ; $82B837
    pea.w $837E                 ; $82B838
    plb                         ; $82B83B
    lda.w $217C,x               ; $82B83C
    sta.w $230C,x               ; $82B83F
    lda.w $217E,x               ; $82B842
    sta.w $230E,x               ; $82B845
    lda.w $2180,x               ; $82B848
    sta.w $2310,x               ; $82B84B
    lda.w $2182,x               ; $82B84E
    sta.w $2312,x               ; $82B851
    lda.w $2184,x               ; $82B854
    sta.w $2314,x               ; $82B857
    lda.w $21CC,x               ; $82B85A
    sta.w $235C,x               ; $82B85D
    lda.w $21CE,x               ; $82B860
    sta.w $235E,x               ; $82B863
    lda.w $21D0,x               ; $82B866
    sta.w $2360,x               ; $82B869
    lda.w $21D2,x               ; $82B86C
    sta.w $2362,x               ; $82B86F
    lda.w $21D4,x               ; $82B872
    sta.w $2364,x               ; $82B875
    plb                         ; $82B878
    rts                         ; $82B879
    incbin !baserom:$1387A..$14BD4
    pea.w $8330                 ; $82CBD4
    plb                         ; $82CBD7
    lda.w $0186                 ; $82CBD8
    sta.w $0184                 ; $82CBDB
    lda.w #$0003                ; $82CBDE
    sta.w $8000                 ; $82CBE1
    lda.b $8E                   ; $82CBE4
    clc                         ; $82CBE6
    adc.w #$0200                ; $82CBE7
    sta.b $1A                   ; $82CBEA
    stz.b $3C                   ; $82CBEC
    lda.b $C8                   ; $82CBEE
    bne CODE_82CBF7             ; $82CBF0
    lda.w #$1000                ; $82CBF2
    sta.b $3C                   ; $82CBF5
CODE_82CBF7:
    lda.w $C000                 ; $82CBF7
    bpl CODE_82CBF7             ; $82CBFA
    lda.w #$0005                ; $82CBFC
    sta.w $8000                 ; $82CBFF
    ldy.w #$0028                ; $82CC02
    lda.l $811E90               ; $82CC05
    tax                         ; $82CC09
    lda.w $0225                 ; $82CC0A
    cmp.l $7EFAB5,x             ; $82CC0D
    lda.w $0227                 ; $82CC11
    sbc.l $7EFAB7,x             ; $82CC14
    bcs CODE_82CC37             ; $82CC18
    ldy.w #$0026                ; $82CC1A
CODE_82CC1D:
    tyx                         ; $82CC1D
    lda.l $811E68,x             ; $82CC1E
    tax                         ; $82CC22
    lda.w $0225                 ; $82CC23
    cmp.l $7EFAB5,x             ; $82CC26
    lda.w $0227                 ; $82CC2A
    sbc.l $7EFAB7,x             ; $82CC2D
    bcs CODE_82CC37             ; $82CC31
    dey                         ; $82CC33
    dey                         ; $82CC34
    bne CODE_82CC1D             ; $82CC35
CODE_82CC37:
    sty.b $28                   ; $82CC37
    lda.w $0226                 ; $82CC39
    sta.b $2C                   ; $82CC3C
CODE_82CC3E:
    lda.w $C000                 ; $82CC3E
    bpl CODE_82CC3E             ; $82CC41
    lda.l $7EFAD1               ; $82CC43
    ora.w #$30FD                ; $82CC47
    tay                         ; $82CC4A
    lda.w $0226                 ; $82CC4B
    lsr a                       ; $82CC4E
    lsr a                       ; $82CC4F
    and.w #$FFFC                ; $82CC50
    tax                         ; $82CC53
    lda.w $1E66                 ; $82CC54
    bpl CODE_82CC5B             ; $82CC57
    incbin !baserom:$14C59..$14C5B
CODE_82CC5B:
    lda.w $0235                 ; $82CC5B
    sta.b $14                   ; $82CC5E
    lda.w #$001E                ; $82CC60
    jsr.w $D8C6                 ; $82CC63
    ldx.w #$0035                ; $82CC66
    lda.l $7EFAD1,x             ; $82CC69
    ora.w #$30FD                ; $82CC6D
    tay                         ; $82CC70
    lda.w $02D9                 ; $82CC71
    lsr a                       ; $82CC74
    lsr a                       ; $82CC75
    and.w #$FFFC                ; $82CC76
    tax                         ; $82CC79
    lda.w $1E68                 ; $82CC7A
    bpl CODE_82CC81             ; $82CC7D
    incbin !baserom:$14C7F..$14C81
CODE_82CC81:
    lda.w $02E8                 ; $82CC81
    sta.b $14                   ; $82CC84
    lda.w #$001E                ; $82CC86
    jsr.w $D8C6                 ; $82CC89
    lda.w #$0009                ; $82CC8C
    sta.w $8000                 ; $82CC8F
    lda.w #$0080                ; $82CC92
    sta.w $8000                 ; $82CC95
    lda.w #$00A9                ; $82CC98
    sta.w $8000                 ; $82CC9B
    stz.w $8000                 ; $82CC9E
    lda.w #$0000                ; $82CCA1
    sta.w $8000                 ; $82CCA4
    lda.w #$0100                ; $82CCA7
    sta.w $8000                 ; $82CCAA
    lda.w #$0004                ; $82CCAD
    sta.w $8000                 ; $82CCB0
    lda.w $1EF2                 ; $82CCB3
    bpl CODE_82CCBE             ; $82CCB6
    incbin !baserom:$14CB8..$14CBE
CODE_82CCBE:
    lda.w #$00DC                ; $82CCBE
    sta.w $8000                 ; $82CCC1
    jmp.w $CDB7                 ; $82CCC4
    incbin !baserom:$14CC7..$14DB7
    stz.b $14                   ; $82CDB7
    lda.w $0226                 ; $82CDB9
    asl a                       ; $82CDBC
    asl a                       ; $82CDBD
    sta.b $16                   ; $82CDBE
    lda.w $019A                 ; $82CDC0
    asl a                       ; $82CDC3
    asl a                       ; $82CDC4
    sta.b $18                   ; $82CDC5
    lda.w $1EDE                 ; $82CDC7
    asl a                       ; $82CDCA
    asl a                       ; $82CDCB
    sta.b $30                   ; $82CDCC
    lda.w $019C                 ; $82CDCE
    clc                         ; $82CDD1
    adc.w $1E5E                 ; $82CDD2
    tax                         ; $82CDD5
    lda.l $840000,x             ; $82CDD6
    clc                         ; $82CDDA
    adc.w #$003F                ; $82CDDB
    asl a                       ; $82CDDE
    asl a                       ; $82CDDF
    sta.b $2E                   ; $82CDE0
    lda.l $840010,x             ; $82CDE2
    asl a                       ; $82CDE6
    asl a                       ; $82CDE7
    bit.w $1E66                 ; $82CDE8
    bpl CODE_82CDF0             ; $82CDEB
    incbin !baserom:$14DED..$14DF0
CODE_82CDF0:
    sta.b $36                   ; $82CDF0
    stz.w $1DF6                 ; $82CDF2
    stz.w $1DF8                 ; $82CDF5
    lda.l $7E34EE               ; $82CDF8
    sec                         ; $82CDFC
    sbc.w $020C                 ; $82CDFD
    sta.l $7E34EC               ; $82CE00
    lda.w $0209                 ; $82CE04
    and.w #$00FF                ; $82CE07
    bit.w #$0080                ; $82CE0A
    beq CODE_82CE12             ; $82CE0D
    incbin !baserom:$14E0F..$14E12
CODE_82CE12:
    eor.w #$FFFF                ; $82CE12
    sec                         ; $82CE15
    adc.l $7E36EE               ; $82CE16
    sta.l $7E36EC               ; $82CE1A
    lda.l $7E34EC               ; $82CE1E
    clc                         ; $82CE22
    adc.l $7E356C               ; $82CE23
    sta.l $7E356C               ; $82CE27
    lda.l $7E36EC               ; $82CE2B
    clc                         ; $82CE2F
    adc.l $7E376C               ; $82CE30
    sta.l $7E376C               ; $82CE34
    lda.w #$0033                ; $82CE38
    sta.l $7E386C               ; $82CE3B
    lda.l $7E39EE               ; $82CE3F
    sec                         ; $82CE43
    sbc.w #$0080                ; $82CE44
    sta.l $7E39EC               ; $82CE47
    jmp.w $CEFD                 ; $82CE4B
    ldx.b $16                   ; $82CE4E
    txa                         ; $82CE50
    clc                         ; $82CE51
    adc.w #$0004                ; $82CE52
    cmp.b $18                   ; $82CE55
    bcc CODE_82CE5B             ; $82CE57
    sbc.b $18                   ; $82CE59
CODE_82CE5B:
    cmp.b $30                   ; $82CE5B
    bne CODE_82CE70             ; $82CE5D
    incbin !baserom:$14E5F..$14E70
CODE_82CE70:
    cmp.b $2E                   ; $82CE70
    bne CODE_82CE82             ; $82CE72
    incbin !baserom:$14E74..$14E82
CODE_82CE82:
    sta.b $16                   ; $82CE82
    lda.l $7E8CB3,x             ; $82CE84
    beq CODE_82CEEE             ; $82CE88
    stx.b $1C                   ; $82CE8A
    xba                         ; $82CE8C
    and.w #$007F                ; $82CE8D
    asl a                       ; $82CE90
    tax                         ; $82CE91
    stx.b $1E                   ; $82CE92
    lda.b $3C                   ; $82CE94
    sta.b $3A                   ; $82CE96
    ldx.b $14                   ; $82CE98
    lda.l $7E39EC,x             ; $82CE9A
    cmp.w #$0082                ; $82CE9E
    bcc CODE_82CEDA             ; $82CEA1
    cmp.w #$0442                ; $82CEA3
    bcs CODE_82CEC1             ; $82CEA6
    ldx.b $1E                   ; $82CEA8
    lda.l $86AD66,x             ; $82CEAA
    tax                         ; $82CEAE
    lda.l $7E45EC,x             ; $82CEAF
    sta.b $26                   ; $82CEB3
    ldx.b $1E                   ; $82CEB5
    jsr.w ($80CE,x)             ; $82CEB7
    db $B0,$02 ; bcs            ; $82CEBA
    bra CODE_82CEFD             ; $82CEBC
    incbin !baserom:$14EBE..$14EC1
CODE_82CEC1:
    ldx.b $1E                   ; $82CEC1
    lda.l $86AD66,x             ; $82CEC3
    tax                         ; $82CEC7
    lda.l $7E495C,x             ; $82CEC8
    sta.b $26                   ; $82CECC
    ldx.b $1E                   ; $82CECE
    jsr.w ($80CE,x)             ; $82CED0
    db $B0,$E9 ; bcs            ; $82CED3
    bra CODE_82CEFD             ; $82CED5
    incbin !baserom:$14ED7..$14EDA
CODE_82CEDA:
    tax                         ; $82CEDA
    lda.l $7E45EC               ; $82CEDB
    sta.b $26                   ; $82CEDF
    lda.b $1E                   ; $82CEE1
    sta.w $1DF6                 ; $82CEE3
    tax                         ; $82CEE6
    jsr.w ($80CE,x)             ; $82CEE7
    db $B0,$D2 ; bcs            ; $82CEEA
    bra CODE_82CEFD             ; $82CEEC
CODE_82CEEE:
    ldx.b $14                   ; $82CEEE
    lda.l $7E386C,x             ; $82CEF0
    sta.w $8000                 ; $82CEF4
    nop                         ; $82CEF7
    nop                         ; $82CEF8
    nop                         ; $82CEF9
    stz.w $8000                 ; $82CEFA
CODE_82CEFD:
    ldx.b $28                   ; $82CEFD
    lda.l $811E6A,x             ; $82CEFF
    tax                         ; $82CF03
    lda.l $7EFAB6,x             ; $82CF04
    cmp.b $2C                   ; $82CF08
    beq CODE_82CF1A             ; $82CF0A
CODE_82CF0C:
    bmi CODE_82CF11             ; $82CF0C
    jmp.w $D1CD                 ; $82CF0E
CODE_82CF11:
    inc.b $28                   ; $82CF11
    inc.b $28                   ; $82CF13
    bra CODE_82CEFD             ; $82CF15
CODE_82CF17:
    jmp.w $D062                 ; $82CF17
CODE_82CF1A:
    txy                         ; $82CF1A
    ldx.b $14                   ; $82CF1B
    inx                         ; $82CF1D
    inx                         ; $82CF1E
    cpx.b $90                   ; $82CF1F
    beq CODE_82CF0C             ; $82CF21
    cpy.w #$0000                ; $82CF23
    bne CODE_82CF17             ; $82CF26
    lda.w $0244                 ; $82CF28
    bit.w #$0001                ; $82CF2B
    bne CODE_82CF17             ; $82CF2E
    lda.w $021A                 ; $82CF30
    bne CODE_82CF74             ; $82CF33
    lda.w $1F18                 ; $82CF35
    sta.w $1F24                 ; $82CF38
    bmi CODE_82CF74             ; $82CF3B
    incbin !baserom:$14F3D..$14F74
CODE_82CF74:
    lda.w $021D                 ; $82CF74
    bpl CODE_82CFA5             ; $82CF77
    incbin !baserom:$14F79..$14FA5
CODE_82CFA5:
    phy                         ; $82CFA5
    phx                         ; $82CFA6
    stz.b $32                   ; $82CFA7
    stz.b $34                   ; $82CFA9
    jsr.w $D267                 ; $82CFAB
    lda.l $7EFABF,x             ; $82CFAE
    sta.w $8000                 ; $82CFB2
    lda.b $3C                   ; $82CFB5
    clc                         ; $82CFB7
    adc.w #$0F00                ; $82CFB8
    sta.w $8000                 ; $82CFBB
    lda.w $023A                 ; $82CFBE
    xba                         ; $82CFC1
    and.w #$00FF                ; $82CFC2
    asl a                       ; $82CFC5
    asl a                       ; $82CFC6
    asl a                       ; $82CFC7
    tax                         ; $82CFC8
    lda.l $86C97A,x             ; $82CFC9
    tax                         ; $82CFCD
    jsr.w $DEA8                 ; $82CFCE
    plx                         ; $82CFD1
    ply                         ; $82CFD2
    jsr.w $D267                 ; $82CFD3
    lda.w #$0000                ; $82CFD6
    sta.w $8000                 ; $82CFD9
    lda.b $3C                   ; $82CFDC
    sta.w $8000                 ; $82CFDE
    lda.w $023A                 ; $82CFE1
    xba                         ; $82CFE4
    and.w #$00FF                ; $82CFE5
    asl a                       ; $82CFE8
    asl a                       ; $82CFE9
    asl a                       ; $82CFEA
    tax                         ; $82CFEB
    lda.l $86CC6A,x             ; $82CFEC
    lda.l $86CC66,x             ; $82CFF0
    pha                         ; $82CFF4
    lda.l $86CC68,x             ; $82CFF5
    tay                         ; $82CFF9
    lda.l $86CC64,x             ; $82CFFA
    plx                         ; $82CFFE
    jsl.l $818487               ; $82CFFF
    lda.w $014C                 ; $82D003
    cmp.w #$0002                ; $82D006
    bne CODE_82D058             ; $82D009
    incbin !baserom:$1500B..$15058
CODE_82D058:
    lda.w #$FFFF                ; $82D058
    sta.w $0281                 ; $82D05B
    clc                         ; $82D05E
    jmp.w $D1B5                 ; $82D05F
    lda.w $0281                 ; $82D062
    bpl CODE_82D088             ; $82D065
    sty.w $0281                 ; $82D067
    stx.w $0283                 ; $82D06A
    tyx                         ; $82D06D
    lda.l $7EFAB5,x             ; $82D06E
    lsr a                       ; $82D072
    and.w #$007E                ; $82D073
    ldx.b $14                   ; $82D076
    clc                         ; $82D078
    adc.l $7E39EC,x             ; $82D079
    tax                         ; $82D07D
    lda.l $83ACEC,x             ; $82D07E
    sta.w $0293                 ; $82D082
    ldx.w $0283                 ; $82D085
CODE_82D088:
    lda.l $7E386A,x             ; $82D088
    sta.w $8000                 ; $82D08C
    lda.w #$9000                ; $82D08F
    sta.w $8000                 ; $82D092
    tyx                         ; $82D095
    lda.l $7EFAB4,x             ; $82D096
    sta.w $8000                 ; $82D09A
    lda.l $7EFADE,x             ; $82D09D
    and.w #$0022                ; $82D0A1
    cmp.w #$0020                ; $82D0A4
    bne CODE_82D10D             ; $82D0A7
    incbin !baserom:$150A9..$1510D
CODE_82D10D:
    ldx.b $14                   ; $82D10D
    lda.l $7E36EE,x             ; $82D10F
    sta.w $8000                 ; $82D113
    lda.l $7E36EC,x             ; $82D116
    sta.w $8000                 ; $82D11A
    lda.l $7E34EE,x             ; $82D11D
    sta.w $8000                 ; $82D121
    lda.l $7E34EC,x             ; $82D124
    sta.w $8000                 ; $82D128
    tyx                         ; $82D12B
    lda.l $7EFAB5,x             ; $82D12C
    lsr a                       ; $82D130
    and.w #$007E                ; $82D131
    ldx.b $14                   ; $82D134
    clc                         ; $82D136
    adc.l $7E39EC,x             ; $82D137
    tax                         ; $82D13B
    lda.l $83ACEC,x             ; $82D13C
    sta.w $8000                 ; $82D140
    phx                         ; $82D143
    tyx                         ; $82D144
    lda.l $7EFABB,x             ; $82D145
    sta.w $8000                 ; $82D149
    nop                         ; $82D14C
    nop                         ; $82D14D
    lda.w $8000                 ; $82D14E
    sta.b $2A                   ; $82D151
    lda.l $7EFABF,x             ; $82D153
    sta.w $8000                 ; $82D157
    lda.b $3C                   ; $82D15A
    ora.l $7EFAD1,x             ; $82D15C
    sta.w $8000                 ; $82D160
    ldx.b $14                   ; $82D163
    lda.l $7E34EC,x             ; $82D165
    sec                         ; $82D169
    sbc.l $7E34EE,x             ; $82D16A
    asl a                       ; $82D16E
    eor.w #$FFFF                ; $82D16F
    sec                         ; $82D172
    adc.b $2A                   ; $82D173
    ldx.w #$0000                ; $82D175
    cmp.w #$FFA0                ; $82D178
    bmi CODE_82D1A2             ; $82D17B
    inx                         ; $82D17D
    inx                         ; $82D17E
    cmp.w #$FFC0                ; $82D17F
    bmi CODE_82D1A2             ; $82D182
    inx                         ; $82D184
    inx                         ; $82D185
    cmp.w #$FFE8                ; $82D186
    bmi CODE_82D1A2             ; $82D189
    inx                         ; $82D18B
    inx                         ; $82D18C
    cmp.w #$0018                ; $82D18D
    bmi CODE_82D1A2             ; $82D190
    inx                         ; $82D192
    inx                         ; $82D193
    cmp.w #$0040                ; $82D194
    bmi CODE_82D1A2             ; $82D197
    incbin !baserom:$15199..$151A2
CODE_82D1A2:
    lda.l $7F0000,x             ; $82D1A2
    plx                         ; $82D1A6
    clc                         ; $82D1A7
    adc.l $849180,x             ; $82D1A8
    tax                         ; $82D1AC
    lda.l $7F0000,x             ; $82D1AD
    tax                         ; $82D1B1
    jsr.w $DEA8                 ; $82D1B2
    bcs CODE_82D1E7             ; $82D1B5
    inc.b $28                   ; $82D1B7
    inc.b $28                   ; $82D1B9
    ldx.b $28                   ; $82D1BB
    lda.l $811E6A,x             ; $82D1BD
    tax                         ; $82D1C1
    lda.l $7EFAB6,x             ; $82D1C2
    cmp.b $2C                   ; $82D1C6
    bne CODE_82D1CD             ; $82D1C8
    incbin !baserom:$151CA..$151CD
CODE_82D1CD:
    inc.b $2C                   ; $82D1CD
    lda.b $2C                   ; $82D1CF
    cmp.w $019A                 ; $82D1D1
    bne CODE_82D1DA             ; $82D1D4
    stz.b $2C                   ; $82D1D6
    stz.b $28                   ; $82D1D8
CODE_82D1DA:
    inc.b $14                   ; $82D1DA
    inc.b $14                   ; $82D1DC
    lda.b $14                   ; $82D1DE
    cmp.b $90                   ; $82D1E0
    beq CODE_82D1E7             ; $82D1E2
    jmp.w $CE4E                 ; $82D1E4
CODE_82D1E7:
    stz.w $8000                 ; $82D1E7
    lda.w #$8000                ; $82D1EA
    sta.w $8000                 ; $82D1ED
    stz.w $1E02                 ; $82D1F0
    ldx.w $1DF6                 ; $82D1F3
    beq CODE_82D23F             ; $82D1F6
    lda.l $86AE36,x             ; $82D1F8
    beq CODE_82D23F             ; $82D1FC
    sta.b $14                   ; $82D1FE
    tax                         ; $82D200
    lda.w #$007F                ; $82D201
    sta.w $1E04                 ; $82D204
    lda.l $7E39EE               ; $82D207
    tax                         ; $82D20B
    lda.l $848000,x             ; $82D20C
    asl a                       ; $82D210
    adc.b $14                   ; $82D211
    tax                         ; $82D213
    lda.l $860006,x             ; $82D214
    sta.w $1DFE                 ; $82D218
    lda.l $860008,x             ; $82D21B
    sta.w $1E02                 ; $82D21F
    ldx.b $14                   ; $82D222
    lda.l $860000,x             ; $82D224
    tax                         ; $82D228
    lda.l $7E6591,x             ; $82D229
    clc                         ; $82D22D
    adc.w $1DFE                 ; $82D22E
    sta.w $1DFE                 ; $82D231
    lda.l $7E45EC               ; $82D234
    asl a                       ; $82D238
    asl a                       ; $82D239
    asl a                       ; $82D23A
    asl a                       ; $82D23B
    sta.w $1E00                 ; $82D23C
CODE_82D23F:
    lda.w $0184                 ; $82D23F
    sta.w $0186                 ; $82D242
    lda.w $023A                 ; $82D245
    xba                         ; $82D248
    and.w #$00FF                ; $82D249
    asl a                       ; $82D24C
    asl a                       ; $82D24D
    asl a                       ; $82D24E
    tax                         ; $82D24F
    lda.l $86C974,x             ; $82D250
    sta.w $1E52                 ; $82D254
    lda.l $86C976,x             ; $82D257
    sta.w $1E54                 ; $82D25B
    lda.l $86C978,x             ; $82D25E
    sta.w $1E56                 ; $82D262
    plb                         ; $82D265
    rts                         ; $82D266
    lda.l $7E386A,x             ; $82D267
    sta.w $8000                 ; $82D26B
    lda.w #$9000                ; $82D26E
    sta.w $8000                 ; $82D271
    tyx                         ; $82D274
    lda.l $7EFAB4,x             ; $82D275
    sta.w $8000                 ; $82D279
    ldx.b $14                   ; $82D27C
    lda.w $0243                 ; $82D27E
    and.w #$00FF                ; $82D281
    bit.w #$0080                ; $82D284
    beq CODE_82D28C             ; $82D287
    incbin !baserom:$15289..$1528C
CODE_82D28C:
    clc                         ; $82D28C
    adc.w #$0033                ; $82D28D
    clc                         ; $82D290
    adc.b $34                   ; $82D291
    sta.w $8000                 ; $82D293
    nop                         ; $82D296
    sta.w $8000                 ; $82D297
    lda.w #$0000                ; $82D29A
    sta.w $8000                 ; $82D29D
    lda.b $32                   ; $82D2A0
    sta.w $8000                 ; $82D2A2
    tyx                         ; $82D2A5
    lda.l $7EFAB5,x             ; $82D2A6
    lsr a                       ; $82D2AA
    and.w #$007E                ; $82D2AB
    ldx.b $14                   ; $82D2AE
    clc                         ; $82D2B0
    adc.l $7E39EC,x             ; $82D2B1
    tax                         ; $82D2B5
    lda.l $83ACEC,x             ; $82D2B6
    sta.w $8000                 ; $82D2BA
    tyx                         ; $82D2BD
    lda.w #$0000                ; $82D2BE
    sta.w $8000                 ; $82D2C1
    nop                         ; $82D2C4
    nop                         ; $82D2C5
    lda.w $8000                 ; $82D2C6
    sta.b $2A                   ; $82D2C9
    rts                         ; $82D2CB
    incbin !baserom:$152CC..$158C6
    pha                         ; $82D8C6
    phx                         ; $82D8C7
    ldx.w $047B                 ; $82D8C8
    lda.l $82DC68,x             ; $82D8CB
    bpl CODE_82D8EC             ; $82D8CF
    incbin !baserom:$158D1..$158EC
CODE_82D8EC:
    plx                         ; $82D8EC
    pla                         ; $82D8ED
    clc                         ; $82D8EE
    adc.l $7E4F32,x             ; $82D8EF
    and.w #$00FF                ; $82D8F3
    pha                         ; $82D8F6
    lda.w #$000B                ; $82D8F7
    sta.w $8000                 ; $82D8FA
    lda.l $7E4F31,x             ; $82D8FD
    and.w #$00FF                ; $82D901
    clc                         ; $82D904
    adc.w #$00BE                ; $82D905
    sta.w $8000                 ; $82D908
    pla                         ; $82D90B
    sta.w $8000                 ; $82D90C
    nop                         ; $82D90F
    sty.w $8000                 ; $82D910
    ldy.w $0184                 ; $82D913
CODE_82D916:
    lda.w $C000                 ; $82D916
    bpl CODE_82D916             ; $82D919
    lda.w $8000                 ; $82D91B
    beq CODE_82D933             ; $82D91E
    lda.w $8000                 ; $82D920
    sta.w $1576,y               ; $82D923
    lda.w $8000                 ; $82D926
    sta.w $1578,y               ; $82D929
    iny                         ; $82D92C
    iny                         ; $82D92D
    iny                         ; $82D92E
    iny                         ; $82D92F
    sty.w $0184                 ; $82D930
CODE_82D933:
    rts                         ; $82D933
    ldx.b $14                   ; $82D934
    lda.l $7E386C,x             ; $82D936
    sta.w $8000                 ; $82D93A
    lda.l $7E38EC,x             ; $82D93D
    sta.w $8000                 ; $82D941
    lda.l $7E35EC,x             ; $82D944
    sta.w $8000                 ; $82D948
    lda.l $7E37EC,x             ; $82D94B
    sta.w $8000                 ; $82D94F
    lda.b $20                   ; $82D952
    sta.w $8000                 ; $82D954
    lda.b $22                   ; $82D957
    sta.w $8000                 ; $82D959
    lda.b $26                   ; $82D95C
    ora.b $3A                   ; $82D95E
    sta.w $8000                 ; $82D960
    ldx.b $24                   ; $82D963
    lda.l $860000,x             ; $82D965
    tax                         ; $82D969
    lda.l $7E6591,x             ; $82D96A
    pha                         ; $82D96E
    ldx.b $14                   ; $82D96F
    lda.l $7E39EC,x             ; $82D971
    tax                         ; $82D975
    lda.l $848000,x             ; $82D976
    clc                         ; $82D97A
    adc.b $24                   ; $82D97B
    tax                         ; $82D97D
    pla                         ; $82D97E
    clc                         ; $82D97F
    adc.l $860002,x             ; $82D980
    tax                         ; $82D984
    jmp.w $DDBF                 ; $82D985
    incbin !baserom:$15988..$15A46
    clc                         ; $82DA46
    rts                         ; $82DA47
    lda.l $86AF06,x             ; $82DA48
    sta.b $24                   ; $82DA4C
    ldx.b $1C                   ; $82DA4E
    lda.l $7E8CB3,x             ; $82DA50
    and.w #$00FF                ; $82DA54
    asl a                       ; $82DA57
    asl a                       ; $82DA58
    asl a                       ; $82DA59
    asl a                       ; $82DA5A
    bit.w #$0800                ; $82DA5B
    beq CODE_82DA63             ; $82DA5E
    ora.w #$F000                ; $82DA60
CODE_82DA63:
    sta.b $20                   ; $82DA63
    lda.l $7E8CB4,x             ; $82DA65
    and.w #$0700                ; $82DA69
    asl a                       ; $82DA6C
    ora.b $26                   ; $82DA6D
    sta.b $26                   ; $82DA6F
    lda.w $0024                 ; $82DA71
    sec                         ; $82DA74
    sbc.l $7E8CB6,x             ; $82DA75
    and.w #$003E                ; $82DA79
    tax                         ; $82DA7C
    lda.l $84A200,x             ; $82DA7D
    clc                         ; $82DA81
    adc.w #$FFC0                ; $82DA82
    sta.b $22                   ; $82DA85
    lda.l $7E8CB5,x             ; $82DA87
    and.w #$0010                ; $82DA8B
    beq CODE_82DAA0             ; $82DA8E
    incbin !baserom:$15A90..$15AA0
CODE_82DAA0:
    jsr.w $D934                 ; $82DAA0
    db $B0,$FA ; bcs            ; $82DAA3
    stz.b $22                   ; $82DAA5
    lda.w #$BC30                ; $82DAA7
    sta.b $24                   ; $82DAAA
    jmp.w $D934                 ; $82DAAC
    incbin !baserom:$15AAF..$15B87
    lda.l $86AF06,x             ; $82DB87
    sta.b $24                   ; $82DB8B
    ldx.b $1C                   ; $82DB8D
    lda.l $7E8CB4,x             ; $82DB8F
    and.w #$0700                ; $82DB93
    asl a                       ; $82DB96
    ora.b $26                   ; $82DB97
    sta.b $26                   ; $82DB99
    lda.l $7E8CB5,x             ; $82DB9B
    and.w #$0030                ; $82DB9F
    lsr a                       ; $82DBA2
    lsr a                       ; $82DBA3
    lsr a                       ; $82DBA4
    txy                         ; $82DBA5
    tax                         ; $82DBA6
    jmp.w ($DBAA,x)             ; $82DBA7
    incbin !baserom:$15BAA..$15BB2
    tyx                         ; $82DBB2
    lda.l $7E8CB3,x             ; $82DBB3
    and.w #$00FF                ; $82DBB7
    asl a                       ; $82DBBA
    asl a                       ; $82DBBB
    asl a                       ; $82DBBC
    asl a                       ; $82DBBD
    bit.w #$0800                ; $82DBBE
    beq CODE_82DBC6             ; $82DBC1
    ora.w #$F000                ; $82DBC3
CODE_82DBC6:
    sta.b $20                   ; $82DBC6
    stz.b $22                   ; $82DBC8
    jsr.w $D934                 ; $82DBCA
    db $B0,$25 ; bcs            ; $82DBCD
    lda.b $20                   ; $82DBCF
    eor.w #$FFFF                ; $82DBD1
    inc a                       ; $82DBD4
    sta.b $20                   ; $82DBD5
    jsr.w $D934                 ; $82DBD7
    db $B0,$18 ; bcs            ; $82DBDA
    lda.b $20                   ; $82DBDC
    sec                         ; $82DBDE
    sbc.w #$0080                ; $82DBDF
    sta.b $20                   ; $82DBE2
    jsr.w $D934                 ; $82DBE4
    db $B0,$0B ; bcs            ; $82DBE7
    lda.b $20                   ; $82DBE9
    eor.w #$FFFF                ; $82DBEB
    inc a                       ; $82DBEE
    sta.b $20                   ; $82DBEF
    jmp.w $D934                 ; $82DBF1
    incbin !baserom:$15BF4..$15DBF
    ldy.b $84                   ; $82DDBF
    lda.l $7F0000,x             ; $82DDC1
    sta.w $8000                 ; $82DDC5
    beq CODE_82DE0F             ; $82DDC8
CODE_82DDCA:
    lda.l $7F0002,x             ; $82DDCA
    sta.w $8000                 ; $82DDCE
    lda.l $7F0004,x             ; $82DDD1
    sta.w $8000                 ; $82DDD5
CODE_82DDD8:
    lda.w $C000                 ; $82DDD8
    bpl CODE_82DDD8             ; $82DDDB
    lda.w $8000                 ; $82DDDD
    beq CODE_82DE00             ; $82DDE0
CODE_82DDE2:
    iny                         ; $82DDE2
    iny                         ; $82DDE3
    lda.w $8000                 ; $82DDE4
    sta.w $1574,y               ; $82DDE7
    iny                         ; $82DDEA
    iny                         ; $82DDEB
    lda.w $8000                 ; $82DDEC
    sta.w $1574,y               ; $82DDEF
    cpy.b $1A                   ; $82DDF2
    db $F0,$6D ; beq            ; $82DDF4
CODE_82DDF6:
    lda.w $C000                 ; $82DDF6
    bpl CODE_82DDF6             ; $82DDF9
    lda.w $8000                 ; $82DDFB
    bne CODE_82DDE2             ; $82DDFE
CODE_82DE00:
    txa                         ; $82DE00
    clc                         ; $82DE01
    adc.w #$0006                ; $82DE02
    tax                         ; $82DE05
    lda.l $7F0000,x             ; $82DE06
    sta.w $8000                 ; $82DE0A
    bne CODE_82DDCA             ; $82DE0D
CODE_82DE0F:
    inx                         ; $82DE0F
    inx                         ; $82DE10
    lda.l $7F0000,x             ; $82DE11
    sta.w $8000                 ; $82DE15
    beq CODE_82DE5F             ; $82DE18
CODE_82DE1A:
    lda.l $7F0002,x             ; $82DE1A
    sta.w $8000                 ; $82DE1E
    lda.l $7F0004,x             ; $82DE21
    sta.w $8000                 ; $82DE25
CODE_82DE28:
    lda.w $C000                 ; $82DE28
    bpl CODE_82DE28             ; $82DE2B
    lda.w $8000                 ; $82DE2D
    beq CODE_82DE50             ; $82DE30
CODE_82DE32:
    iny                         ; $82DE32
    iny                         ; $82DE33
    lda.w $8000                 ; $82DE34
    sta.w $1574,y               ; $82DE37
    iny                         ; $82DE3A
    iny                         ; $82DE3B
    lda.w $8000                 ; $82DE3C
    sta.w $1574,y               ; $82DE3F
    cpy.b $1A                   ; $82DE42
    db $F0,$41 ; beq            ; $82DE44
CODE_82DE46:
    lda.w $C000                 ; $82DE46
    bpl CODE_82DE46             ; $82DE49
    lda.w $8000                 ; $82DE4B
    bne CODE_82DE32             ; $82DE4E
CODE_82DE50:
    txa                         ; $82DE50
    clc                         ; $82DE51
    adc.w #$0006                ; $82DE52
    tax                         ; $82DE55
    lda.l $7F0000,x             ; $82DE56
    sta.w $8000                 ; $82DE5A
    bne CODE_82DE1A             ; $82DE5D
CODE_82DE5F:
    sty.b $84                   ; $82DE5F
    clc                         ; $82DE61
    rts                         ; $82DE62
    incbin !baserom:$15E63..$15EA8
    ldy.b $84                   ; $82DEA8
    lda.l $860000,x             ; $82DEAA
    sta.w $8000                 ; $82DEAE
    beq CODE_82DEF8             ; $82DEB1
CODE_82DEB3:
    lda.l $860002,x             ; $82DEB3
    sta.w $8000                 ; $82DEB7
    lda.l $860004,x             ; $82DEBA
    sta.w $8000                 ; $82DEBE
CODE_82DEC1:
    lda.w $C000                 ; $82DEC1
    bpl CODE_82DEC1             ; $82DEC4
    lda.w $8000                 ; $82DEC6
    beq CODE_82DEE9             ; $82DEC9
CODE_82DECB:
    iny                         ; $82DECB
    iny                         ; $82DECC
    lda.w $8000                 ; $82DECD
    sta.w $1574,y               ; $82DED0
    iny                         ; $82DED3
    iny                         ; $82DED4
    lda.w $8000                 ; $82DED5
    sta.w $1574,y               ; $82DED8
    cpy.b $1A                   ; $82DEDB
    db $F0,$6D ; beq            ; $82DEDD
CODE_82DEDF:
    lda.w $C000                 ; $82DEDF
    bpl CODE_82DEDF             ; $82DEE2
    lda.w $8000                 ; $82DEE4
    bne CODE_82DECB             ; $82DEE7
CODE_82DEE9:
    txa                         ; $82DEE9
    clc                         ; $82DEEA
    adc.w #$0006                ; $82DEEB
    tax                         ; $82DEEE
    lda.l $860000,x             ; $82DEEF
    sta.w $8000                 ; $82DEF3
    bne CODE_82DEB3             ; $82DEF6
CODE_82DEF8:
    inx                         ; $82DEF8
    inx                         ; $82DEF9
    lda.l $860000,x             ; $82DEFA
    sta.w $8000                 ; $82DEFE
    beq CODE_82DF48             ; $82DF01
CODE_82DF03:
    lda.l $860002,x             ; $82DF03
    sta.w $8000                 ; $82DF07
    lda.l $860004,x             ; $82DF0A
    sta.w $8000                 ; $82DF0E
CODE_82DF11:
    lda.w $C000                 ; $82DF11
    bpl CODE_82DF11             ; $82DF14
    lda.w $8000                 ; $82DF16
    beq CODE_82DF39             ; $82DF19
CODE_82DF1B:
    iny                         ; $82DF1B
    iny                         ; $82DF1C
    lda.w $8000                 ; $82DF1D
    sta.w $1574,y               ; $82DF20
    iny                         ; $82DF23
    iny                         ; $82DF24
    lda.w $8000                 ; $82DF25
    sta.w $1574,y               ; $82DF28
    cpy.b $1A                   ; $82DF2B
    db $F0,$41 ; beq            ; $82DF2D
CODE_82DF2F:
    lda.w $C000                 ; $82DF2F
    bpl CODE_82DF2F             ; $82DF32
    lda.w $8000                 ; $82DF34
    bne CODE_82DF1B             ; $82DF37
CODE_82DF39:
    txa                         ; $82DF39
    clc                         ; $82DF3A
    adc.w #$0006                ; $82DF3B
    tax                         ; $82DF3E
    lda.l $860000,x             ; $82DF3F
    sta.w $8000                 ; $82DF43
    bne CODE_82DF03             ; $82DF46
CODE_82DF48:
    sty.b $84                   ; $82DF48
    clc                         ; $82DF4A
    rts                         ; $82DF4B
    incbin !baserom:$15F4C..$15FA9
    jsr.w $DFAD                 ; $82DFA9
    rts                         ; $82DFAC
    pea.w $837E                 ; $82DFAD
    plb                         ; $82DFB0
    lda.w $023E                 ; $82DFB1
    clc                         ; $82DFB4
    adc.w $1EE6                 ; $82DFB5
    asl a                       ; $82DFB8
    and.w #$003E                ; $82DFB9
    tax                         ; $82DFBC
    lda.w $0226                 ; $82DFBD
    sec                         ; $82DFC0
    sbc.w $023E                 ; $82DFC1
    beq CODE_82E03F             ; $82DFC4
    tay                         ; $82DFC6
    lda.w #$0000                ; $82DFC7
    sta.l $308000               ; $82DFCA
    sec                         ; $82DFCE
    sbc.w $023C                 ; $82DFCF
    xba                         ; $82DFD2
    and.w #$00FF                ; $82DFD3
    sta.l $308000               ; $82DFD6
    lda.w $3AEE,x               ; $82DFDA
    sta.l $308000               ; $82DFDD
    nop                         ; $82DFE1
    lda.l $308000               ; $82DFE2
    xba                         ; $82DFE6
    pha                         ; $82DFE7
    and.w #$FF00                ; $82DFE8
    sta.b $14                   ; $82DFEB
    pla                         ; $82DFED
    and.w #$00FF                ; $82DFEE
    sta.b $16                   ; $82DFF1
    lda.l $308000               ; $82DFF3
    xba                         ; $82DFF7
    and.w #$FF00                ; $82DFF8
    ora.b $16                   ; $82DFFB
    sta.w $01CE                 ; $82DFFD
    inx                         ; $82E000
    inx                         ; $82E001
    dey                         ; $82E002
    beq CODE_82E010             ; $82E003
CODE_82E005:
    adc.w $3AEE,x               ; $82E005
    inx                         ; $82E008
    inx                         ; $82E009
    dey                         ; $82E00A
    bne CODE_82E005             ; $82E00B
    sta.w $01CE                 ; $82E00D
CODE_82E010:
    lda.w #$0000                ; $82E010
    sta.l $308000               ; $82E013
    lda.w $0224                 ; $82E017
    and.w #$FF00                ; $82E01A
    lsr a                       ; $82E01D
    sta.l $308000               ; $82E01E
    lda.w $3AEE,x               ; $82E022
    asl a                       ; $82E025
    sta.l $308000               ; $82E026
    nop                         ; $82E02A
    lda.l $308000               ; $82E02B
    clc                         ; $82E02F
    adc.b $14                   ; $82E030
    lda.l $308000               ; $82E032
    clc                         ; $82E036
    adc.w $01CE                 ; $82E037
    sta.w $01CE                 ; $82E03A
    bra CODE_82E069             ; $82E03D
CODE_82E03F:
    lda.w #$0000                ; $82E03F
    sta.l $308000               ; $82E042
    lda.w $0224                 ; $82E046
    and.w #$FF00                ; $82E049
    sec                         ; $82E04C
    sbc.w $023C                 ; $82E04D
    lsr a                       ; $82E050
    sta.l $308000               ; $82E051
    lda.w $3AEE,x               ; $82E055
    asl a                       ; $82E058
    sta.l $308000               ; $82E059
    nop                         ; $82E05D
    lda.l $308000               ; $82E05E
    lda.l $308000               ; $82E062
    sta.w $01CE                 ; $82E066
CODE_82E069:
    lda.w $023E                 ; $82E069
    clc                         ; $82E06C
    adc.w #$0020                ; $82E06D
    clc                         ; $82E070
    adc.w $1EE6                 ; $82E071
    cmp.w $019A                 ; $82E074
    bcc CODE_82E07C             ; $82E077
    sbc.w $019A                 ; $82E079
CODE_82E07C:
    sta.b $16                   ; $82E07C
    lda.w $0224                 ; $82E07E
    and.w #$FF00                ; $82E081
    sta.w $023C                 ; $82E084
    lda.w $0226                 ; $82E087
    sec                         ; $82E08A
    sbc.w $023E                 ; $82E08B
    bne CODE_82E093             ; $82E08E
    jmp.w $E497                 ; $82E090
CODE_82E093:
    sta.b $14                   ; $82E093
    clc                         ; $82E095
    adc.w $0240                 ; $82E096
    sta.w $0240                 ; $82E099
    lda.w $1EDE                 ; $82E09C
    sta.b $22                   ; $82E09F
    lda.w $023E                 ; $82E0A1
    asl a                       ; $82E0A4
    and.w #$003E                ; $82E0A5
    tax                         ; $82E0A8
    lda.l $7E3E6E,x             ; $82E0A9
    sec                         ; $82E0AD
    sbc.l $7E3E6C,x             ; $82E0AE
    sta.w $1EF8                 ; $82E0B2
    lda.l $7E3DEE,x             ; $82E0B5
    sec                         ; $82E0B9
    sbc.l $7E3DEC,x             ; $82E0BA
    sta.w $1EFA                 ; $82E0BE
    lda.b $16                   ; $82E0C1
    cmp.b $22                   ; $82E0C3
    bcc CODE_82E0D2             ; $82E0C5
    incbin !baserom:$160C7..$160D2
CODE_82E0D2:
    lda.w #$0000                ; $82E0D2
    sta.l $308000               ; $82E0D5
    lda.b $16                   ; $82E0D9
    asl a                       ; $82E0DB
    asl a                       ; $82E0DC
    xba                         ; $82E0DD
    and.w #$00FF                ; $82E0DE
    sta.l $308000               ; $82E0E1
    lda.w #$000A                ; $82E0E5
    sta.l $308000               ; $82E0E8
    nop                         ; $82E0EC
    lda.l $308000               ; $82E0ED
    clc                         ; $82E0F1
    adc.b $98                   ; $82E0F2
    sta.b $18                   ; $82E0F4
    tax                         ; $82E0F6
    lda.l $308000               ; $82E0F7
    lda.b $16                   ; $82E0FB
    asl a                       ; $82E0FD
    and.w #$003E                ; $82E0FE
    tay                         ; $82E101
    sta.b $1A                   ; $82E102
    lda.b $16                   ; $82E104
    and.w #$003E                ; $82E106
    sta.b $1C                   ; $82E109
    adc.l $840000,x             ; $82E10B
    tax                         ; $82E10F
    lda.l $850000,x             ; $82E110
    sta.w $3AEC,y               ; $82E114
    sta.w $3B2C,y               ; $82E117
    ldx.b $18                   ; $82E11A
    lda.b $1C                   ; $82E11C
    clc                         ; $82E11E
    adc.l $840002,x             ; $82E11F
    tax                         ; $82E123
    lda.l $850000,x             ; $82E124
    sec                         ; $82E128
    sbc.w $1E1E                 ; $82E129
    sta.w $3B6C,y               ; $82E12C
    sta.w $3BAC,y               ; $82E12F
    tya                         ; $82E132
    and.w #$0006                ; $82E133
    cmp.w #$0006                ; $82E136
    bne CODE_82E142             ; $82E139
    lda.l $850000,x             ; $82E13B
    sta.w $1E1E                 ; $82E13F
CODE_82E142:
    ldx.b $18                   ; $82E142
    tya                         ; $82E144
    bit.w #$0002                ; $82E145
    beq CODE_82E151             ; $82E148
    lda.b $1C                   ; $82E14A
    ora.w #$0001                ; $82E14C
    bra CODE_82E153             ; $82E14F
CODE_82E151:
    lda.b $1C                   ; $82E151
CODE_82E153:
    asl a                       ; $82E153
    bit.w #$0040                ; $82E154
    bne CODE_82E15F             ; $82E157
    adc.l $840006,x             ; $82E159
    bra CODE_82E166             ; $82E15D
CODE_82E15F:
    and.w #$003F                ; $82E15F
    adc.l $840008,x             ; $82E162
CODE_82E166:
    tax                         ; $82E166
    lda.l $7E5391,x             ; $82E167
    sta.w $3CEC,y               ; $82E16B
    sta.w $3D2C,y               ; $82E16E
    ldx.b $18                   ; $82E171
    lda.l $840004,x             ; $82E173
    tax                         ; $82E177
    lda.l $850000,x             ; $82E178
    tay                         ; $82E17C
    lda.l $850002,x             ; $82E17D
    cmp.b $1A                   ; $82E181
    bcs CODE_82E19A             ; $82E183
    incbin !baserom:$16185..$1619A
CODE_82E19A:
    tya                         ; $82E19A
    ldy.b $1A                   ; $82E19B
    sta.w $3BEC,y               ; $82E19D
    sta.w $3C2C,y               ; $82E1A0
    lda.w #$0000                ; $82E1A3
    sta.w $3D6C,y               ; $82E1A6
    sta.w $3DAC,y               ; $82E1A9
    lda.b $9C                   ; $82E1AC
    clc                         ; $82E1AE
    adc.b $A2                   ; $82E1AF
    tax                         ; $82E1B1
    lda.b $16                   ; $82E1B2
    sec                         ; $82E1B4
    sbc.l $840000,x             ; $82E1B5
    db $B0,$18 ; bcs            ; $82E1B9
    lda.b $16                   ; $82E1BB
    inc a                       ; $82E1BD
    cmp.b $22                   ; $82E1BE
    bne CODE_82E1D0             ; $82E1C0
    incbin !baserom:$161C2..$161D0
CODE_82E1D0:
    jmp.w $E2C3                 ; $82E1D0
    incbin !baserom:$161D3..$162C3
    lda.b $16                   ; $82E2C3
    and.w #$0007                ; $82E2C5
    asl a                       ; $82E2C8
    asl a                       ; $82E2C9
    asl a                       ; $82E2CA
    clc                         ; $82E2CB
    adc.w $1F0A                 ; $82E2CC
    sta.w $3FEC,y               ; $82E2CF
    sta.w $402C,y               ; $82E2D2
    lda.w #$0000                ; $82E2D5
    sta.w $3DEC,y               ; $82E2D8
    sta.w $3E2C,y               ; $82E2DB
    sta.w $3E6C,y               ; $82E2DE
    sta.w $3EAC,y               ; $82E2E1
    lda.w $1F0E                 ; $82E2E4
    sta.w $3C6C,y               ; $82E2E7
    sta.w $3CAC,y               ; $82E2EA
    lda.w $1F10                 ; $82E2ED
    sta.w $3F6C,y               ; $82E2F0
    sta.w $3FAC,y               ; $82E2F3
    lda.b $16                   ; $82E2F6
    asl a                       ; $82E2F8
    asl a                       ; $82E2F9
    tax                         ; $82E2FA
    tay                         ; $82E2FB
    lda.l $7E8CB4,x             ; $82E2FC
    and.w #$00FF                ; $82E300
    beq CODE_82E30A             ; $82E303
    asl a                       ; $82E305
    tax                         ; $82E306
    jsr.w ($819E,x)             ; $82E307
CODE_82E30A:
    inc.b $16                   ; $82E30A
    lda.b $16                   ; $82E30C
    cmp.w $019A                 ; $82E30E
    bne CODE_82E317             ; $82E311
    stz.b $16                   ; $82E313
    stz.b $A2                   ; $82E315
CODE_82E317:
    dec.b $14                   ; $82E317
    beq CODE_82E31E             ; $82E319
    jmp.w $E0C1                 ; $82E31B
CODE_82E31E:
    pea.w $837E                 ; $82E31E
    plb                         ; $82E321
    ldx.b $A0                   ; $82E322
    lda.w $0226                 ; $82E324
    cmp.l $7E5B91,x             ; $82E327
    db $B0,$03 ; bcs            ; $82E32B
    jmp.w $E429                 ; $82E32D
    incbin !baserom:$16330..$16429
    plb                         ; $82E429
    lda.w $0226                 ; $82E42A
    cmp.w $019A                 ; $82E42D
    bcc CODE_82E44D             ; $82E430
    incbin !baserom:$16432..$1644D
CODE_82E44D:
    sta.w $0226                 ; $82E44D
    sta.w $023E                 ; $82E450
    stz.w $0260                 ; $82E453
    ldx.b $9C                   ; $82E456
    lda.w $0226                 ; $82E458
    inc a                       ; $82E45B
    inc a                       ; $82E45C
    sec                         ; $82E45D
    sbc.l $840000,x             ; $82E45E
    bcc CODE_82E48E             ; $82E462
    incbin !baserom:$16464..$1648E
CODE_82E48E:
    lda.w $0260                 ; $82E48E
    and.w #$00FF                ; $82E491
    sta.w $0142                 ; $82E494
    lda.w $0226                 ; $82E497
    clc                         ; $82E49A
    adc.w #$0001                ; $82E49B
    cmp.w $019A                 ; $82E49E
    bcc CODE_82E4A6             ; $82E4A1
    incbin !baserom:$164A3..$164A6
CODE_82E4A6:
    jsr.w $EA49                 ; $82E4A6
    sta.b $14                   ; $82E4A9
    lda.w $0226                 ; $82E4AB
    sec                         ; $82E4AE
    sbc.w #$0003                ; $82E4AF
    bcs CODE_82E4B7             ; $82E4B2
    incbin !baserom:$164B4..$164B7
CODE_82E4B7:
    jsr.w $EA49                 ; $82E4B7
    sta.b $16                   ; $82E4BA
    lda.w #$0000                ; $82E4BC
    sta.l $308000               ; $82E4BF
    lda.b $14                   ; $82E4C3
    sec                         ; $82E4C5
    sbc.b $16                   ; $82E4C6
    sta.l $308000               ; $82E4C8
    lda.w $0225                 ; $82E4CC
    clc                         ; $82E4CF
    adc.w #$0100                ; $82E4D0
    and.w #$03FF                ; $82E4D3
    sta.l $308000               ; $82E4D6
    lda.b $16                   ; $82E4DA
    cmp.w #$8000                ; $82E4DC
    rol a                       ; $82E4DF
    rol a                       ; $82E4E0
    xba                         ; $82E4E1
    and.w #$03FF                ; $82E4E2
    bit.w #$0080                ; $82E4E5
    beq CODE_82E4ED             ; $82E4E8
    incbin !baserom:$164EA..$164ED
CODE_82E4ED:
    sta.l $308000               ; $82E4ED
    clc                         ; $82E4F1
    adc.l $308000               ; $82E4F2
    clc                         ; $82E4F6
    adc.w #$0009                ; $82E4F7
    eor.w #$FFFF                ; $82E4FA
    inc a                       ; $82E4FD
    sta.w $0237                 ; $82E4FE
    plb                         ; $82E501
    rts                         ; $82E502
    incbin !baserom:$16503..$16A49
    tay                         ; $82EA49
    lda.w #$0000                ; $82EA4A
    sta.l $308000               ; $82EA4D
    tya                         ; $82EA51
    asl a                       ; $82EA52
    asl a                       ; $82EA53
    xba                         ; $82EA54
    and.w #$00FF                ; $82EA55
    sta.l $308000               ; $82EA58
    lda.w #$000A                ; $82EA5C
    sta.l $308000               ; $82EA5F
    nop                         ; $82EA63
    lda.l $308000               ; $82EA64
    clc                         ; $82EA68
    adc.b $98                   ; $82EA69
    sta.b $18                   ; $82EA6B
    tax                         ; $82EA6D
    lda.l $308000               ; $82EA6E
    tya                         ; $82EA72
    and.w #$003E                ; $82EA73
    adc.l $840002,x             ; $82EA76
    tax                         ; $82EA7A
    lda.l $850000,x             ; $82EA7B
    rts                         ; $82EA7F
    rts                         ; $82EA80
    incbin !baserom:$16A81..$16AD4
    lda.w #$C2E7                ; $82EAD4
    jmp.w $EB1A                 ; $82EAD7
    ldx.b $1A                   ; $82EADA
    lda.w #$0000                ; $82EADC
    sta.w $1F0E                 ; $82EADF
    sta.w $3C6C,x               ; $82EAE2
    sta.w $3CAC,x               ; $82EAE5
    sta.w $1F10                 ; $82EAE8
    sta.w $3F6C,x               ; $82EAEB
    sta.w $3FAC,x               ; $82EAEE
    lda.w #$0001                ; $82EAF1
    ora.w $3D6C,x               ; $82EAF4
    sta.w $3D6C,x               ; $82EAF7
    sta.w $3DAC,x               ; $82EAFA
    rts                         ; $82EAFD
    incbin !baserom:$16AFE..$16B1A
    sta.w $1F0A                 ; $82EB1A
    tyx                         ; $82EB1D
    jsr.w $EC09                 ; $82EB1E
    ldx.b $1A                   ; $82EB21
    sta.w $1F0E                 ; $82EB23
    sta.w $3C6C,x               ; $82EB26
    sta.w $3CAC,x               ; $82EB29
    clc                         ; $82EB2C
    adc.b $1E                   ; $82EB2D
    sta.w $1F10                 ; $82EB2F
    sta.w $3F6C,x               ; $82EB32
    sta.w $3FAC,x               ; $82EB35
    lda.w #$0001                ; $82EB38
    ora.w $3D6C,x               ; $82EB3B
    sta.w $3D6C,x               ; $82EB3E
    sta.w $3DAC,x               ; $82EB41
    rts                         ; $82EB44
    incbin !baserom:$16B45..$16C09
    lda.l $7E8CB5,x             ; $82EC09
    and.w #$00FF                ; $82EC0D
    sta.b $1E                   ; $82EC10
    lda.l $7E8CB3,x             ; $82EC12
    and.w #$00FF                ; $82EC16
    asl a                       ; $82EC19
    asl a                       ; $82EC1A
    asl a                       ; $82EC1B
    asl a                       ; $82EC1C
    bit.w #$0800                ; $82EC1D
    beq CODE_82EC25             ; $82EC20
    incbin !baserom:$16C22..$16C25
CODE_82EC25:
    sec                         ; $82EC25
    sbc.b $1E                   ; $82EC26
    asl.b $1E                   ; $82EC28
    rts                         ; $82EC2A
    incbin !baserom:$16C2B..$16C32
    jsr.w $EC36                 ; $82EC32
    rts                         ; $82EC35
    pea.w $8330                 ; $82EC36
    plb                         ; $82EC39
    stz.b $3C                   ; $82EC3A
    lda.w $0226                 ; $82EC3C
    asl a                       ; $82EC3F
    and.w #$003E                ; $82EC40
    sta.b $16                   ; $82EC43
    tax                         ; $82EC45
    stz.w $8000                 ; $82EC46
    lda.l $7E3DEE,x             ; $82EC49
    sec                         ; $82EC4D
    sbc.l $7E3DEC,x             ; $82EC4E
    asl a                       ; $82EC52
    sta.w $8000                 ; $82EC53
    lda.w $0224                 ; $82EC56
    lsr a                       ; $82EC59
    and.w #$FF80                ; $82EC5A
    sta.w $8000                 ; $82EC5D
    sta.b $14                   ; $82EC60
    lda.w $8000                 ; $82EC62
    sta.w $1EF4                 ; $82EC65
    lda.w $8000                 ; $82EC68
    clc                         ; $82EC6B
    adc.l $7E3DEC,x             ; $82EC6C
    beq CODE_82EC75             ; $82EC70
    incbin !baserom:$16C72..$16C75
CODE_82EC75:
    sta.w $1EF6                 ; $82EC75
    lda.w $0261                 ; $82EC78
    and.w #$00FF                ; $82EC7B
    tax                         ; $82EC7E
    jsr.w ($ED23,x)             ; $82EC7F
    lda.w $1EF4                 ; $82EC82
    pha                         ; $82EC85
    lda.w $1EF6                 ; $82EC86
    pha                         ; $82EC89
    lda.w $0260                 ; $82EC8A
    and.w #$00FF                ; $82EC8D
    tax                         ; $82EC90
    jsr.w ($ED17,x)             ; $82EC91
    pla                         ; $82EC94
    sta.w $1EC4                 ; $82EC95
    pla                         ; $82EC98
    sta.w $1EC2                 ; $82EC99
    lda.w $01CE                 ; $82EC9C
    sta.w $0229                 ; $82EC9F
    lda.w $01CE                 ; $82ECA2
    bpl CODE_82ECC1             ; $82ECA5
    eor.w #$FFFF                ; $82ECA7
    inc a                       ; $82ECAA
    sta.w $4204                 ; $82ECAB
    sep.b #$20                  ; $82ECAE
    lda.w $1E9E                 ; $82ECB0
    sta.w $4206                 ; $82ECB3
    rep.b #$20                  ; $82ECB6
    nop                         ; $82ECB8
    stz.w $8000                 ; $82ECB9
    lda.w #$0001                ; $82ECBC
    bra CODE_82ECD5             ; $82ECBF
CODE_82ECC1:
    sta.w $4204                 ; $82ECC1
    sep.b #$20                  ; $82ECC4
    lda.w $1E9E                 ; $82ECC6
    sta.w $4206                 ; $82ECC9
    rep.b #$20                  ; $82ECCC
    nop                         ; $82ECCE
    stz.w $8000                 ; $82ECCF
    lda.w #$FFFF                ; $82ECD2
CODE_82ECD5:
    sta.w $8000                 ; $82ECD5
    lda.w $4214                 ; $82ECD8
    sta.w $8000                 ; $82ECDB
    nop                         ; $82ECDE
    lda.w $8000                 ; $82ECDF
    sta.w $01CE                 ; $82ECE2
    lda.w $8000                 ; $82ECE5
    plb                         ; $82ECE8
    rts                         ; $82ECE9
    stz.w $0242                 ; $82ECEA
    stz.w $0208                 ; $82ECED
    rts                         ; $82ECF0
    stz.w $1EEE                 ; $82ECF1
    stz.w $1EF0                 ; $82ECF4
    lda.w $0204                 ; $82ECF7
    sta.w $0200                 ; $82ECFA
    clc                         ; $82ECFD
    adc.w $1EF4                 ; $82ECFE
    sta.w $1EF4                 ; $82ED01
    lda.w $0206                 ; $82ED04
    sta.w $0202                 ; $82ED07
    adc.w $1EF6                 ; $82ED0A
    sta.w $1EF6                 ; $82ED0D
    stz.w $020A                 ; $82ED10
    stz.w $020C                 ; $82ED13
    rts                         ; $82ED16
    incbin !baserom:$16D17..$172B3
    stz.b $2A                   ; $82F2B3
    lda.w $0226                 ; $82F2B5
    cmp.w $1EDE                 ; $82F2B8
    bcc CODE_82F31D             ; $82F2BB
    incbin !baserom:$172BD..$1731D
CODE_82F31D:
    lda.w $019C                 ; $82F31D
    clc                         ; $82F320
    adc.w $1E5E                 ; $82F321
    tax                         ; $82F324
    lda.w $0226                 ; $82F325
    clc                         ; $82F328
    adc.w #$0002                ; $82F329
    sbc.l $840000,x             ; $82F32C
    cmp.w #$0040                ; $82F330
    bmi CODE_82F34D             ; $82F333
    incbin !baserom:$17335..$1734D
CODE_82F34D:
    lda.w $019C                 ; $82F34D
    clc                         ; $82F350
    adc.w $1E60                 ; $82F351
    tax                         ; $82F354
    lda.w $0226                 ; $82F355
    clc                         ; $82F358
    adc.w #$0002                ; $82F359
    sbc.l $840000,x             ; $82F35C
    bmi CODE_82F373             ; $82F360
    incbin !baserom:$17362..$17373
CODE_82F373:
    rts                         ; $82F373
    incbin !baserom:$17374..$17A4F
    stz.b $2A                   ; $82FA4F
    lda.w $02D9                 ; $82FA51
    cmp.w $1EE2                 ; $82FA54
    bcc CODE_82FAB9             ; $82FA57
    incbin !baserom:$17A59..$17AB9
CODE_82FAB9:
    lda.w $019C                 ; $82FAB9
    clc                         ; $82FABC
    adc.w $1E62                 ; $82FABD
    tax                         ; $82FAC0
    lda.w $02D9                 ; $82FAC1
    clc                         ; $82FAC4
    adc.w #$0002                ; $82FAC5
    sbc.l $840000,x             ; $82FAC8
    cmp.w #$0040                ; $82FACC
    bmi CODE_82FAE9             ; $82FACF
    incbin !baserom:$17AD1..$17AE9
CODE_82FAE9:
    lda.w $019C                 ; $82FAE9
    clc                         ; $82FAEC
    adc.w $1E64                 ; $82FAED
    tax                         ; $82FAF0
    lda.w $02D9                 ; $82FAF1
    clc                         ; $82FAF4
    adc.w #$0002                ; $82FAF5
    sbc.l $840000,x             ; $82FAF8
    bmi CODE_82FB0F             ; $82FAFC
    incbin !baserom:$17AFE..$17B0F
CODE_82FB0F:
    rts                         ; $82FB0F
    incbin !baserom:$17B10..$17B70
    phy                         ; $82FB70
    phb                         ; $82FB71
    sep.b #$20                  ; $82FB72
    pha                         ; $82FB74
    plb                         ; $82FB75
    xba                         ; $82FB76
    sta.b $02                   ; $82FB77
    rep.b #$20                  ; $82FB79
    stz.b $00                   ; $82FB7B
    stx.b $04                   ; $82FB7D
    ldx.b $04                   ; $82FB7F
    lda.w $0000,x               ; $82FB81
    and.w #$00FE                ; $82FB84
    tax                         ; $82FB87
    jmp.w ($FB8B,x)             ; $82FB88
    incbin !baserom:$17B8B..$17C8B
    ldx.b $04                   ; $82FC8B
    lda.w $0000,x               ; $82FC8D
    inx                         ; $82FC90
    and.w #$00FF                ; $82FC91
    beq CODE_82FCC6             ; $82FC94
    lsr a                       ; $82FC96
    sta.b $08                   ; $82FC97
    bcc CODE_82FCA4             ; $82FC99
    beq CODE_82FCB6             ; $82FC9B
    lda.w $0000,x               ; $82FC9D
    sta.b [$00],y               ; $82FCA0
    inx                         ; $82FCA2
    iny                         ; $82FCA3
CODE_82FCA4:
    lda.w $0000,x               ; $82FCA4
    sta.b [$00],y               ; $82FCA7
    inx                         ; $82FCA9
    iny                         ; $82FCAA
    inx                         ; $82FCAB
    iny                         ; $82FCAC
    dec.b $08                   ; $82FCAD
    bne CODE_82FCA4             ; $82FCAF
    stx.b $04                   ; $82FCB1
    jmp.w $FB7F                 ; $82FCB3
CODE_82FCB6:
    sep.b #$20                  ; $82FCB6
    lda.w $0000,x               ; $82FCB8
    sta.b [$00],y               ; $82FCBB
    inx                         ; $82FCBD
    iny                         ; $82FCBE
    rep.b #$20                  ; $82FCBF
    stx.b $04                   ; $82FCC1
    jmp.w $FB7F                 ; $82FCC3
CODE_82FCC6:
    plb                         ; $82FCC6
    sty.b $0A                   ; $82FCC7
    pla                         ; $82FCC9
    eor.w #$FFFF                ; $82FCCA
    sec                         ; $82FCCD
    adc.b $0A                   ; $82FCCE
    rtl                         ; $82FCD0
    ldx.b $04                   ; $82FCD1
    lda.w $0000,x               ; $82FCD3
    inx                         ; $82FCD6
    stx.b $04                   ; $82FCD7
    and.w #$000F                ; $82FCD9
    tax                         ; $82FCDC
    inx                         ; $82FCDD
    sep.b #$20                  ; $82FCDE
    dey                         ; $82FCE0
    lda.b [$00],y               ; $82FCE1
    dey                         ; $82FCE3
    sec                         ; $82FCE4
    sbc.b [$00],y               ; $82FCE5
    iny                         ; $82FCE7
    sta.b $0A                   ; $82FCE8
    lda.b [$00],y               ; $82FCEA
    iny                         ; $82FCEC
    clc                         ; $82FCED
    adc.b $0A                   ; $82FCEE
    sta.b [$00],y               ; $82FCF0
    iny                         ; $82FCF2
    clc                         ; $82FCF3
    adc.b $0A                   ; $82FCF4
    sta.b [$00],y               ; $82FCF6
    iny                         ; $82FCF8
CODE_82FCF9:
    clc                         ; $82FCF9
    adc.b $0A                   ; $82FCFA
    sta.b [$00],y               ; $82FCFC
    iny                         ; $82FCFE
    dex                         ; $82FCFF
    bne CODE_82FCF9             ; $82FD00
    rep.b #$20                  ; $82FD02
    jmp.w $FB7F                 ; $82FD04
    ldx.b $04                   ; $82FD07
    lda.w $0000,x               ; $82FD09
    inx                         ; $82FD0C
    stx.b $04                   ; $82FD0D
    and.w #$000F                ; $82FD0F
    tax                         ; $82FD12
    inx                         ; $82FD13
    dey                         ; $82FD14
    dey                         ; $82FD15
    lda.b [$00],y               ; $82FD16
    dey                         ; $82FD18
    dey                         ; $82FD19
    sec                         ; $82FD1A
    sbc.b [$00],y               ; $82FD1B
    iny                         ; $82FD1D
    iny                         ; $82FD1E
    sta.b $0A                   ; $82FD1F
    lda.b [$00],y               ; $82FD21
    iny                         ; $82FD23
    iny                         ; $82FD24
    clc                         ; $82FD25
    adc.b $0A                   ; $82FD26
    sta.b [$00],y               ; $82FD28
    iny                         ; $82FD2A
    iny                         ; $82FD2B
CODE_82FD2C:
    clc                         ; $82FD2C
    adc.b $0A                   ; $82FD2D
    sta.b [$00],y               ; $82FD2F
    iny                         ; $82FD31
    iny                         ; $82FD32
    dex                         ; $82FD33
    bne CODE_82FD2C             ; $82FD34
    jmp.w $FB7F                 ; $82FD36
    ldx.b $04                   ; $82FD39
    lda.w $0000,x               ; $82FD3B
    inx                         ; $82FD3E
    stx.b $04                   ; $82FD3F
    and.w #$000F                ; $82FD41
    asl a                       ; $82FD44
    tax                         ; $82FD45
    dey                         ; $82FD46
    sep.b #$20                  ; $82FD47
    lda.b [$00],y               ; $82FD49
    xba                         ; $82FD4B
    lda.b [$00],y               ; $82FD4C
    rep.b #$20                  ; $82FD4E
    iny                         ; $82FD50
    sta.b [$00],y               ; $82FD51
    iny                         ; $82FD53
    iny                         ; $82FD54
    jmp.w ($FD58,x)             ; $82FD55
    incbin !baserom:$17D58..$17D78
    sta.b [$00],y               ; $82FD78
    iny                         ; $82FD7A
    iny                         ; $82FD7B
    sta.b [$00],y               ; $82FD7C
    iny                         ; $82FD7E
    iny                         ; $82FD7F
    sta.b [$00],y               ; $82FD80
    iny                         ; $82FD82
    iny                         ; $82FD83
    sta.b [$00],y               ; $82FD84
    iny                         ; $82FD86
    iny                         ; $82FD87
    sta.b [$00],y               ; $82FD88
    iny                         ; $82FD8A
    iny                         ; $82FD8B
    sta.b [$00],y               ; $82FD8C
    iny                         ; $82FD8E
    iny                         ; $82FD8F
    sta.b [$00],y               ; $82FD90
    iny                         ; $82FD92
    iny                         ; $82FD93
    sep.b #$20                  ; $82FD94
    sta.b [$00],y               ; $82FD96
    iny                         ; $82FD98
    rep.b #$20                  ; $82FD99
    jmp.w $FB7F                 ; $82FD9B
    sta.b [$00],y               ; $82FD9E
    iny                         ; $82FDA0
    iny                         ; $82FDA1
    sta.b [$00],y               ; $82FDA2
    iny                         ; $82FDA4
    iny                         ; $82FDA5
    sta.b [$00],y               ; $82FDA6
    iny                         ; $82FDA8
    iny                         ; $82FDA9
    sta.b [$00],y               ; $82FDAA
    iny                         ; $82FDAC
    iny                         ; $82FDAD
    sta.b [$00],y               ; $82FDAE
    iny                         ; $82FDB0
    iny                         ; $82FDB1
    sta.b [$00],y               ; $82FDB2
    iny                         ; $82FDB4
    iny                         ; $82FDB5
    sta.b [$00],y               ; $82FDB6
    iny                         ; $82FDB8
    iny                         ; $82FDB9
    sta.b [$00],y               ; $82FDBA
    iny                         ; $82FDBC
    iny                         ; $82FDBD
    jmp.w $FB7F                 ; $82FDBE
    ldx.b $04                   ; $82FDC1
    lda.w $0000,x               ; $82FDC3
    inx                         ; $82FDC6
    stx.b $04                   ; $82FDC7
    and.w #$000F                ; $82FDC9
    asl a                       ; $82FDCC
    tax                         ; $82FDCD
    dey                         ; $82FDCE
    dey                         ; $82FDCF
    lda.b [$00],y               ; $82FDD0
    iny                         ; $82FDD2
    iny                         ; $82FDD3
    jmp.w ($FDD7,x)             ; $82FDD4
    incbin !baserom:$17DD7..$17DF7
    sta.b [$00],y               ; $82FDF7
    iny                         ; $82FDF9
    iny                         ; $82FDFA
    sta.b [$00],y               ; $82FDFB
    iny                         ; $82FDFD
    iny                         ; $82FDFE
    sta.b [$00],y               ; $82FDFF
    iny                         ; $82FE01
    iny                         ; $82FE02
    sta.b [$00],y               ; $82FE03
    iny                         ; $82FE05
    iny                         ; $82FE06
    sta.b [$00],y               ; $82FE07
    iny                         ; $82FE09
    iny                         ; $82FE0A
    sta.b [$00],y               ; $82FE0B
    iny                         ; $82FE0D
    iny                         ; $82FE0E
    sta.b [$00],y               ; $82FE0F
    iny                         ; $82FE11
    iny                         ; $82FE12
    sta.b [$00],y               ; $82FE13
    iny                         ; $82FE15
    iny                         ; $82FE16
    sta.b [$00],y               ; $82FE17
    iny                         ; $82FE19
    iny                         ; $82FE1A
    sta.b [$00],y               ; $82FE1B
    iny                         ; $82FE1D
    iny                         ; $82FE1E
    sta.b [$00],y               ; $82FE1F
    iny                         ; $82FE21
    iny                         ; $82FE22
    sta.b [$00],y               ; $82FE23
    iny                         ; $82FE25
    iny                         ; $82FE26
    sta.b [$00],y               ; $82FE27
    iny                         ; $82FE29
    iny                         ; $82FE2A
    sta.b [$00],y               ; $82FE2B
    iny                         ; $82FE2D
    iny                         ; $82FE2E
    sta.b [$00],y               ; $82FE2F
    iny                         ; $82FE31
    iny                         ; $82FE32
    sta.b [$00],y               ; $82FE33
    iny                         ; $82FE35
    iny                         ; $82FE36
    sta.b [$00],y               ; $82FE37
    iny                         ; $82FE39
    iny                         ; $82FE3A
    jmp.w $FB7F                 ; $82FE3B
    ldx.b $04                   ; $82FE3E
    lda.w $0000,x               ; $82FE40
    inx                         ; $82FE43
    stx.b $04                   ; $82FE44
    sty.b $0A                   ; $82FE46
    and.w #$003F                ; $82FE48
    clc                         ; $82FE4B
    adc.w #$0002                ; $82FE4C
    eor.w #$FFFF                ; $82FE4F
    sec                         ; $82FE52
    adc.b $0A                   ; $82FE53
    tay                         ; $82FE55
    lda.b [$00],y               ; $82FE56
    tax                         ; $82FE58
    dey                         ; $82FE59
    lda.b [$00],y               ; $82FE5A
    ldy.b $0A                   ; $82FE5C
    sta.b [$00],y               ; $82FE5E
    iny                         ; $82FE60
    txa                         ; $82FE61
    sta.b [$00],y               ; $82FE62
    iny                         ; $82FE64
    iny                         ; $82FE65
    jmp.w $FB7F                 ; $82FE66
    ldx.b $04                   ; $82FE69
    lda.w $0000,x               ; $82FE6B
    and.w #$FF03                ; $82FE6E
    xba                         ; $82FE71
    sty.b $0A                   ; $82FE72
    clc                         ; $82FE74
    adc.w #$0003                ; $82FE75
    eor.w #$FFFF                ; $82FE78
    sec                         ; $82FE7B
    adc.b $0A                   ; $82FE7C
    sta.b $0A                   ; $82FE7E
    lda.w $0000,x               ; $82FE80
    and.w #$001C                ; $82FE83
    inx                         ; $82FE86
    inx                         ; $82FE87
    stx.b $04                   ; $82FE88
    tax                         ; $82FE8A
    phb                         ; $82FE8B
    lda.l $82FE99,x             ; $82FE8C
    pha                         ; $82FE90
    lda.b $01                   ; $82FE91
    pha                         ; $82FE93
    plb                         ; $82FE94
    plb                         ; $82FE95
    ldx.b $0A                   ; $82FE96
    rts                         ; $82FE98
    incbin !baserom:$17E99..$17EB9
    lda.w $0000,x               ; $82FEB9
    sta.w $0000,y               ; $82FEBC
    iny                         ; $82FEBF
    iny                         ; $82FEC0
    inx                         ; $82FEC1
    inx                         ; $82FEC2
    lda.w $0000,x               ; $82FEC3
    sta.w $0000,y               ; $82FEC6
    iny                         ; $82FEC9
    iny                         ; $82FECA
    inx                         ; $82FECB
    inx                         ; $82FECC
    lda.w $0000,x               ; $82FECD
    sta.w $0000,y               ; $82FED0
    iny                         ; $82FED3
    iny                         ; $82FED4
    inx                         ; $82FED5
    inx                         ; $82FED6
    lda.w $0000,x               ; $82FED7
    sta.w $0000,y               ; $82FEDA
    lda.w $0002,x               ; $82FEDD
    sta.w $0002,y               ; $82FEE0
    iny                         ; $82FEE3
    iny                         ; $82FEE4
    iny                         ; $82FEE5
    iny                         ; $82FEE6
    plb                         ; $82FEE7
    jmp.w $FB7F                 ; $82FEE8
    lda.w $0000,x               ; $82FEEB
    sta.w $0000,y               ; $82FEEE
    iny                         ; $82FEF1
    iny                         ; $82FEF2
    inx                         ; $82FEF3
    inx                         ; $82FEF4
    lda.w $0000,x               ; $82FEF5
    sta.w $0000,y               ; $82FEF8
    iny                         ; $82FEFB
    iny                         ; $82FEFC
    inx                         ; $82FEFD
    inx                         ; $82FEFE
    lda.w $0000,x               ; $82FEFF
    sta.w $0000,y               ; $82FF02
    iny                         ; $82FF05
    iny                         ; $82FF06
    inx                         ; $82FF07
    inx                         ; $82FF08
    lda.w $0000,x               ; $82FF09
    sta.w $0000,y               ; $82FF0C
    lda.w $0002,x               ; $82FF0F
    sta.w $0002,y               ; $82FF12
    lda.w $0003,x               ; $82FF15
    sta.w $0003,y               ; $82FF18
    tya                         ; $82FF1B
    clc                         ; $82FF1C
    adc.w #$0005                ; $82FF1D
    tay                         ; $82FF20
    plb                         ; $82FF21
    jmp.w $FB7F                 ; $82FF22
    ldx.b $04                   ; $82FF25
    lda.w $0000,x               ; $82FF27
    and.w #$FF1F                ; $82FF2A
    xba                         ; $82FF2D
    sty.b $0A                   ; $82FF2E
    clc                         ; $82FF30
    adc.w #$0003                ; $82FF31
    eor.w #$FFFF                ; $82FF34
    sec                         ; $82FF37
    adc.b $0A                   ; $82FF38
    sta.b $0A                   ; $82FF3A
    lda.w $0002,x               ; $82FF3C
    and.w #$00FF                ; $82FF3F
    clc                         ; $82FF42
    adc.w #$0001                ; $82FF43
    inx                         ; $82FF46
    inx                         ; $82FF47
    inx                         ; $82FF48
    stx.b $04                   ; $82FF49
    phb                         ; $82FF4B
    ldx.b $01                   ; $82FF4C
    phx                         ; $82FF4E
    plb                         ; $82FF4F
    plb                         ; $82FF50
    ldx.b $0A                   ; $82FF51
    lsr a                       ; $82FF53
    sta.b $08                   ; $82FF54
    bcc CODE_82FF62             ; $82FF56
    beq CODE_82FF88             ; $82FF58
    lda.w $0000,x               ; $82FF5A
    sta.w $0000,y               ; $82FF5D
    inx                         ; $82FF60
    iny                         ; $82FF61
CODE_82FF62:
    lda.w $0000,x               ; $82FF62
    sta.w $0000,y               ; $82FF65
    inx                         ; $82FF68
    inx                         ; $82FF69
    iny                         ; $82FF6A
    iny                         ; $82FF6B
    lda.w $0000,x               ; $82FF6C
    sta.w $0000,y               ; $82FF6F
    inx                         ; $82FF72
    inx                         ; $82FF73
    iny                         ; $82FF74
    iny                         ; $82FF75
CODE_82FF76:
    lda.w $0000,x               ; $82FF76
    sta.w $0000,y               ; $82FF79
    inx                         ; $82FF7C
    inx                         ; $82FF7D
    iny                         ; $82FF7E
    iny                         ; $82FF7F
    dec.b $08                   ; $82FF80
    bne CODE_82FF76             ; $82FF82
    plb                         ; $82FF84
    jmp.w $FB7F                 ; $82FF85
CODE_82FF88:
    lda.w $0000,x               ; $82FF88
    sta.w $0000,y               ; $82FF8B
    inx                         ; $82FF8E
    inx                         ; $82FF8F
    iny                         ; $82FF90
    iny                         ; $82FF91
    lda.w $0000,x               ; $82FF92
    sta.w $0000,y               ; $82FF95
    inx                         ; $82FF98
    inx                         ; $82FF99
    iny                         ; $82FF9A
    iny                         ; $82FF9B
    sep.b #$20                  ; $82FF9C
    lda.w $0000,x               ; $82FF9E
    sta.w $0000,y               ; $82FFA1
    iny                         ; $82FFA4
    rep.b #$20                  ; $82FFA5
    plb                         ; $82FFA7
    jmp.w $FB7F                 ; $82FFA8
    incbin !baserom:$17FAB..$18000
