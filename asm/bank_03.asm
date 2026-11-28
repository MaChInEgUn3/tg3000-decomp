; bank $83 (file offset $018000)
org $838000

    incbin !baserom:$18000..$18623
nmi_race_wrapper:
    rep.b #$30                  ; $838623
    phb                         ; $838625
    pha                         ; $838626
    phx                         ; $838627
    phy                         ; $838628
    phd                         ; $838629
    phk                         ; $83862A
    plb                         ; $83862B
    lda.w $4210                 ; $83862C
    lda.w #$0000                ; $83862F
    tcd                         ; $838632
    jsl.l sound_frame           ; $838633
    jsr.w pads_read             ; $838637
    inc.b $24                   ; $83863A
    inc.b $BA                   ; $83863C
    pld                         ; $83863E
    ply                         ; $83863F
    plx                         ; $838640
    pla                         ; $838641
    plb                         ; $838642
    rti                         ; $838643
race_upload_queue_drain:
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
    bmi CODE_8386B7             ; $838668
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
CODE_8386B7:
    lda.b $FC                   ; $8386B7
    sta.w $4375                 ; $8386B9
    lsr a                       ; $8386BC
    sta.b $3C                   ; $8386BD
    lda.l $7EF433,x             ; $8386BF
    sec                         ; $8386C3
    sbc.b $FC                   ; $8386C4
    sta.l $7EF433,x             ; $8386C6
    lda.l $7EF533,x             ; $8386CA
    sta.w $4372                 ; $8386CE
    clc                         ; $8386D1
    adc.b $FC                   ; $8386D2
    sta.l $7EF533,x             ; $8386D4
    lda.l $7EF633,x             ; $8386D8
    sta.w $2116                 ; $8386DC
    clc                         ; $8386DF
    adc.b $3C                   ; $8386E0
    sta.l $7EF633,x             ; $8386E2
    sep.b #$20                  ; $8386E6
    lda.l $7EF733,x             ; $8386E8
    sta.w $4374                 ; $8386EC
    lda.b #$80                  ; $8386EF
    sta.w $420B                 ; $8386F1
    rep.b #$20                  ; $8386F4
    stz.b $FC                   ; $8386F6
    stx.b $FE                   ; $8386F8
    rts                         ; $8386FA
pads_read_long:
    jsr.w pads_read             ; $8386FB
    rtl                         ; $8386FE
pads_read:
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
    jsr.w pads_read_pair        ; $838732
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
    jsr.w pads_read_pair        ; $838756
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
    lda.w #$0001                ; $83878F
    sta.b $10                   ; $838792
    sta.b $12                   ; $838794
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
pads_read_pair:
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
vs_view_scrolls:
    ldx.b $E2                   ; $83886A
    lda.w $0026                 ; $83886C
    bit.w #$0010                ; $83886F
    bne CODE_8388E0             ; $838872
    ldy.w #$0000                ; $838874
    lda.w $01CC                 ; $838877
    bpl CODE_83887D             ; $83887A
    dey                         ; $83887C
CODE_83887D:
    clc                         ; $83887D
    adc.w $00D4                 ; $83887E
    sta.w $00D4                 ; $838881
    sep.b #$20                  ; $838884
    tya                         ; $838886
    adc.w $00D6                 ; $838887
    sta.w $00D6                 ; $83888A
    rep.b #$20                  ; $83888D
    ldy.w #$0000                ; $83888F
    lda.w $01D4                 ; $838892
    bpl CODE_838898             ; $838895
    dey                         ; $838897
CODE_838898:
    clc                         ; $838898
    adc.w $00D7                 ; $838899
    sta.w $00D7                 ; $83889C
    sep.b #$20                  ; $83889F
    tya                         ; $8388A1
    adc.w $00D9                 ; $8388A2
    sta.w $00D9                 ; $8388A5
    rep.b #$20                  ; $8388A8
    ldy.w #$0000                ; $8388AA
    lda.w $01D0                 ; $8388AD
    bpl CODE_8388B3             ; $8388B0
    dey                         ; $8388B2
CODE_8388B3:
    clc                         ; $8388B3
    adc.w $00DA                 ; $8388B4
    sta.w $00DA                 ; $8388B7
    sep.b #$20                  ; $8388BA
    tya                         ; $8388BC
    adc.w $00DC                 ; $8388BD
    sta.w $00DC                 ; $8388C0
    rep.b #$20                  ; $8388C3
    ldy.w #$0000                ; $8388C5
    lda.w $01D8                 ; $8388C8
    bpl CODE_8388CE             ; $8388CB
    dey                         ; $8388CD
CODE_8388CE:
    clc                         ; $8388CE
    adc.w $00DD                 ; $8388CF
    sta.w $00DD                 ; $8388D2
    sep.b #$20                  ; $8388D5
    tya                         ; $8388D7
    adc.w $00DF                 ; $8388D8
    sta.w $00DF                 ; $8388DB
    rep.b #$20                  ; $8388DE
CODE_8388E0:
    lda.w $00D5                 ; $8388E0
    clc                         ; $8388E3
    adc.w #$003C                ; $8388E4
    sta.w $1536,x               ; $8388E7
    lda.w $00D8                 ; $8388EA
    clc                         ; $8388ED
    adc.w #$003C                ; $8388EE
    sta.w $1546,x               ; $8388F1
    lda.w $00DB                 ; $8388F4
    sec                         ; $8388F7
    sbc.w #$004C                ; $8388F8
    sta.w $1556,x               ; $8388FB
    lda.w $00DE                 ; $8388FE
    sec                         ; $838901
    sbc.w #$004C                ; $838902
    sta.w $1566,x               ; $838905
    rts                         ; $838908
mode1_view_scrolls:
    ldx.b $E2                   ; $838909
    lda.w $0026                 ; $83890B
    bit.w #$0010                ; $83890E
    bne CODE_838949             ; $838911
    ldy.w #$0000                ; $838913
    lda.w $01CC                 ; $838916
    bpl CODE_83891C             ; $838919
    dey                         ; $83891B
CODE_83891C:
    clc                         ; $83891C
    adc.w $00D4                 ; $83891D
    sta.w $00D4                 ; $838920
    sep.b #$20                  ; $838923
    tya                         ; $838925
    adc.w $00D6                 ; $838926
    sta.w $00D6                 ; $838929
    rep.b #$20                  ; $83892C
    ldy.w #$0000                ; $83892E
    lda.w $01D0                 ; $838931
    bpl CODE_838937             ; $838934
    dey                         ; $838936
CODE_838937:
    clc                         ; $838937
    adc.w $00D7                 ; $838938
    sta.w $00D7                 ; $83893B
    sep.b #$20                  ; $83893E
    tya                         ; $838940
    adc.w $00D9                 ; $838941
    sta.w $00D9                 ; $838944
    rep.b #$20                  ; $838947
CODE_838949:
    lda.w $00D5                 ; $838949
    clc                         ; $83894C
    adc.w #$003C                ; $83894D
    sta.w $1536,x               ; $838950
    sta.w $1556,x               ; $838953
    lda.w $00D8                 ; $838956
    clc                         ; $838959
    adc.w #$003C                ; $83895A
    sta.w $1546,x               ; $83895D
    sta.w $1566,x               ; $838960
    lda.w $1F32                 ; $838963
    bpl CODE_838988             ; $838966
    incbin !baserom:$18968..$18988
CODE_838988:
    rts                         ; $838988
func_838989:
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
    incbin !baserom:$189D1..$18A26
irq_vs_band:
    rep.b #$10                  ; $838A26
    sep.b #$20                  ; $838A28
    phb                         ; $838A2A
    pha                         ; $838A2B
    phx                         ; $838A2C
    phk                         ; $838A2D
    plb                         ; $838A2E
    ldx.w $00BC                 ; $838A2F
    lda.b #$FF                  ; $838A32
    sta.w $4207                 ; $838A34
    lda.b #$01                  ; $838A37
    sta.w $4325                 ; $838A39
    sta.w $4375                 ; $838A3C
    lda.b #$08                  ; $838A3F
    sta.w $4315                 ; $838A41
    sta.w $4335                 ; $838A44
    lda.b #$21                  ; $838A47
    sta.w $2121                 ; $838A49
    lda.w $4211                 ; $838A4C
    lda.w $0596,x               ; $838A4F
    wai                         ; $838A52
    sta.w $420B                 ; $838A53
    inx                         ; $838A56
    lda.w $4211                 ; $838A57
    lda.w $0486,x               ; $838A5A
    beq CODE_838A6E             ; $838A5D
    sta.w $4209                 ; $838A5F
    lda.b #$63                  ; $838A62
    sta.w $4207                 ; $838A64
    stx.w $00BC                 ; $838A67
    plx                         ; $838A6A
    pla                         ; $838A6B
    plb                         ; $838A6C
    rti                         ; $838A6D
CODE_838A6E:
    ldx.w #$8A84                ; $838A6E
    stx.w $0005                 ; $838A71
    ldx.w #$006C                ; $838A74
    stx.w $4209                 ; $838A77
    ldx.w #$0078                ; $838A7A
    stx.w $4207                 ; $838A7D
    plx                         ; $838A80
    pla                         ; $838A81
    plb                         ; $838A82
    rti                         ; $838A83
irq_vs_split:
    rep.b #$30                  ; $838A84
    phb                         ; $838A86
    pha                         ; $838A87
    phx                         ; $838A88
    phy                         ; $838A89
    phd                         ; $838A8A
    phk                         ; $838A8B
    plb                         ; $838A8C
    lda.w $4210                 ; $838A8D
    lda.w #$0000                ; $838A90
    tcd                         ; $838A93
    lda.w #$010A                ; $838A94
    sta.w $4207                 ; $838A97
    sep.b #$20                  ; $838A9A
    lda.b #$80                  ; $838A9C
    wai                         ; $838A9E
    sta.w $2100                 ; $838A9F
    lda.w $4211                 ; $838AA2
    rep.b #$20                  ; $838AA5
    stz.w $2102                 ; $838AA7
    ldx.w #$19B6                ; $838AAA
    lda.w $00B6                 ; $838AAD
    bne CODE_838AB5             ; $838AB0
    ldx.w #$1BD6                ; $838AB2
CODE_838AB5:
    stx.w $4372                 ; $838AB5
    lda.w #$0402                ; $838AB8
    sta.w $4370                 ; $838ABB
    lda.w #$0220                ; $838ABE
    sta.w $4375                 ; $838AC1
    sep.b #$20                  ; $838AC4
    lda.b #$81                  ; $838AC6
    sta.w $4374                 ; $838AC8
    lda.b #$80                  ; $838ACB
    sta.w $420B                 ; $838ACD
    rep.b #$20                  ; $838AD0
    stz.w $2102                 ; $838AD2
    lda.w $0038                 ; $838AD5
    sep.b #$30                  ; $838AD8
    ldx.b #$21                  ; $838ADA
    stx.w $2121                 ; $838ADC
    sta.w $2122                 ; $838ADF
    xba                         ; $838AE2
    sta.w $2122                 ; $838AE3
    xba                         ; $838AE6
    ldx.b #$31                  ; $838AE7
    stx.w $2121                 ; $838AE9
    sta.w $2122                 ; $838AEC
    xba                         ; $838AEF
    sta.w $2122                 ; $838AF0
    rep.b #$30                  ; $838AF3
    lda.w #$0500                ; $838AF5
    sta.w $4370                 ; $838AF8
    lda.w $00CA                 ; $838AFB
    sta.w $4312                 ; $838AFE
    lda.w $00CE                 ; $838B01
    sta.w $4332                 ; $838B04
    ldx.w #$0482                ; $838B07
    lda.w $00B6                 ; $838B0A
    bne CODE_838B12             ; $838B0D
    ldx.w #$0484                ; $838B0F
CODE_838B12:
    stx.w $4372                 ; $838B12
    sep.b #$20                  ; $838B15
    lda.b #$09                  ; $838B17
    sta.w $2105                 ; $838B19
    lda.b #$01                  ; $838B1C
    sta.w $4325                 ; $838B1E
    sta.w $4375                 ; $838B21
    lda.b #$08                  ; $838B24
    sta.w $4315                 ; $838B26
    sta.w $4335                 ; $838B29
    lda.b #$21                  ; $838B2C
    sta.w $2121                 ; $838B2E
    lda.b $32                   ; $838B31
    sta.w $420B                 ; $838B33
    lda.b #$21                  ; $838B36
    sta.w $2121                 ; $838B38
    lda.b $38                   ; $838B3B
    sta.w $2122                 ; $838B3D
    lda.b $39                   ; $838B40
    sta.w $2122                 ; $838B42
    lda.b #$08                  ; $838B45
    sta.w $2101                 ; $838B47
    rep.b #$20                  ; $838B4A
    lda.w #$0073                ; $838B4C
    sta.w $4209                 ; $838B4F
    lda.w $0036                 ; $838B52
    sta.w $0005                 ; $838B55
    sep.b #$20                  ; $838B58
    lda.w $014B                 ; $838B5A
    wai                         ; $838B5D
    sta.w $2100                 ; $838B5E
    lda.w $4211                 ; $838B61
    lda.b #$07                  ; $838B64
    sta.w $2121                 ; $838B66
    lda.b $A2                   ; $838B69
    sta.w $2122                 ; $838B6B
    lda.b $A3                   ; $838B6E
    sta.w $2122                 ; $838B70
    rep.b #$20                  ; $838B73
    lda.w $4342                 ; $838B75
    sta.l $7FFFFE               ; $838B78
    lda.w #$0063                ; $838B7C
    sta.w $4207                 ; $838B7F
    lda.w $0034                 ; $838B82
    sta.w $4209                 ; $838B85
    pld                         ; $838B88
    ply                         ; $838B89
    plx                         ; $838B8A
    pla                         ; $838B8B
    plb                         ; $838B8C
    rti                         ; $838B8D
    rep.b #$20                  ; $838B8E
    phb                         ; $838B90
    pha                         ; $838B91
    phx                         ; $838B92
    phy                         ; $838B93
    phd                         ; $838B94
    phk                         ; $838B95
    plb                         ; $838B96
    lda.w $4210                 ; $838B97
    lda.w #$0000                ; $838B9A
    tcd                         ; $838B9D
    pld                         ; $838B9E
    ply                         ; $838B9F
    plx                         ; $838BA0
    pla                         ; $838BA1
    plb                         ; $838BA2
    rti                         ; $838BA3
    incbin !baserom:$18BA4..$18BE3
irq_vs_band2:
    rep.b #$10                  ; $838BE3
    sep.b #$20                  ; $838BE5
    phb                         ; $838BE7
    pha                         ; $838BE8
    phx                         ; $838BE9
    phk                         ; $838BEA
    plb                         ; $838BEB
    ldx.w $00C2                 ; $838BEC
    lda.b #$FF                  ; $838BEF
    sta.w $4207                 ; $838BF1
    lda.b #$01                  ; $838BF4
    sta.w $4325                 ; $838BF6
    sta.w $4375                 ; $838BF9
    lda.b #$08                  ; $838BFC
    sta.w $4315                 ; $838BFE
    sta.w $4335                 ; $838C01
    lda.b #$21                  ; $838C04
    sta.w $2121                 ; $838C06
    lda.w $4211                 ; $838C09
    lda.w $061E,x               ; $838C0C
    wai                         ; $838C0F
    sta.w $420B                 ; $838C10
    inx                         ; $838C13
    lda.w $4211                 ; $838C14
    lda.w $050E,x               ; $838C17
    beq CODE_838C2B             ; $838C1A
    sta.w $4209                 ; $838C1C
    lda.b #$63                  ; $838C1F
    sta.w $4207                 ; $838C21
    stx.w $00C2                 ; $838C24
    plx                         ; $838C27
    pla                         ; $838C28
    plb                         ; $838C29
    rti                         ; $838C2A
CODE_838C2B:
    ldx.w #$0000                ; $838C2B
    stx.w $4209                 ; $838C2E
    plx                         ; $838C31
    pla                         ; $838C32
    plb                         ; $838C33
    rti                         ; $838C34
    incbin !baserom:$18C35..$196CD
pads_read_long_2:
    jsr.w pads_read             ; $8396CD
    rtl                         ; $8396D0
race_upload_queue_drain_long:
    jsr.w race_upload_queue_drain; $8396D1
    rtl                         ; $8396D4
func_838989_long:
    jsr.w func_838989           ; $8396D5
    rtl                         ; $8396D8
    jsr.w mode1_view_scrolls    ; $8396D9
    rtl                         ; $8396DC
    incbin !baserom:$196DD..$19721
nmi_vs_prologue:
    rep.b #$20                  ; $839721
    phb                         ; $839723
    pha                         ; $839724
    phx                         ; $839725
    phy                         ; $839726
    phd                         ; $839727
    phk                         ; $839728
    plb                         ; $839729
    lda.w $4210                 ; $83972A
    lda.w #$0000                ; $83972D
    tcd                         ; $839730
    jmp.w ($0008)               ; $839731
nmi_vs_tail:
    lda.w $4210                 ; $839734
    cli                         ; $839737
    lda.w $0146                 ; $839738
    beq CODE_839758             ; $83973B
    clc                         ; $83973D
    adc.w $014A                 ; $83973E
    sta.w $014A                 ; $839741
    sec                         ; $839744
    sbc.w $0148                 ; $839745
    beq CODE_83974F             ; $839748
    eor.w $0146                 ; $83974A
    bmi CODE_839758             ; $83974D
CODE_83974F:
    lda.w $0148                 ; $83974F
    sta.w $014A                 ; $839752
    stz.w $0146                 ; $839755
CODE_839758:
    sep.b #$20                  ; $839758
    lda.w $014B                 ; $83975A
    bne CODE_839761             ; $83975D
    ora.b #$80                  ; $83975F
CODE_839761:
    sta.w $2100                 ; $839761
    rep.b #$20                  ; $839764
    jsl.l sound_frame           ; $839766
    jsr.w pads_read             ; $83976A
    inc.b $24                   ; $83976D
    inc.b $BA                   ; $83976F
    pld                         ; $839771
    ply                         ; $839772
    plx                         ; $839773
    pla                         ; $839774
    plb                         ; $839775
    rti                         ; $839776
nmi_vs_full:
    lda.w #$1000                ; $839777
    sta.b $FC                   ; $83977A
    lda.w $00B8                 ; $83977C
    bne CODE_839784             ; $83977F
    jmp.w $983B                 ; $839781
CODE_839784:
    stz.w $00B8                 ; $839784
    lda.w $00D0                 ; $839787
    sta.w $4352                 ; $83978A
    lda.w $00D2                 ; $83978D
    sta.w $4362                 ; $839790
    lda.w $00B4                 ; $839793
    eor.w #$0001                ; $839796
    sta.w $00B4                 ; $839799
    sta.w $00B6                 ; $83979C
    beq CODE_8397A4             ; $83979F
    jmp.w $97A4                 ; $8397A1
CODE_8397A4:
    lda.w $01CE                 ; $8397A4
    sta.w $01CC                 ; $8397A7
    lda.w $01D2                 ; $8397AA
    sta.w $01D0                 ; $8397AD
    lda.w $01D6                 ; $8397B0
    sta.w $01D4                 ; $8397B3
    lda.w $01DA                 ; $8397B6
    sta.w $01D8                 ; $8397B9
    lda.w $016C                 ; $8397BC
    sta.b $C8                   ; $8397BF
    lda.w $0170                 ; $8397C1
    sta.b $CC                   ; $8397C4
    lda.w $016E                 ; $8397C6
    sta.b $CA                   ; $8397C9
    lda.w $0172                 ; $8397CB
    sta.b $CE                   ; $8397CE
    lda.w $00C0                 ; $8397D0
    sta.w $00BE                 ; $8397D3
    lda.w $00C6                 ; $8397D6
    sta.w $00C4                 ; $8397D9
    lda.w $00E0                 ; $8397DC
    sta.w $00E2                 ; $8397DF
    lda.w $0174                 ; $8397E2
    sta.b $EC                   ; $8397E5
    lda.w $0178                 ; $8397E7
    sta.b $F0                   ; $8397EA
    lda.w $0176                 ; $8397EC
    sta.b $EE                   ; $8397EF
    lda.w $017A                 ; $8397F1
    sta.b $F2                   ; $8397F4
    lda.w $017C                 ; $8397F6
    sta.b $F4                   ; $8397F9
    lda.w $0180                 ; $8397FB
    sta.b $F8                   ; $8397FE
    lda.w $017E                 ; $839800
    sta.b $F6                   ; $839803
    lda.w $0182                 ; $839805
    sta.b $FA                   ; $839808
    lda.w $1E02                 ; $83980A
    beq CODE_83983B             ; $83980D
    incbin !baserom:$1980F..$1983B
CODE_83983B:
    ldx.w #$1576                ; $83983B
    lda.w $00B4                 ; $83983E
    bne CODE_839846             ; $839841
    ldx.w #$1796                ; $839843
CODE_839846:
    stx.w $4372                 ; $839846
    lda.w #$0400                ; $839849
    sta.w $4370                 ; $83984C
    lda.w #$0220                ; $83984F
    sta.w $4375                 ; $839852
    stz.w $2102                 ; $839855
    sep.b #$20                  ; $839858
    lda.b #$81                  ; $83985A
    sta.w $4374                 ; $83985C
    lda.b #$80                  ; $83985F
    sta.w $420B                 ; $839861
    stz.w $2101                 ; $839864
    rep.b #$20                  ; $839867
    stz.w $2102                 ; $839869
    lda.l $7E5395               ; $83986C
    sec                         ; $839870
    sbc.w #$8007                ; $839871
    tax                         ; $839874
    lda.l $908000,x             ; $839875
    sta.w $0038                 ; $839879
    sep.b #$30                  ; $83987C
    ldx.b #$21                  ; $83987E
    stx.w $2121                 ; $839880
    sta.w $2122                 ; $839883
    xba                         ; $839886
    sta.w $2122                 ; $839887
    xba                         ; $83988A
    ldx.b #$31                  ; $83988B
    stx.w $2121                 ; $83988D
    sta.w $2122                 ; $839890
    xba                         ; $839893
    sta.w $2122                 ; $839894
    rep.b #$30                  ; $839897
    jsr.w race_upload_queue_drain; $839899
    lda.b $FC                   ; $83989C
    cmp.w #$0800                ; $83989E
    bmi CODE_8398A6             ; $8398A1
    jsr.w vs_hud_draw           ; $8398A3
CODE_8398A6:
    jsr.w vs_view_scrolls       ; $8398A6
    lda.w #$0080                ; $8398A9
    sta.w $210D                 ; $8398AC
    lda.w #$0001                ; $8398AF
    sta.w $210D                 ; $8398B2
    lda.w #$0053                ; $8398B5
    sta.w $210E                 ; $8398B8
    lda.w #$0000                ; $8398BB
    sta.w $210E                 ; $8398BE
    rep.b #$20                  ; $8398C1
    sed                         ; $8398C3
    dec.b $60                   ; $8398C4
    bne CODE_8398E1             ; $8398C6
    lda.w #$0003                ; $8398C8
    sta.b $60                   ; $8398CB
    lda.b $5A                   ; $8398CD
    clc                         ; $8398CF
    adc.w #$0001                ; $8398D0
    sta.b $5A                   ; $8398D3
    sec                         ; $8398D5
    sbc.w #$6000                ; $8398D6
    bcc CODE_8398F3             ; $8398D9
    sta.b $5A                   ; $8398DB
    inc.b $5C                   ; $8398DD
    bra CODE_8398F3             ; $8398DF
CODE_8398E1:
    lda.b $5A                   ; $8398E1
    clc                         ; $8398E3
    adc.w #$0002                ; $8398E4
    sta.b $5A                   ; $8398E7
    sec                         ; $8398E9
    sbc.w #$6000                ; $8398EA
    bcc CODE_8398F3             ; $8398ED
    incbin !baserom:$198EF..$198F3
CODE_8398F3:
    dec.b $62                   ; $8398F3
    bne CODE_839910             ; $8398F5
    lda.w #$0003                ; $8398F7
    sta.b $62                   ; $8398FA
    lda.b $5D                   ; $8398FC
    clc                         ; $8398FE
    adc.w #$0001                ; $8398FF
    sta.b $5D                   ; $839902
    sec                         ; $839904
    sbc.w #$6000                ; $839905
    bcc CODE_839922             ; $839908
    sta.b $5D                   ; $83990A
    inc.b $5F                   ; $83990C
    bra CODE_839922             ; $83990E
CODE_839910:
    lda.b $5D                   ; $839910
    clc                         ; $839912
    adc.w #$0002                ; $839913
    sta.b $5D                   ; $839916
    sec                         ; $839918
    sbc.w #$6000                ; $839919
    bcc CODE_839922             ; $83991C
    incbin !baserom:$1991E..$19922
CODE_839922:
    cld                         ; $839922
    lda.w #$0D43                ; $839923
    sta.w $4350                 ; $839926
    lda.w #$0F43                ; $839929
    sta.w $4360                 ; $83992C
    lda.w #$2644                ; $83992F
    sta.w $4340                 ; $839932
    lda.w #$A411                ; $839935
    sta.w $4342                 ; $839938
    lda.w #$2200                ; $83993B
    sta.w $4310                 ; $83993E
    lda.w $00C8                 ; $839941
    sta.w $4312                 ; $839944
    stz.w $4315                 ; $839947
    lda.w #$2108                ; $83994A
    sta.w $4320                 ; $83994D
    lda.w #$8D35                ; $839950
    sta.w $4322                 ; $839953
    stz.w $4325                 ; $839956
    lda.w #$2200                ; $839959
    sta.w $4330                 ; $83995C
    lda.w $00CC                 ; $83995F
    sta.w $4332                 ; $839962
    stz.w $4335                 ; $839965
    lda.w #$0500                ; $839968
    sta.w $4370                 ; $83996B
    ldx.w #$047E                ; $83996E
    lda.w $00B4                 ; $839971
    bne CODE_839979             ; $839974
    ldx.w #$0480                ; $839976
CODE_839979:
    stx.w $4372                 ; $839979
    stz.w $4375                 ; $83997C
    sep.b #$20                  ; $83997F
    lda.b #$83                  ; $839981
    sta.w $4354                 ; $839983
    sta.w $4364                 ; $839986
    sta.w $4344                 ; $839989
    sta.w $4357                 ; $83998C
    sta.w $4367                 ; $83998F
    sta.w $4347                 ; $839992
    lda.b #$7E                  ; $839995
    sta.w $4314                 ; $839997
    sta.w $4334                 ; $83999A
    sta.w $4374                 ; $83999D
    lda.b #$83                  ; $8399A0
    sta.w $4324                 ; $8399A2
    lda.b #$60                  ; $8399A5
    sta.w $420C                 ; $8399A7
    rep.b #$20                  ; $8399AA
    ldx.w $00BE                 ; $8399AC
    stx.w $00BC                 ; $8399AF
    lda.w $0486,x               ; $8399B2
    and.w #$00FF                ; $8399B5
    beq CODE_839A2F             ; $8399B8
    sec                         ; $8399BA
    sbc.w #$0004                ; $8399BB
    bit.w #$0080                ; $8399BE
    beq CODE_8399C6             ; $8399C1
    incbin !baserom:$199C3..$199C6
CODE_8399C6:
    sta.b $2E                   ; $8399C6
    sep.b #$20                  ; $8399C8
    lda.b #$09                  ; $8399CA
    sta.w $2105                 ; $8399CC
    rep.b #$20                  ; $8399CF
    lda.b $2E                   ; $8399D1
    beq CODE_8399D7             ; $8399D3
    bpl CODE_839A2C             ; $8399D5
CODE_8399D7:
    ldx.w $00BC                 ; $8399D7
    lda.w #$0001                ; $8399DA
    sta.w $4325                 ; $8399DD
    sta.w $4375                 ; $8399E0
    lda.w #$0008                ; $8399E3
    sta.w $4315                 ; $8399E6
    sta.w $4335                 ; $8399E9
    sep.b #$20                  ; $8399EC
    lda.b #$21                  ; $8399EE
    sta.w $2121                 ; $8399F0
    lda.w $0596,x               ; $8399F3
    sta.w $420B                 ; $8399F6
    rep.b #$20                  ; $8399F9
    inx                         ; $8399FB
    stx.w $00BC                 ; $8399FC
    lda.w $0486,x               ; $8399FF
    and.w #$00FF                ; $839A02
    db $F0,$11 ; beq            ; $839A05
    sta.w $4209                 ; $839A07
    lda.w #$8A26                ; $839A0A
    sta.w $0005                 ; $839A0D
    lda.w #$006B                ; $839A10
    sta.w $4207                 ; $839A13
    bra CODE_839A6C             ; $839A16
    incbin !baserom:$19A18..$19A2C
CODE_839A2C:
    cmp.w #$0068                ; $839A2C
CODE_839A2F:
    bpl CODE_839A51             ; $839A2F
    lda.w #$8A26                ; $839A31
    sta.w $0005                 ; $839A34
    lda.b $2E                   ; $839A37
    clc                         ; $839A39
    adc.w #$0004                ; $839A3A
    sta.w $4209                 ; $839A3D
    lda.w #$006B                ; $839A40
    sta.w $4207                 ; $839A43
    sep.b #$20                  ; $839A46
    lda.b #$09                  ; $839A48
    sta.w $2105                 ; $839A4A
    rep.b #$20                  ; $839A4D
    bra CODE_839A6C             ; $839A4F
CODE_839A51:
    lda.w #$8A84                ; $839A51
    sta.w $0005                 ; $839A54
    lda.w #$006C                ; $839A57
    sta.w $4209                 ; $839A5A
    lda.w #$0078                ; $839A5D
    sta.w $4207                 ; $839A60
    sep.b #$20                  ; $839A63
    lda.b #$29                  ; $839A65
    sta.w $2105                 ; $839A67
    rep.b #$20                  ; $839A6A
CODE_839A6C:
    ldx.w $00C4                 ; $839A6C
    stx.w $00C2                 ; $839A6F
    lda.w $050E,x               ; $839A72
    and.w #$00FF                ; $839A75
    beq CODE_839AD7             ; $839A78
    sec                         ; $839A7A
    sbc.w #$0074                ; $839A7B
    bit.w #$0080                ; $839A7E
    beq CODE_839A86             ; $839A81
    incbin !baserom:$19A83..$19A86
CODE_839A86:
    sta.b $30                   ; $839A86
    lda.b $30                   ; $839A88
    beq CODE_839A8E             ; $839A8A
    bpl CODE_839ABA             ; $839A8C
CODE_839A8E:
    ldx.w $00C2                 ; $839A8E
    sep.b #$20                  ; $839A91
    lda.w $061E,x               ; $839A93
    sta.w $0032                 ; $839A96
    rep.b #$20                  ; $839A99
    inx                         ; $839A9B
    stx.w $00C2                 ; $839A9C
    lda.w $050E,x               ; $839A9F
    and.w #$00FF                ; $839AA2
    db $F0,$0B ; beq            ; $839AA5
    sta.w $0034                 ; $839AA7
    lda.w #$8BE3                ; $839AAA
    sta.w $0036                 ; $839AAD
    bra CODE_839AE0             ; $839AB0
    incbin !baserom:$19AB2..$19ABA
CODE_839ABA:
    cmp.w #$0068                ; $839ABA
    bpl CODE_839AD7             ; $839ABD
    clc                         ; $839ABF
    adc.w #$0074                ; $839AC0
    sta.w $0034                 ; $839AC3
    sep.b #$20                  ; $839AC6
    lda.b #$00                  ; $839AC8
    stz.w $0032                 ; $839ACA
    rep.b #$20                  ; $839ACD
    lda.w #$8BE3                ; $839ACF
    sta.w $0036                 ; $839AD2
    bra CODE_839AE0             ; $839AD5
CODE_839AD7:
    stz.w $0032                 ; $839AD7
    lda.w #$0020                ; $839ADA
    sta.w $0034                 ; $839ADD
CODE_839AE0:
    bit.w $047C                 ; $839AE0
    bmi CODE_839B46             ; $839AE3
    lda.w $0024                 ; $839AE5
    and.w #$0003                ; $839AE8
    asl a                       ; $839AEB
    tax                         ; $839AEC
    jmp.w ($9AF0,x)             ; $839AED
    incbin !baserom:$19AF0..$19AF8
    lda.l $7E4E2D               ; $839AF8
    tax                         ; $839AFC
    lda.l $7E4E35               ; $839AFD
    and.w #$00F8                ; $839B01
    ora.w #$0006                ; $839B04
    bra CODE_839B37             ; $839B07
    lda.l $7E4E2F               ; $839B09
    tax                         ; $839B0D
    lda.l $7E4E37               ; $839B0E
    and.w #$00F8                ; $839B12
    ora.w #$0004                ; $839B15
    bra CODE_839B37             ; $839B18
    lda.l $7E4E31               ; $839B1A
    tax                         ; $839B1E
    lda.l $7E4E39               ; $839B1F
    and.w #$00F8                ; $839B23
    ora.w #$0002                ; $839B26
    bra CODE_839B37             ; $839B29
    lda.l $7E4E33               ; $839B2B
    tax                         ; $839B2F
    lda.l $7E4E3B               ; $839B30
    and.w #$00F8                ; $839B34
CODE_839B37:
    sta.w $0460                 ; $839B37
    stx.w $0461                 ; $839B3A
    sep.b #$20                  ; $839B3D
    lda.b #$20                  ; $839B3F
    tsb.w $045B                 ; $839B41
    rep.b #$20                  ; $839B44
CODE_839B46:
    jmp.w nmi_vs_tail           ; $839B46
vs_hud_draw:
    lda.w #$7C21                ; $839B49
    sta.w $2116                 ; $839B4C
    ldy.w #$0000                ; $839B4F
    lda.b $3E                   ; $839B52
    and.w #$FFFE                ; $839B54
    beq CODE_839B66             ; $839B57
    tax                         ; $839B59
CODE_839B5A:
    lda.w $9F2E,y               ; $839B5A
    sta.w $2118                 ; $839B5D
    iny                         ; $839B60
    iny                         ; $839B61
    dex                         ; $839B62
    dex                         ; $839B63
    bne CODE_839B5A             ; $839B64
CODE_839B66:
    lda.b $3E                   ; $839B66
    bit.w #$0001                ; $839B68
    beq CODE_839B75             ; $839B6B
    lda.w $9F3A,y               ; $839B6D
    sta.w $2118                 ; $839B70
    iny                         ; $839B73
    iny                         ; $839B74
CODE_839B75:
    cpy.w #$000C                ; $839B75
    beq CODE_839B87             ; $839B78
CODE_839B7A:
    lda.w $9F46,y               ; $839B7A
    sta.w $2118                 ; $839B7D
    iny                         ; $839B80
    iny                         ; $839B81
    cpy.w #$000C                ; $839B82
    bne CODE_839B7A             ; $839B85
CODE_839B87:
    lda.w #$7D81                ; $839B87
    sta.w $2116                 ; $839B8A
    lda.b $42                   ; $839B8D
    sta.w $2118                 ; $839B8F
    lda.b $44                   ; $839B92
    sta.w $2118                 ; $839B94
    lda.b $46                   ; $839B97
    sta.w $2118                 ; $839B99
    lda.w #$7C28                ; $839B9C
    sta.w $2116                 ; $839B9F
    ldy.w #$0000                ; $839BA2
    lda.w $024E                 ; $839BA5
    and.w #$00FF                ; $839BA8
    tax                         ; $839BAB
    beq CODE_839BC2             ; $839BAC
    dex                         ; $839BAE
    beq CODE_839BBB             ; $839BAF
    lda.w $9F68                 ; $839BB1
CODE_839BB4:
    sta.w $2118                 ; $839BB4
    iny                         ; $839BB7
    dex                         ; $839BB8
    bne CODE_839BB4             ; $839BB9
CODE_839BBB:
    lda.w $9F68                 ; $839BBB
    sta.w $2118                 ; $839BBE
    iny                         ; $839BC1
CODE_839BC2:
    lda.w $9F52                 ; $839BC2
CODE_839BC5:
    sta.w $2118                 ; $839BC5
    iny                         ; $839BC8
    cpy.w #$0006                ; $839BC9
    bne CODE_839BC5             ; $839BCC
    ldx.b $A4                   ; $839BCE
    stx.w $2116                 ; $839BD0
    lda.w $2139                 ; $839BD3
    lda.w $2139                 ; $839BD6
    ora.w #$6000                ; $839BD9
    stx.w $2116                 ; $839BDC
    sta.w $2118                 ; $839BDF
    sep.b #$20                  ; $839BE2
    ldx.w #$4E61                ; $839BE4
    stx.w $4372                 ; $839BE7
    lda.b #$7E                  ; $839BEA
    sta.w $4374                 ; $839BEC
    lda.l $7E4E4D               ; $839BEF
    clc                         ; $839BF3
    adc.l $7E4E4E               ; $839BF4
    sta.l $7E4E4E               ; $839BF8
    bpl CODE_839C09             ; $839BFC
    ldx.w #$A3DD                ; $839BFE
    stx.w $4372                 ; $839C01
    lda.b #$83                  ; $839C04
    sta.w $4374                 ; $839C06
CODE_839C09:
    ldx.w #$7CA1                ; $839C09
    stx.w $2116                 ; $839C0C
    ldx.w #$1801                ; $839C0F
    stx.w $4370                 ; $839C12
    ldx.w #$001A                ; $839C15
    stx.w $4375                 ; $839C18
    lda.b #$80                  ; $839C1B
    sta.w $420B                 ; $839C1D
    ldx.w #$001A                ; $839C20
    stx.w $4375                 ; $839C23
    ldx.w #$7CC1                ; $839C26
    stx.w $2116                 ; $839C29
    sta.w $420B                 ; $839C2C
    rep.b #$20                  ; $839C2F
    lda.w #$7C32                ; $839C31
    sta.w $2116                 ; $839C34
    ldy.w #$0000                ; $839C37
    lda.b $40                   ; $839C3A
    and.w #$FFFE                ; $839C3C
    beq CODE_839C4E             ; $839C3F
    tax                         ; $839C41
CODE_839C42:
    lda.w $9F2E,y               ; $839C42
    sta.w $2118                 ; $839C45
    iny                         ; $839C48
    iny                         ; $839C49
    dex                         ; $839C4A
    dex                         ; $839C4B
    bne CODE_839C42             ; $839C4C
CODE_839C4E:
    lda.b $40                   ; $839C4E
    bit.w #$0001                ; $839C50
    beq CODE_839C5D             ; $839C53
    lda.w $9F3A,y               ; $839C55
    sta.w $2118                 ; $839C58
    iny                         ; $839C5B
    iny                         ; $839C5C
CODE_839C5D:
    cpy.w #$000C                ; $839C5D
    beq CODE_839C6F             ; $839C60
CODE_839C62:
    lda.w $9F46,y               ; $839C62
    sta.w $2118                 ; $839C65
    iny                         ; $839C68
    iny                         ; $839C69
    cpy.w #$000C                ; $839C6A
    bne CODE_839C62             ; $839C6D
CODE_839C6F:
    lda.w #$7D92                ; $839C6F
    sta.w $2116                 ; $839C72
    lda.b $4E                   ; $839C75
    sta.w $2118                 ; $839C77
    lda.b $50                   ; $839C7A
    sta.w $2118                 ; $839C7C
    lda.b $52                   ; $839C7F
    sta.w $2118                 ; $839C81
    lda.w #$7C39                ; $839C84
    sta.w $2116                 ; $839C87
    ldy.w #$0000                ; $839C8A
    lda.w $0301                 ; $839C8D
    and.w #$00FF                ; $839C90
    tax                         ; $839C93
    beq CODE_839CAA             ; $839C94
    dex                         ; $839C96
    beq CODE_839CA3             ; $839C97
    lda.w $9F68                 ; $839C99
CODE_839C9C:
    sta.w $2118                 ; $839C9C
    iny                         ; $839C9F
    dex                         ; $839CA0
    bne CODE_839C9C             ; $839CA1
CODE_839CA3:
    lda.w $9F68                 ; $839CA3
    sta.w $2118                 ; $839CA6
    iny                         ; $839CA9
CODE_839CAA:
    lda.w $9F52                 ; $839CAA
CODE_839CAD:
    sta.w $2118                 ; $839CAD
    iny                         ; $839CB0
    cpy.w #$0006                ; $839CB1
    bne CODE_839CAD             ; $839CB4
    ldx.b $AC                   ; $839CB6
    stx.w $2116                 ; $839CB8
    lda.w $2139                 ; $839CBB
    lda.w $2139                 ; $839CBE
    ora.w #$0006                ; $839CC1
    stx.w $2116                 ; $839CC4
    sta.w $2118                 ; $839CC7
    sep.b #$20                  ; $839CCA
    ldx.w #$4E95                ; $839CCC
    stx.w $4372                 ; $839CCF
    lda.b #$7E                  ; $839CD2
    sta.w $4374                 ; $839CD4
    lda.l $7E4E4F               ; $839CD7
    clc                         ; $839CDB
    adc.l $7E4E50               ; $839CDC
    sta.l $7E4E50               ; $839CE0
    bpl CODE_839CF1             ; $839CE4
    ldx.w #$A3DD                ; $839CE6
    stx.w $4372                 ; $839CE9
    lda.b #$83                  ; $839CEC
    sta.w $4374                 ; $839CEE
CODE_839CF1:
    ldx.w #$7CB2                ; $839CF1
    stx.w $2116                 ; $839CF4
    ldx.w #$1801                ; $839CF7
    stx.w $4370                 ; $839CFA
    ldx.w #$001A                ; $839CFD
    stx.w $4375                 ; $839D00
    lda.b #$80                  ; $839D03
    sta.w $420B                 ; $839D05
    ldx.w #$001A                ; $839D08
    stx.w $4375                 ; $839D0B
    ldx.w #$7CD2                ; $839D0E
    stx.w $2116                 ; $839D11
    sta.w $420B                 ; $839D14
    rep.b #$20                  ; $839D17
    lda.w #$7DE1                ; $839D19
    sta.w $2116                 ; $839D1C
    ldy.w #$0000                ; $839D1F
    lda.b $64                   ; $839D22
    and.w #$FFFE                ; $839D24
    beq CODE_839D36             ; $839D27
    tax                         ; $839D29
CODE_839D2A:
    lda.w $9F2E,y               ; $839D2A
    sta.w $2118                 ; $839D2D
    iny                         ; $839D30
    iny                         ; $839D31
    dex                         ; $839D32
    dex                         ; $839D33
    bne CODE_839D2A             ; $839D34
CODE_839D36:
    lda.b $64                   ; $839D36
    bit.w #$0001                ; $839D38
    beq CODE_839D45             ; $839D3B
    lda.w $9F3A,y               ; $839D3D
    sta.w $2118                 ; $839D40
    iny                         ; $839D43
    iny                         ; $839D44
CODE_839D45:
    cpy.w #$000C                ; $839D45
    beq CODE_839D57             ; $839D48
CODE_839D4A:
    lda.w $9F46,y               ; $839D4A
    sta.w $2118                 ; $839D4D
    iny                         ; $839D50
    iny                         ; $839D51
    cpy.w #$000C                ; $839D52
    bne CODE_839D4A             ; $839D55
CODE_839D57:
    lda.w #$7F41                ; $839D57
    sta.w $2116                 ; $839D5A
    lda.b $48                   ; $839D5D
    sta.w $2118                 ; $839D5F
    lda.b $4A                   ; $839D62
    sta.w $2118                 ; $839D64
    lda.b $4C                   ; $839D67
    sta.w $2118                 ; $839D69
    lda.w #$7DE8                ; $839D6C
    sta.w $2116                 ; $839D6F
    ldy.w #$0000                ; $839D72
    lda.w $02B0                 ; $839D75
    and.w #$00FF                ; $839D78
    tax                         ; $839D7B
    beq CODE_839D92             ; $839D7C
    dex                         ; $839D7E
    beq CODE_839D8B             ; $839D7F
    lda.w $9F68                 ; $839D81
CODE_839D84:
    sta.w $2118                 ; $839D84
    iny                         ; $839D87
    dex                         ; $839D88
    bne CODE_839D84             ; $839D89
CODE_839D8B:
    lda.w $9F68                 ; $839D8B
    sta.w $2118                 ; $839D8E
    iny                         ; $839D91
CODE_839D92:
    lda.w $9F52                 ; $839D92
CODE_839D95:
    sta.w $2118                 ; $839D95
    iny                         ; $839D98
    cpy.w #$0006                ; $839D99
    bne CODE_839D95             ; $839D9C
    ldx.b $A6                   ; $839D9E
    stx.w $2116                 ; $839DA0
    lda.w $2139                 ; $839DA3
    lda.w $2139                 ; $839DA6
    ora.w #$6000                ; $839DA9
    stx.w $2116                 ; $839DAC
    sta.w $2118                 ; $839DAF
    sep.b #$20                  ; $839DB2
    ldx.w #$4EC9                ; $839DB4
    stx.w $4372                 ; $839DB7
    lda.b #$7E                  ; $839DBA
    sta.w $4374                 ; $839DBC
    lda.l $7E4E51               ; $839DBF
    clc                         ; $839DC3
    adc.l $7E4E52               ; $839DC4
    sta.l $7E4E52               ; $839DC8
    bpl CODE_839DD9             ; $839DCC
    ldx.w #$A3DD                ; $839DCE
    stx.w $4372                 ; $839DD1
    lda.b #$83                  ; $839DD4
    sta.w $4374                 ; $839DD6
CODE_839DD9:
    ldx.w #$7E61                ; $839DD9
    stx.w $2116                 ; $839DDC
    ldx.w #$1801                ; $839DDF
    stx.w $4370                 ; $839DE2
    ldx.w #$001A                ; $839DE5
    stx.w $4375                 ; $839DE8
    lda.b #$80                  ; $839DEB
    sta.w $420B                 ; $839DED
    ldx.w #$001A                ; $839DF0
    stx.w $4375                 ; $839DF3
    ldx.w #$7E81                ; $839DF6
    stx.w $2116                 ; $839DF9
    sta.w $420B                 ; $839DFC
    rep.b #$20                  ; $839DFF
    lda.w #$7DF2                ; $839E01
    sta.w $2116                 ; $839E04
    ldy.w #$0000                ; $839E07
    lda.b $68                   ; $839E0A
    and.w #$FFFE                ; $839E0C
    beq CODE_839E1E             ; $839E0F
    tax                         ; $839E11
CODE_839E12:
    lda.w $9F2E,y               ; $839E12
    sta.w $2118                 ; $839E15
    iny                         ; $839E18
    iny                         ; $839E19
    dex                         ; $839E1A
    dex                         ; $839E1B
    bne CODE_839E12             ; $839E1C
CODE_839E1E:
    lda.b $68                   ; $839E1E
    bit.w #$0001                ; $839E20
    beq CODE_839E2D             ; $839E23
    lda.w $9F3A,y               ; $839E25
    sta.w $2118                 ; $839E28
    iny                         ; $839E2B
    iny                         ; $839E2C
CODE_839E2D:
    cpy.w #$000C                ; $839E2D
    beq CODE_839E3F             ; $839E30
CODE_839E32:
    lda.w $9F46,y               ; $839E32
    sta.w $2118                 ; $839E35
    iny                         ; $839E38
    iny                         ; $839E39
    cpy.w #$000C                ; $839E3A
    bne CODE_839E32             ; $839E3D
CODE_839E3F:
    lda.w #$7F52                ; $839E3F
    sta.w $2116                 ; $839E42
    lda.b $54                   ; $839E45
    sta.w $2118                 ; $839E47
    lda.b $56                   ; $839E4A
    sta.w $2118                 ; $839E4C
    lda.b $58                   ; $839E4F
    sta.w $2118                 ; $839E51
    lda.w #$7DF9                ; $839E54
    sta.w $2116                 ; $839E57
    ldy.w #$0000                ; $839E5A
    lda.w $0363                 ; $839E5D
    and.w #$00FF                ; $839E60
    tax                         ; $839E63
    beq CODE_839E7A             ; $839E64
    dex                         ; $839E66
    beq CODE_839E73             ; $839E67
    lda.w $9F68                 ; $839E69
CODE_839E6C:
    sta.w $2118                 ; $839E6C
    iny                         ; $839E6F
    dex                         ; $839E70
    bne CODE_839E6C             ; $839E71
CODE_839E73:
    lda.w $9F68                 ; $839E73
    sta.w $2118                 ; $839E76
    iny                         ; $839E79
CODE_839E7A:
    lda.w $9F52                 ; $839E7A
CODE_839E7D:
    sta.w $2118                 ; $839E7D
    iny                         ; $839E80
    cpy.w #$0006                ; $839E81
    bne CODE_839E7D             ; $839E84
    ldx.b $AE                   ; $839E86
    stx.w $2116                 ; $839E88
    lda.w $2139                 ; $839E8B
    lda.w $2139                 ; $839E8E
    ora.w #$0006                ; $839E91
    stx.w $2116                 ; $839E94
    sta.w $2118                 ; $839E97
    sep.b #$20                  ; $839E9A
    ldx.w #$4EFD                ; $839E9C
    stx.w $4372                 ; $839E9F
    lda.b #$7E                  ; $839EA2
    sta.w $4374                 ; $839EA4
    lda.l $7E4E53               ; $839EA7
    clc                         ; $839EAB
    adc.l $7E4E54               ; $839EAC
    sta.l $7E4E54               ; $839EB0
    bpl CODE_839EC1             ; $839EB4
    ldx.w #$A3DD                ; $839EB6
    stx.w $4372                 ; $839EB9
    lda.b #$83                  ; $839EBC
    sta.w $4374                 ; $839EBE
CODE_839EC1:
    ldx.w #$7E72                ; $839EC1
    stx.w $2116                 ; $839EC4
    ldx.w #$1801                ; $839EC7
    stx.w $4370                 ; $839ECA
    ldx.w #$001A                ; $839ECD
    stx.w $4375                 ; $839ED0
    lda.b #$80                  ; $839ED3
    sta.w $420B                 ; $839ED5
    ldx.w #$001A                ; $839ED8
    stx.w $4375                 ; $839EDB
    ldx.w #$7E92                ; $839EDE
    stx.w $2116                 ; $839EE1
    sta.w $420B                 ; $839EE4
    rep.b #$20                  ; $839EE7
    rts                         ; $839EE9
    incbin !baserom:$19EEA..$1CD70
mode1_road_project_view2:
    pea.w $8330                 ; $83CD70
    plb                         ; $83CD73
    ldy.w #$0023                ; $83CD74
    ldx.w #$2B2B                ; $83CD77
    lda.w #$019C                ; $83CD7A
    sta.b $1A                   ; $83CD7D
    lda.w #$0004                ; $83CD7F
    sta.b $1C                   ; $83CD82
    lda.w $00B6                 ; $83CD84
    bne CODE_83CD99             ; $83CD87
    ldy.w #$0045                ; $83CD89
    ldx.w #$2C2B                ; $83CD8C
    lda.w #$033C                ; $83CD8F
    sta.b $1A                   ; $83CD92
    lda.w #$01A4                ; $83CD94
    sta.b $1C                   ; $83CD97
CODE_83CD99:
    stx.b $18                   ; $83CD99
    lda.w $02D8                 ; $83CD9B
    lsr a                       ; $83CD9E
    and.w #$007E                ; $83CD9F
    eor.w #$FFFF                ; $83CDA2
    sec                         ; $83CDA5
    adc.w #$0080                ; $83CDA6
    sta.b $14                   ; $83CDA9
    lda.w $02D9                 ; $83CDAB
    asl a                       ; $83CDAE
    and.w #$003E                ; $83CDAF
    sta.b $16                   ; $83CDB2
    ldx.b $16                   ; $83CDB4
    stz.w $8000                 ; $83CDB6
    lda.l $7E406C,x             ; $83CDB9
    sta.w $8000                 ; $83CDBD
    lda.w #$FC00                ; $83CDC0
    sta.w $8000                 ; $83CDC3
    nop                         ; $83CDC6
    lda.w $8000                 ; $83CDC7
    clc                         ; $83CDCA
    adc.w #$8000                ; $83CDCB
    lda.w $8000                 ; $83CDCE
    adc.w #$0000                ; $83CDD1
    sta.w $0178                 ; $83CDD4
    stz.w $8000                 ; $83CDD7
    lda.l $7E40EE,x             ; $83CDDA
    sta.w $8000                 ; $83CDDE
    lda.w #$0080                ; $83CDE1
    sec                         ; $83CDE4
    sbc.b $14                   ; $83CDE5
    lsr a                       ; $83CDE7
    xba                         ; $83CDE8
    sta.w $8000                 ; $83CDE9
    nop                         ; $83CDEC
    lda.w $8000                 ; $83CDED
    sta.b $1E                   ; $83CDF0
    lda.w $8000                 ; $83CDF2
    asl.b $1E                   ; $83CDF5
    rol a                       ; $83CDF7
    asl.b $1E                   ; $83CDF8
    rol a                       ; $83CDFA
    eor.w #$FFFF                ; $83CDFB
    inc a                       ; $83CDFE
    sta.b $1E                   ; $83CDFF
    lda.w $02BB                 ; $83CE01
    clc                         ; $83CE04
    adc.b $1E                   ; $83CE05
    sta.b $1E                   ; $83CE07
    stz.w $8000                 ; $83CE09
    lda.w $02BB                 ; $83CE0C
    sta.w $8000                 ; $83CE0F
    lda.b $14                   ; $83CE12
    lsr a                       ; $83CE14
    xba                         ; $83CE15
    sta.w $8000                 ; $83CE16
    nop                         ; $83CE19
    lda.w $8000                 ; $83CE1A
    sta.b $20                   ; $83CE1D
    lda.w $8000                 ; $83CE1F
    asl.b $20                   ; $83CE22
    rol a                       ; $83CE24
    asl.b $20                   ; $83CE25
    rol a                       ; $83CE27
    sta.b $20                   ; $83CE28
    stz.w $8000                 ; $83CE2A
    lda.l $7E406E,x             ; $83CE2D
    sta.w $8000                 ; $83CE31
    lda.w #$0080                ; $83CE34
    sec                         ; $83CE37
    sbc.b $14                   ; $83CE38
    lsr a                       ; $83CE3A
    xba                         ; $83CE3B
    sta.w $8000                 ; $83CE3C
    ldx.w #$0000                ; $83CE3F
    lda.w $8000                 ; $83CE42
    sta.b $26                   ; $83CE45
    lda.w $8000                 ; $83CE47
    asl.b $26                   ; $83CE4A
    rol a                       ; $83CE4C
    asl.b $26                   ; $83CE4D
    rol a                       ; $83CE4F
    eor.w #$FFFF                ; $83CE50
    inc a                       ; $83CE53
    sta.b $26                   ; $83CE54
    bpl CODE_83CE59             ; $83CE56
    dex                         ; $83CE58
CODE_83CE59:
    stx.b $28                   ; $83CE59
    lda.w $02BE                 ; $83CE5B
    adc.b $26                   ; $83CE5E
    sta.b $26                   ; $83CE60
    lda.w $02C0                 ; $83CE62
    and.w #$00FF                ; $83CE65
    bit.w #$0080                ; $83CE68
    beq CODE_83CE70             ; $83CE6B
    ora.w #$FF00                ; $83CE6D
CODE_83CE70:
    adc.b $28                   ; $83CE70
    sta.b $28                   ; $83CE72
    stz.w $8000                 ; $83CE74
    lda.b $26                   ; $83CE77
    sta.w $8000                 ; $83CE79
    lda.b $14                   ; $83CE7C
    lsr a                       ; $83CE7E
    xba                         ; $83CE7F
    sta.w $8000                 ; $83CE80
    ldx.w #$0000                ; $83CE83
    lda.w $8000                 ; $83CE86
    lda.w $8000                 ; $83CE89
    sta.b $2A                   ; $83CE8C
    bpl CODE_83CE91             ; $83CE8E
    dex                         ; $83CE90
CODE_83CE91:
    stx.b $2C                   ; $83CE91
    stz.w $8000                 ; $83CE93
    lda.b $28                   ; $83CE96
    bit.b $26                   ; $83CE98
    bpl CODE_83CE9D             ; $83CE9A
    inc a                       ; $83CE9C
CODE_83CE9D:
    sta.w $8000                 ; $83CE9D
    lda.b $14                   ; $83CEA0
    lsr a                       ; $83CEA2
    xba                         ; $83CEA3
    sta.w $8000                 ; $83CEA4
    nop                         ; $83CEA7
    lda.w $8000                 ; $83CEA8
    clc                         ; $83CEAB
    adc.b $2A                   ; $83CEAC
    sta.b $2A                   ; $83CEAE
    lda.w $8000                 ; $83CEB0
    adc.b $2C                   ; $83CEB3
    asl.b $2A                   ; $83CEB5
    rol a                       ; $83CEB7
    asl.b $2A                   ; $83CEB8
    rol a                       ; $83CEBA
    sta.b $2C                   ; $83CEBB
    tyx                         ; $83CEBD
    lda.w #$0000                ; $83CEBE
    sta.l $7E4D95,x             ; $83CEC1
    lda.w #$0001                ; $83CEC5
    bit.b $E6                   ; $83CEC8
    bmi CODE_83CED4             ; $83CECA
    lda.w #$000F                ; $83CECC
    sta.w $8000                 ; $83CECF
    lda.b $E6                   ; $83CED2
CODE_83CED4:
    sta.w $8000                 ; $83CED4
    nop                         ; $83CED7
    nop                         ; $83CED8
    lda.w #$0000                ; $83CED9
    sta.w $8000                 ; $83CEDC
    lda.w $02F5                 ; $83CEDF
    bpl CODE_83CEF1             ; $83CEE2
    incbin !baserom:$1CEE4..$1CEF1
CODE_83CEF1:
    clc                         ; $83CEF1
    adc.w #$0033                ; $83CEF2
    sta.w $8000                 ; $83CEF5
    lda.w #$00DC                ; $83CEF8
    sta.l $7E4D95,x             ; $83CEFB
    dex                         ; $83CEFF
    lda.w $02F5                 ; $83CF00
    bpl CODE_83CF0B             ; $83CF03
    incbin !baserom:$1CF05..$1CF0B
CODE_83CF0B:
    lda.w #$0033                ; $83CF0B
    sta.w $8000                 ; $83CF0E
    lda.w #$FFCB                ; $83CF11
    sta.w $8000                 ; $83CF14
    lda.w #$00BF                ; $83CF17
    sta.w $8000                 ; $83CF1A
    lda.w $02F5                 ; $83CF1D
    bpl CODE_83CF28             ; $83CF20
    incbin !baserom:$1CF22..$1CF28
CODE_83CF28:
    lda.w #$00DC                ; $83CF28
    sta.w $8000                 ; $83CF2B
    lda.w $02B3                 ; $83CF2E
    sta.w $8000                 ; $83CF31
    lda.w $02B5                 ; $83CF34
    beq CODE_83CF3E             ; $83CF37
    dec a                       ; $83CF39
    bpl CODE_83CF3E             ; $83CF3A
    inc a                       ; $83CF3C
    inc a                       ; $83CF3D
CODE_83CF3E:
    sta.w $8000                 ; $83CF3E
    lda.w #$0180                ; $83CF41
    sta.w $8000                 ; $83CF44
    lda.w $02F5                 ; $83CF47
    bpl CODE_83CF53             ; $83CF4A
    incbin !baserom:$1CF4C..$1CF53
CODE_83CF53:
    lda.b $1A                   ; $83CF53
    sta.w $8000                 ; $83CF55
    lda.w #$00C0                ; $83CF58
    sta.w $8000                 ; $83CF5B
    stx.b $22                   ; $83CF5E
    lda.b $1D                   ; $83CF60
    and.w #$FF00                ; $83CF62
    sta.w $8000                 ; $83CF65
    lda.b $1F                   ; $83CF68
    and.w #$00FF                ; $83CF6A
    bit.w #$0080                ; $83CF6D
    beq CODE_83CF75             ; $83CF70
    ora.w #$FF00                ; $83CF72
CODE_83CF75:
    sta.w $8000                 ; $83CF75
    lda.w $02BD                 ; $83CF78
    and.w #$00FF                ; $83CF7B
    pha                         ; $83CF7E
    lda.b $25                   ; $83CF7F
    and.w #$FF00                ; $83CF81
    ora.b $01,s                 ; $83CF84
    sta.w $8000                 ; $83CF86
    lda.b $27                   ; $83CF89
    sta.w $8000                 ; $83CF8B
    pla                         ; $83CF8E
    lda.w #$0002                ; $83CF8F
    sta.b $24                   ; $83CF92
    ldx.b $14                   ; $83CF94
    lda.l $83ACEC,x             ; $83CF96
    sta.w $8000                 ; $83CF9A
    sta.b $30                   ; $83CF9D
    lda.b $20                   ; $83CF9F
    sta.w $8000                 ; $83CFA1
    lda.b $29                   ; $83CFA4
    and.w #$FF00                ; $83CFA6
    sta.w $8000                 ; $83CFA9
    lda.b $2B                   ; $83CFAC
    sta.w $8000                 ; $83CFAE
    jmp.w $CFBD                 ; $83CFB1
    lda.l $83ACEC,x             ; $83CFB4
    sta.w $8000                 ; $83CFB8
    sta.b $30                   ; $83CFBB
    ldx.b $16                   ; $83CFBD
    lda.l $7E40EE,x             ; $83CFBF
    sta.w $8000                 ; $83CFC3
    lda.l $7E406E,x             ; $83CFC6
    sta.w $8000                 ; $83CFCA
    lda.l $7E416C,x             ; $83CFCD
    sta.w $8000                 ; $83CFD1
    inx                         ; $83CFD4
    inx                         ; $83CFD5
    stx.b $16                   ; $83CFD6
    ldx.b $24                   ; $83CFD8
    lda.w $8000                 ; $83CFDA
    sta.l $7E352C,x             ; $83CFDD
    lda.w $8000                 ; $83CFE1
    sta.l $7E362C,x             ; $83CFE4
    lda.w $8000                 ; $83CFE8
    sta.l $7E372C,x             ; $83CFEB
    lda.w $8000                 ; $83CFEF
    sta.l $7E382C,x             ; $83CFF2
    lda.b $30                   ; $83CFF6
    sta.l $7E392C,x             ; $83CFF8
    lda.b $14                   ; $83CFFC
    sta.l $7E3A2C,x             ; $83CFFE
    inx                         ; $83D002
    inx                         ; $83D003
    stx.b $24                   ; $83D004
CODE_83D006:
    lda.w $C000                 ; $83D006
    bpl CODE_83D006             ; $83D009
    lda.w $8000                 ; $83D00B
    bne CODE_83D013             ; $83D00E
    jmp.w $D0EE                 ; $83D010
CODE_83D013:
    tay                         ; $83D013
    ldx.b $22                   ; $83D014
    sep.b #$20                  ; $83D016
    eor.b #$FF                  ; $83D018
    sec                         ; $83D01A
    adc.l $7E4D96,x             ; $83D01B
    sta.l $7E4D95,x             ; $83D01F
    rep.b #$20                  ; $83D023
    phy                         ; $83D025
    lda.b $E6                   ; $83D026
    bpl CODE_83D02D             ; $83D028
    jmp.w $D0AF                 ; $83D02A
CODE_83D02D:
    ldx.b $14                   ; $83D02D
    lda.l $83ACEC,x             ; $83D02F
    sta.w $8000                 ; $83D033
    ldx.b $16                   ; $83D036
    lda.l $7E426A,x             ; $83D038
    sec                         ; $83D03C
    sbc.w #$0007                ; $83D03D
    tax                         ; $83D040
    txy                         ; $83D041
    lda.l $900000,x             ; $83D042
    sta.w $8000                 ; $83D046
    lda.b $18                   ; $83D049
    sec                         ; $83D04B
    sbc.w #$0008                ; $83D04C
    sta.b $18                   ; $83D04F
    tax                         ; $83D051
    lda.w $8000                 ; $83D052
    sta.l $7E0001,x             ; $83D055
    ldx.b $14                   ; $83D059
    lda.l $7E7C73,x             ; $83D05B
    sta.w $8000                 ; $83D05F
    tyx                         ; $83D062
    lda.l $900002,x             ; $83D063
    sta.w $8000                 ; $83D067
    nop                         ; $83D06A
    nop                         ; $83D06B
    ldx.b $18                   ; $83D06C
    lda.w $8000                 ; $83D06E
    sta.l $7E0003,x             ; $83D071
    ldx.b $14                   ; $83D075
    lda.l $7E7C73,x             ; $83D077
    sta.w $8000                 ; $83D07B
    tyx                         ; $83D07E
    lda.l $900004,x             ; $83D07F
    sta.w $8000                 ; $83D083
    nop                         ; $83D086
    nop                         ; $83D087
    ldx.b $18                   ; $83D088
    lda.w $8000                 ; $83D08A
    sta.l $7E0005,x             ; $83D08D
    ldx.b $14                   ; $83D091
    lda.l $7E7C73,x             ; $83D093
    sta.w $8000                 ; $83D097
    tyx                         ; $83D09A
    lda.l $900006,x             ; $83D09B
    sta.w $8000                 ; $83D09F
    nop                         ; $83D0A2
    nop                         ; $83D0A3
    ldx.b $18                   ; $83D0A4
    lda.w $8000                 ; $83D0A6
    sta.l $7E0007,x             ; $83D0A9
    bra CODE_83D0C2             ; $83D0AD
    ldx.b $16                   ; $83D0AF
    lda.l $7E426A,x             ; $83D0B1
    tax                         ; $83D0B5
    phb                         ; $83D0B6
    ldy.b $18                   ; $83D0B7
    lda.w #$0007                ; $83D0B9
    mvp $7E,$90                 ; $83D0BC
    sty.b $18                   ; $83D0BF
    plb                         ; $83D0C1
CODE_83D0C2:
    ply                         ; $83D0C2
CODE_83D0C3:
    lda.w $C000                 ; $83D0C3
    bpl CODE_83D0C3             ; $83D0C6
    dey                         ; $83D0C8
    beq CODE_83D0DD             ; $83D0C9
CODE_83D0CB:
    ldx.w $8000                 ; $83D0CB
    lda.w $8000                 ; $83D0CE
    sta.w $0AF8,x               ; $83D0D1
    lda.w $8000                 ; $83D0D4
    sta.w $0AF6,x               ; $83D0D7
    dey                         ; $83D0DA
    bne CODE_83D0CB             ; $83D0DB
CODE_83D0DD:
    ldx.w $8000                 ; $83D0DD
    lda.w $8000                 ; $83D0E0
    sta.w $0AF8,x               ; $83D0E3
    lda.w $8000                 ; $83D0E6
    sta.w $0AF6,x               ; $83D0E9
    dec.b $22                   ; $83D0EC
    ldx.b $16                   ; $83D0EE
    lda.l $7E42EA,x             ; $83D0F0
    bpl CODE_83D16F             ; $83D0F4
    ldx.w #$8001                ; $83D0F6
    stx.w $8000                 ; $83D0F9
    ldx.b $24                   ; $83D0FC
    bit.w #$0002                ; $83D0FE
    beq CODE_83D143             ; $83D101
    lda.l $7E392A,x             ; $83D103
    sta.w $8000                 ; $83D107
    ldx.b $16                   ; $83D10A
    lda.l $7E406D,x             ; $83D10C
    and.w #$00FF                ; $83D110
    bit.w #$0080                ; $83D113
    beq CODE_83D11B             ; $83D116
    incbin !baserom:$1D118..$1D11B
CODE_83D11B:
    sta.b $20                   ; $83D11B
    lda.l $7E436E,x             ; $83D11D
    sec                         ; $83D121
    sbc.l $7E4370,x             ; $83D122
    clc                         ; $83D126
    adc.l $7E436E,x             ; $83D127
    eor.w #$FFFF                ; $83D12B
    inc a                       ; $83D12E
    sta.w $8000                 ; $83D12F
    lda.l $7E436E,x             ; $83D132
    sec                         ; $83D136
    sbc.l $7E4370,x             ; $83D137
    sec                         ; $83D13B
    sbc.b $20                   ; $83D13C
    sta.w $8000                 ; $83D13E
    bra CODE_83D16F             ; $83D141
CODE_83D143:
    lda.l $7E392A,x             ; $83D143
    sta.w $8000                 ; $83D147
    ldx.b $16                   ; $83D14A
    lda.l $7E406B,x             ; $83D14C
    and.w #$00FF                ; $83D150
    bit.w #$0080                ; $83D153
    beq CODE_83D15B             ; $83D156
    incbin !baserom:$1D158..$1D15B
CODE_83D15B:
    sta.b $20                   ; $83D15B
    lda.l $7E436C,x             ; $83D15D
    sta.w $8000                 ; $83D161
    sec                         ; $83D164
    sbc.l $7E436A,x             ; $83D165
    sec                         ; $83D169
    sbc.b $20                   ; $83D16A
    sta.w $8000                 ; $83D16C
CODE_83D16F:
    lda.b $14                   ; $83D16F
    clc                         ; $83D171
    adc.w #$0080                ; $83D172
    sta.b $14                   ; $83D175
    cmp.w #$0E00                ; $83D177
    bcs CODE_83D180             ; $83D17A
    tax                         ; $83D17C
    jmp.w $CFB4                 ; $83D17D
CODE_83D180:
    lda.w #$8000                ; $83D180
    sta.w $8000                 ; $83D183
    lda.b $24                   ; $83D186
    sta.b $92                   ; $83D188
    ldx.w #$0023                ; $83D18A
    lda.w $00B6                 ; $83D18D
    bne CODE_83D195             ; $83D190
    ldx.w #$0045                ; $83D192
CODE_83D195:
    ldy.w #$0000                ; $83D195
    lda.w $02F5                 ; $83D198
    bpl CODE_83D1AA             ; $83D19B
    incbin !baserom:$1D19D..$1D1AA
CODE_83D1AA:
    tya                         ; $83D1AA
    sta.l $7E4D95,x             ; $83D1AB
    lda.b $22                   ; $83D1AF
    sta.w $0166                 ; $83D1B1
    ldy.b $18                   ; $83D1B4
    iny                         ; $83D1B6
    sty.w $015E                 ; $83D1B7
    lda.b $1A                   ; $83D1BA
    sta.w $1E42                 ; $83D1BC
    plb                         ; $83D1BF
    rtl                         ; $83D1C0
mode1_road_project_view1:
    pea.w $8330                 ; $83D1C1
    plb                         ; $83D1C4
    ldy.w #$0023                ; $83D1C5
    ldx.w #$272B                ; $83D1C8
    lda.w #$019C                ; $83D1CB
    sta.b $1A                   ; $83D1CE
    lda.w #$0004                ; $83D1D0
    sta.b $1C                   ; $83D1D3
    lda.w $00B4                 ; $83D1D5
    bne CODE_83D1EA             ; $83D1D8
    ldy.w #$0045                ; $83D1DA
    ldx.w #$282B                ; $83D1DD
    lda.w #$033C                ; $83D1E0
    sta.b $1A                   ; $83D1E3
    lda.w #$01A4                ; $83D1E5
    sta.b $1C                   ; $83D1E8
CODE_83D1EA:
    stx.b $18                   ; $83D1EA
    lda.w $0225                 ; $83D1EC
    lsr a                       ; $83D1EF
    and.w #$007E                ; $83D1F0
    eor.w #$FFFF                ; $83D1F3
    sec                         ; $83D1F6
    adc.w #$0080                ; $83D1F7
    sta.b $14                   ; $83D1FA
    lda.w $0226                 ; $83D1FC
    asl a                       ; $83D1FF
    and.w #$003E                ; $83D200
    sta.b $16                   ; $83D203
    ldx.b $16                   ; $83D205
    stz.w $8000                 ; $83D207
    lda.l $7E3AEC,x             ; $83D20A
    sta.w $8000                 ; $83D20E
    lda.w #$FC00                ; $83D211
    sta.w $8000                 ; $83D214
    nop                         ; $83D217
    lda.w $8000                 ; $83D218
    clc                         ; $83D21B
    adc.w #$8000                ; $83D21C
    lda.w $8000                 ; $83D21F
    adc.w #$0000                ; $83D222
    sta.w $0174                 ; $83D225
    stz.w $8000                 ; $83D228
    lda.l $7E3B6E,x             ; $83D22B
    sta.w $8000                 ; $83D22F
    lda.w #$0080                ; $83D232
    sec                         ; $83D235
    sbc.b $14                   ; $83D236
    lsr a                       ; $83D238
    xba                         ; $83D239
    sta.w $8000                 ; $83D23A
    nop                         ; $83D23D
    lda.w $8000                 ; $83D23E
    sta.b $1E                   ; $83D241
    lda.w $8000                 ; $83D243
    asl.b $1E                   ; $83D246
    rol a                       ; $83D248
    asl.b $1E                   ; $83D249
    rol a                       ; $83D24B
    eor.w #$FFFF                ; $83D24C
    inc a                       ; $83D24F
    sta.b $1E                   ; $83D250
    lda.w $0208                 ; $83D252
    clc                         ; $83D255
    adc.b $1E                   ; $83D256
    sta.b $1E                   ; $83D258
    stz.w $8000                 ; $83D25A
    lda.w $0208                 ; $83D25D
    sta.w $8000                 ; $83D260
    lda.b $14                   ; $83D263
    lsr a                       ; $83D265
    xba                         ; $83D266
    sta.w $8000                 ; $83D267
    nop                         ; $83D26A
    lda.w $8000                 ; $83D26B
    sta.b $20                   ; $83D26E
    lda.w $8000                 ; $83D270
    asl.b $20                   ; $83D273
    rol a                       ; $83D275
    asl.b $20                   ; $83D276
    rol a                       ; $83D278
    sta.b $20                   ; $83D279
    stz.w $8000                 ; $83D27B
    lda.l $7E3AEE,x             ; $83D27E
    sta.w $8000                 ; $83D282
    lda.w #$0080                ; $83D285
    sec                         ; $83D288
    sbc.b $14                   ; $83D289
    lsr a                       ; $83D28B
    xba                         ; $83D28C
    sta.w $8000                 ; $83D28D
    ldx.w #$0000                ; $83D290
    lda.w $8000                 ; $83D293
    sta.b $26                   ; $83D296
    lda.w $8000                 ; $83D298
    asl.b $26                   ; $83D29B
    rol a                       ; $83D29D
    asl.b $26                   ; $83D29E
    rol a                       ; $83D2A0
    eor.w #$FFFF                ; $83D2A1
    inc a                       ; $83D2A4
    sta.b $26                   ; $83D2A5
    bpl CODE_83D2AA             ; $83D2A7
    dex                         ; $83D2A9
CODE_83D2AA:
    stx.b $28                   ; $83D2AA
    lda.w $020B                 ; $83D2AC
    adc.b $26                   ; $83D2AF
    sta.b $26                   ; $83D2B1
    lda.w $020D                 ; $83D2B3
    and.w #$00FF                ; $83D2B6
    bit.w #$0080                ; $83D2B9
    beq CODE_83D2C1             ; $83D2BC
    ora.w #$FF00                ; $83D2BE
CODE_83D2C1:
    adc.b $28                   ; $83D2C1
    sta.b $28                   ; $83D2C3
    stz.w $8000                 ; $83D2C5
    lda.b $26                   ; $83D2C8
    sta.w $8000                 ; $83D2CA
    lda.b $14                   ; $83D2CD
    lsr a                       ; $83D2CF
    xba                         ; $83D2D0
    sta.w $8000                 ; $83D2D1
    ldx.w #$0000                ; $83D2D4
    lda.w $8000                 ; $83D2D7
    lda.w $8000                 ; $83D2DA
    sta.b $2A                   ; $83D2DD
    bpl CODE_83D2E2             ; $83D2DF
    dex                         ; $83D2E1
CODE_83D2E2:
    stx.b $2C                   ; $83D2E2
    stz.w $8000                 ; $83D2E4
    lda.b $28                   ; $83D2E7
    bit.b $26                   ; $83D2E9
    bpl CODE_83D2EE             ; $83D2EB
    inc a                       ; $83D2ED
CODE_83D2EE:
    sta.w $8000                 ; $83D2EE
    lda.b $14                   ; $83D2F1
    lsr a                       ; $83D2F3
    xba                         ; $83D2F4
    sta.w $8000                 ; $83D2F5
    nop                         ; $83D2F8
    lda.w $8000                 ; $83D2F9
    clc                         ; $83D2FC
    adc.b $2A                   ; $83D2FD
    sta.b $2A                   ; $83D2FF
    lda.w $8000                 ; $83D301
    adc.b $2C                   ; $83D304
    asl.b $2A                   ; $83D306
    rol a                       ; $83D308
    asl.b $2A                   ; $83D309
    rol a                       ; $83D30B
    sta.b $2C                   ; $83D30C
    tyx                         ; $83D30E
    lda.w #$0000                ; $83D30F
    sta.l $7E4CFD,x             ; $83D312
    lda.w #$0001                ; $83D316
    bit.b $E4                   ; $83D319
    bmi CODE_83D325             ; $83D31B
    lda.w #$000F                ; $83D31D
    sta.w $8000                 ; $83D320
    lda.b $E4                   ; $83D323
CODE_83D325:
    sta.w $8000                 ; $83D325
    nop                         ; $83D328
    nop                         ; $83D329
    lda.w #$0000                ; $83D32A
    sta.w $8000                 ; $83D32D
    lda.w $0242                 ; $83D330
    bpl CODE_83D342             ; $83D333
    incbin !baserom:$1D335..$1D342
CODE_83D342:
    clc                         ; $83D342
    adc.w #$0033                ; $83D343
    sta.w $8000                 ; $83D346
    lda.w #$006C                ; $83D349
    sta.l $7E4CFD,x             ; $83D34C
    dex                         ; $83D350
    lda.w $0242                 ; $83D351
    bpl CODE_83D35C             ; $83D354
    incbin !baserom:$1D356..$1D35C
CODE_83D35C:
    lda.w #$0033                ; $83D35C
    sta.w $8000                 ; $83D35F
    lda.w #$FFCB                ; $83D362
    sta.w $8000                 ; $83D365
    lda.w #$00BF                ; $83D368
    sta.w $8000                 ; $83D36B
    lda.w $0242                 ; $83D36E
    bpl CODE_83D379             ; $83D371
    incbin !baserom:$1D373..$1D379
CODE_83D379:
    lda.w #$006C                ; $83D379
    sta.w $8000                 ; $83D37C
    lda.w $0200                 ; $83D37F
    sta.w $8000                 ; $83D382
    lda.w $0202                 ; $83D385
    beq CODE_83D38F             ; $83D388
    dec a                       ; $83D38A
    bpl CODE_83D38F             ; $83D38B
    inc a                       ; $83D38D
    inc a                       ; $83D38E
CODE_83D38F:
    sta.w $8000                 ; $83D38F
    lda.w #$0180                ; $83D392
    sta.w $8000                 ; $83D395
    lda.w $0242                 ; $83D398
    bpl CODE_83D3A4             ; $83D39B
    incbin !baserom:$1D39D..$1D3A4
CODE_83D3A4:
    lda.b $1A                   ; $83D3A4
    sta.w $8000                 ; $83D3A6
    lda.w #$00C0                ; $83D3A9
    sta.w $8000                 ; $83D3AC
    stx.b $22                   ; $83D3AF
    lda.b $1D                   ; $83D3B1
    and.w #$FF00                ; $83D3B3
    sta.w $8000                 ; $83D3B6
    lda.b $1F                   ; $83D3B9
    and.w #$00FF                ; $83D3BB
    bit.w #$0080                ; $83D3BE
    beq CODE_83D3C6             ; $83D3C1
    ora.w #$FF00                ; $83D3C3
CODE_83D3C6:
    sta.w $8000                 ; $83D3C6
    lda.w $020A                 ; $83D3C9
    and.w #$00FF                ; $83D3CC
    pha                         ; $83D3CF
    lda.b $25                   ; $83D3D0
    and.w #$FF00                ; $83D3D2
    ora.b $01,s                 ; $83D3D5
    sta.w $8000                 ; $83D3D7
    lda.b $27                   ; $83D3DA
    sta.w $8000                 ; $83D3DC
    pla                         ; $83D3DF
    lda.w #$0002                ; $83D3E0
    sta.b $24                   ; $83D3E3
    ldx.b $14                   ; $83D3E5
    lda.l $83ACEC,x             ; $83D3E7
    sta.w $8000                 ; $83D3EB
    sta.b $30                   ; $83D3EE
    lda.b $20                   ; $83D3F0
    sta.w $8000                 ; $83D3F2
    lda.b $29                   ; $83D3F5
    and.w #$FF00                ; $83D3F7
    sta.w $8000                 ; $83D3FA
    lda.b $2B                   ; $83D3FD
    sta.w $8000                 ; $83D3FF
    jmp.w $D40E                 ; $83D402
    lda.l $83ACEC,x             ; $83D405
    sta.w $8000                 ; $83D409
    sta.b $30                   ; $83D40C
    ldx.b $16                   ; $83D40E
    lda.l $7E3B6E,x             ; $83D410
    sta.w $8000                 ; $83D414
    lda.l $7E3AEE,x             ; $83D417
    sta.w $8000                 ; $83D41B
    lda.l $7E3BEC,x             ; $83D41E
    sta.w $8000                 ; $83D422
    inx                         ; $83D425
    inx                         ; $83D426
    stx.b $16                   ; $83D427
    ldx.b $24                   ; $83D429
    lda.w $8000                 ; $83D42B
    sta.l $7E34EC,x             ; $83D42E
    lda.w $8000                 ; $83D432
    sta.l $7E35EC,x             ; $83D435
    lda.w $8000                 ; $83D439
    sta.l $7E36EC,x             ; $83D43C
    lda.w $8000                 ; $83D440
    sta.l $7E37EC,x             ; $83D443
    lda.b $30                   ; $83D447
    sta.l $7E38EC,x             ; $83D449
    lda.b $14                   ; $83D44D
    sta.l $7E39EC,x             ; $83D44F
    inx                         ; $83D453
    inx                         ; $83D454
    stx.b $24                   ; $83D455
CODE_83D457:
    lda.w $C000                 ; $83D457
    bpl CODE_83D457             ; $83D45A
    lda.w $8000                 ; $83D45C
    bne CODE_83D464             ; $83D45F
    jmp.w $D53F                 ; $83D461
CODE_83D464:
    tay                         ; $83D464
    ldx.b $22                   ; $83D465
    sep.b #$20                  ; $83D467
    eor.b #$FF                  ; $83D469
    sec                         ; $83D46B
    adc.l $7E4CFE,x             ; $83D46C
    sta.l $7E4CFD,x             ; $83D470
    rep.b #$20                  ; $83D474
    phy                         ; $83D476
    lda.b $E4                   ; $83D477
    bpl CODE_83D47E             ; $83D479
    jmp.w $D500                 ; $83D47B
CODE_83D47E:
    ldx.b $14                   ; $83D47E
    lda.l $83ACEC,x             ; $83D480
    sta.w $8000                 ; $83D484
    ldx.b $16                   ; $83D487
    lda.l $7E3CEA,x             ; $83D489
    sec                         ; $83D48D
    sbc.w #$0007                ; $83D48E
    tax                         ; $83D491
    txy                         ; $83D492
    lda.l $900000,x             ; $83D493
    sta.w $8000                 ; $83D497
    lda.b $18                   ; $83D49A
    sec                         ; $83D49C
    sbc.w #$0008                ; $83D49D
    sta.b $18                   ; $83D4A0
    tax                         ; $83D4A2
    lda.w $8000                 ; $83D4A3
    sta.l $7E0001,x             ; $83D4A6
    ldx.b $14                   ; $83D4AA
    lda.l $7E6C33,x             ; $83D4AC
    sta.w $8000                 ; $83D4B0
    tyx                         ; $83D4B3
    lda.l $900002,x             ; $83D4B4
    sta.w $8000                 ; $83D4B8
    nop                         ; $83D4BB
    nop                         ; $83D4BC
    ldx.b $18                   ; $83D4BD
    lda.w $8000                 ; $83D4BF
    sta.l $7E0003,x             ; $83D4C2
    ldx.b $14                   ; $83D4C6
    lda.l $7E6C33,x             ; $83D4C8
    sta.w $8000                 ; $83D4CC
    tyx                         ; $83D4CF
    lda.l $900004,x             ; $83D4D0
    sta.w $8000                 ; $83D4D4
    nop                         ; $83D4D7
    nop                         ; $83D4D8
    ldx.b $18                   ; $83D4D9
    lda.w $8000                 ; $83D4DB
    sta.l $7E0005,x             ; $83D4DE
    ldx.b $14                   ; $83D4E2
    lda.l $7E6C33,x             ; $83D4E4
    sta.w $8000                 ; $83D4E8
    tyx                         ; $83D4EB
    lda.l $900006,x             ; $83D4EC
    sta.w $8000                 ; $83D4F0
    nop                         ; $83D4F3
    nop                         ; $83D4F4
    ldx.b $18                   ; $83D4F5
    lda.w $8000                 ; $83D4F7
    sta.l $7E0007,x             ; $83D4FA
    bra CODE_83D513             ; $83D4FE
    ldx.b $16                   ; $83D500
    lda.l $7E3CEA,x             ; $83D502
    tax                         ; $83D506
    phb                         ; $83D507
    ldy.b $18                   ; $83D508
    lda.w #$0007                ; $83D50A
    mvp $7E,$90                 ; $83D50D
    sty.b $18                   ; $83D510
    plb                         ; $83D512
CODE_83D513:
    ply                         ; $83D513
CODE_83D514:
    lda.w $C000                 ; $83D514
    bpl CODE_83D514             ; $83D517
    dey                         ; $83D519
    beq CODE_83D52E             ; $83D51A
CODE_83D51C:
    ldx.w $8000                 ; $83D51C
    lda.w $8000                 ; $83D51F
    sta.w $0798,x               ; $83D522
    lda.w $8000                 ; $83D525
    sta.w $0796,x               ; $83D528
    dey                         ; $83D52B
    bne CODE_83D51C             ; $83D52C
CODE_83D52E:
    ldx.w $8000                 ; $83D52E
    lda.w $8000                 ; $83D531
    sta.w $0798,x               ; $83D534
    lda.w $8000                 ; $83D537
    sta.w $0796,x               ; $83D53A
    dec.b $22                   ; $83D53D
    ldx.b $16                   ; $83D53F
    lda.l $7E3D6A,x             ; $83D541
    bpl CODE_83D5C0             ; $83D545
    incbin !baserom:$1D547..$1D5C0
CODE_83D5C0:
    lda.b $14                   ; $83D5C0
    clc                         ; $83D5C2
    adc.w #$0080                ; $83D5C3
    sta.b $14                   ; $83D5C6
    cmp.w #$0E00                ; $83D5C8
    bcs CODE_83D5D1             ; $83D5CB
    tax                         ; $83D5CD
    jmp.w $D405                 ; $83D5CE
CODE_83D5D1:
    lda.w #$8000                ; $83D5D1
    sta.w $8000                 ; $83D5D4
    lda.b $24                   ; $83D5D7
    sta.b $90                   ; $83D5D9
    ldx.w #$0023                ; $83D5DB
    lda.w $00B4                 ; $83D5DE
    bne CODE_83D5E6             ; $83D5E1
    ldx.w #$0045                ; $83D5E3
CODE_83D5E6:
    ldy.w #$0000                ; $83D5E6
    lda.w $0242                 ; $83D5E9
    bpl CODE_83D5FB             ; $83D5EC
    incbin !baserom:$1D5EE..$1D5FB
CODE_83D5FB:
    tya                         ; $83D5FB
    sta.l $7E4CFD,x             ; $83D5FC
    lda.b $22                   ; $83D600
    sta.w $0164                 ; $83D602
    ldy.b $18                   ; $83D605
    iny                         ; $83D607
    sty.w $015C                 ; $83D608
    lda.b $1A                   ; $83D60B
    sta.w $1E3E                 ; $83D60D
    plb                         ; $83D610
    rtl                         ; $83D611
mode1_road_project2_view2:
    pea.w $8330                 ; $83D612
    plb                         ; $83D615
    ldy.w #$0023                ; $83D616
    ldx.w #$2D2B                ; $83D619
    lda.w #$019C                ; $83D61C
    sta.b $1A                   ; $83D61F
    lda.w #$0004                ; $83D621
    sta.b $1C                   ; $83D624
    lda.w $00B6                 ; $83D626
    bne CODE_83D63B             ; $83D629
    ldy.w #$0045                ; $83D62B
    ldx.w #$2E2B                ; $83D62E
    lda.w #$033C                ; $83D631
    sta.b $1A                   ; $83D634
    lda.w #$01A4                ; $83D636
    sta.b $1C                   ; $83D639
CODE_83D63B:
    stx.b $18                   ; $83D63B
    lda.w $02D8                 ; $83D63D
    lsr a                       ; $83D640
    and.w #$007E                ; $83D641
    eor.w #$FFFF                ; $83D644
    sec                         ; $83D647
    adc.w #$0080                ; $83D648
    sta.b $14                   ; $83D64B
    lda.w $02D9                 ; $83D64D
    asl a                       ; $83D650
    and.w #$003E                ; $83D651
    sta.b $16                   ; $83D654
    ldx.b $16                   ; $83D656
    lda.l $7E43EC,x             ; $83D658
    sta.l $7E37AC               ; $83D65C
    lda.l $7E436C,x             ; $83D660
    sta.l $7E35AC               ; $83D664
    stz.w $8000                 ; $83D668
    lda.l $7E43EE,x             ; $83D66B
    sec                         ; $83D66F
    sbc.l $7E43EC,x             ; $83D670
    asl a                       ; $83D674
    sta.w $8000                 ; $83D675
    lda.w $02D7                 ; $83D678
    lsr a                       ; $83D67B
    and.w #$FF00                ; $83D67C
    sta.w $8000                 ; $83D67F
    nop                         ; $83D682
    lda.w $8000                 ; $83D683
    lda.w $02F5                 ; $83D686
    clc                         ; $83D689
    adc.w $8000                 ; $83D68A
    clc                         ; $83D68D
    adc.l $7E43EC,x             ; $83D68E
    sta.w $1EFC                 ; $83D692
    stz.w $8000                 ; $83D695
    lda.l $7E406C,x             ; $83D698
    sta.w $8000                 ; $83D69C
    lda.w #$FC00                ; $83D69F
    sta.w $8000                 ; $83D6A2
    nop                         ; $83D6A5
    lda.w $8000                 ; $83D6A6
    clc                         ; $83D6A9
    adc.w #$8000                ; $83D6AA
    lda.w $8000                 ; $83D6AD
    adc.w #$0000                ; $83D6B0
    sta.w $0180                 ; $83D6B3
    stz.w $8000                 ; $83D6B6
    lda.l $7E40EE,x             ; $83D6B9
    sta.w $8000                 ; $83D6BD
    lda.w #$0080                ; $83D6C0
    sec                         ; $83D6C3
    sbc.b $14                   ; $83D6C4
    lsr a                       ; $83D6C6
    xba                         ; $83D6C7
    sta.w $8000                 ; $83D6C8
    nop                         ; $83D6CB
    lda.w $8000                 ; $83D6CC
    sta.b $1E                   ; $83D6CF
    lda.w $8000                 ; $83D6D1
    asl.b $1E                   ; $83D6D4
    rol a                       ; $83D6D6
    asl.b $1E                   ; $83D6D7
    rol a                       ; $83D6D9
    eor.w #$FFFF                ; $83D6DA
    inc a                       ; $83D6DD
    sta.b $1E                   ; $83D6DE
    lda.w $1EDC                 ; $83D6E0
    clc                         ; $83D6E3
    adc.b $1E                   ; $83D6E4
    sta.b $1E                   ; $83D6E6
    stz.w $8000                 ; $83D6E8
    lda.l $7E406E,x             ; $83D6EB
    sta.w $8000                 ; $83D6EF
    lda.w #$0080                ; $83D6F2
    sec                         ; $83D6F5
    sbc.b $14                   ; $83D6F6
    lsr a                       ; $83D6F8
    xba                         ; $83D6F9
    sta.w $8000                 ; $83D6FA
    nop                         ; $83D6FD
    lda.w $8000                 ; $83D6FE
    sta.b $26                   ; $83D701
    lda.w $8000                 ; $83D703
    asl.b $26                   ; $83D706
    rol a                       ; $83D708
    asl.b $26                   ; $83D709
    rol a                       ; $83D70B
    eor.w #$FFFF                ; $83D70C
    inc a                       ; $83D70F
    sta.b $26                   ; $83D710
    stz.w $8000                 ; $83D712
    lda.b $26                   ; $83D715
    sta.w $8000                 ; $83D717
    lda.b $14                   ; $83D71A
    lsr a                       ; $83D71C
    xba                         ; $83D71D
    sta.w $8000                 ; $83D71E
    nop                         ; $83D721
    lda.w $8000                 ; $83D722
    sta.b $2A                   ; $83D725
    lda.w $8000                 ; $83D727
    asl.b $2A                   ; $83D72A
    rol a                       ; $83D72C
    asl.b $2A                   ; $83D72D
    rol a                       ; $83D72F
    sta.b $2A                   ; $83D730
    tyx                         ; $83D732
    lda.w #$0000                ; $83D733
    sta.l $7E4DE1,x             ; $83D736
    lda.w #$0007                ; $83D73A
    bit.b $E6                   ; $83D73D
    bmi CODE_83D749             ; $83D73F
    lda.w #$0010                ; $83D741
    sta.w $8000                 ; $83D744
    lda.b $E6                   ; $83D747
CODE_83D749:
    sta.w $8000                 ; $83D749
    nop                         ; $83D74C
    nop                         ; $83D74D
    lda.w #$0000                ; $83D74E
    sta.w $8000                 ; $83D751
    lda.w $1EFC                 ; $83D754
    bpl CODE_83D766             ; $83D757
    incbin !baserom:$1D759..$1D766
CODE_83D766:
    clc                         ; $83D766
    adc.w #$0033                ; $83D767
    sta.w $8000                 ; $83D76A
    lda.w #$00DC                ; $83D76D
    sta.l $7E4DE1,x             ; $83D770
    dex                         ; $83D774
    lda.w $1EFC                 ; $83D775
    bpl CODE_83D780             ; $83D778
    incbin !baserom:$1D77A..$1D780
CODE_83D780:
    lda.w #$0033                ; $83D780
    sta.w $8000                 ; $83D783
    lda.w #$FFCB                ; $83D786
    sta.w $8000                 ; $83D789
    lda.w #$00BF                ; $83D78C
    sta.w $8000                 ; $83D78F
    lda.w $1EFC                 ; $83D792
    bpl CODE_83D79D             ; $83D795
    incbin !baserom:$1D797..$1D79D
CODE_83D79D:
    lda.w #$00DC                ; $83D79D
    sta.w $8000                 ; $83D7A0
    lda.w $1EFE                 ; $83D7A3
    sta.w $8000                 ; $83D7A6
    lda.w $1F00                 ; $83D7A9
    sta.w $8000                 ; $83D7AC
    lda.w #$0180                ; $83D7AF
    sta.w $8000                 ; $83D7B2
    lda.w $1EFC                 ; $83D7B5
    bpl CODE_83D7C1             ; $83D7B8
    incbin !baserom:$1D7BA..$1D7C1
CODE_83D7C1:
    lda.b $1A                   ; $83D7C1
    sta.w $8000                 ; $83D7C3
    lda.w #$00C0                ; $83D7C6
    sta.w $8000                 ; $83D7C9
    stx.b $22                   ; $83D7CC
    lda.w #$0002                ; $83D7CE
    sta.b $24                   ; $83D7D1
    ldx.b $14                   ; $83D7D3
    lda.l $83ACEC,x             ; $83D7D5
    sta.w $8000                 ; $83D7D9
    sta.b $30                   ; $83D7DC
    ldx.b $24                   ; $83D7DE
    lda.l $7E382C,x             ; $83D7E0
    sta.w $8000                 ; $83D7E4
    ldx.b $16                   ; $83D7E7
    lda.l $7E43EE,x             ; $83D7E9
    sta.w $8000                 ; $83D7ED
    ldx.b $24                   ; $83D7F0
    lda.l $7E362C,x             ; $83D7F2
    sta.w $8000                 ; $83D7F6
    ldx.b $16                   ; $83D7F9
    lda.l $7E436E,x             ; $83D7FB
    sta.w $8000                 ; $83D7FF
    ldx.b $16                   ; $83D802
    lda.l $7E416C,x             ; $83D804
    sta.w $8000                 ; $83D808
    inx                         ; $83D80B
    inx                         ; $83D80C
    stx.b $16                   ; $83D80D
    lda.l $7E43EC,x             ; $83D80F
    clc                         ; $83D813
    ldx.b $24                   ; $83D814
    adc.l $7E372C,x             ; $83D816
    sta.l $7E37AC,x             ; $83D81A
    ldx.b $16                   ; $83D81E
    lda.l $7E436C,x             ; $83D820
    clc                         ; $83D824
    ldx.b $24                   ; $83D825
    adc.l $7E352C,x             ; $83D827
    sta.l $7E35AC,x             ; $83D82B
    ldx.b $24                   ; $83D82F
    lda.w $8000                 ; $83D831
    sta.l $7E36AC,x             ; $83D834
    lda.w $8000                 ; $83D838
    sta.l $7E38AC,x             ; $83D83B
    lda.b $30                   ; $83D83F
    sta.l $7E39AC,x             ; $83D841
    lda.b $14                   ; $83D845
    sta.l $7E3AAC,x             ; $83D847
    inx                         ; $83D84B
    inx                         ; $83D84C
    stx.b $24                   ; $83D84D
CODE_83D84F:
    lda.w $C000                 ; $83D84F
    bpl CODE_83D84F             ; $83D852
    lda.w $8000                 ; $83D854
    bne CODE_83D85C             ; $83D857
    jmp.w $D9B8                 ; $83D859
CODE_83D85C:
    tay                         ; $83D85C
    ldx.b $22                   ; $83D85D
    sep.b #$20                  ; $83D85F
    eor.b #$FF                  ; $83D861
    sec                         ; $83D863
    adc.l $7E4DE2,x             ; $83D864
    sta.l $7E4DE1,x             ; $83D868
    rep.b #$20                  ; $83D86C
    phy                         ; $83D86E
    lda.b $E6                   ; $83D86F
    bpl CODE_83D876             ; $83D871
    jmp.w $D969                 ; $83D873
CODE_83D876:
    ldx.b $14                   ; $83D876
    lda.l $83ACEC,x             ; $83D878
    sta.w $8000                 ; $83D87C
    ldx.b $16                   ; $83D87F
    lda.l $7E456C,x             ; $83D881
    sec                         ; $83D885
    sbc.w #$0007                ; $83D886
    tax                         ; $83D889
    bpl CODE_83D8FB             ; $83D88A
    txy                         ; $83D88C
    lda.l $900000,x             ; $83D88D
    sta.w $8000                 ; $83D891
    lda.b $18                   ; $83D894
    sec                         ; $83D896
    sbc.w #$0008                ; $83D897
    sta.b $18                   ; $83D89A
    tax                         ; $83D89C
    lda.w $8000                 ; $83D89D
    sta.l $7E0001,x             ; $83D8A0
    ldx.b $14                   ; $83D8A4
    lda.l $7E7C73,x             ; $83D8A6
    sta.w $8000                 ; $83D8AA
    tyx                         ; $83D8AD
    lda.l $900002,x             ; $83D8AE
    sta.w $8000                 ; $83D8B2
    nop                         ; $83D8B5
    nop                         ; $83D8B6
    ldx.b $18                   ; $83D8B7
    lda.w $8000                 ; $83D8B9
    sta.l $7E0003,x             ; $83D8BC
    ldx.b $14                   ; $83D8C0
    lda.l $7E7C73,x             ; $83D8C2
    sta.w $8000                 ; $83D8C6
    tyx                         ; $83D8C9
    lda.l $900004,x             ; $83D8CA
    sta.w $8000                 ; $83D8CE
    nop                         ; $83D8D1
    nop                         ; $83D8D2
    ldx.b $18                   ; $83D8D3
    lda.w $8000                 ; $83D8D5
    sta.l $7E0005,x             ; $83D8D8
    ldx.b $14                   ; $83D8DC
    lda.l $7E7C73,x             ; $83D8DE
    sta.w $8000                 ; $83D8E2
    tyx                         ; $83D8E5
    lda.l $900006,x             ; $83D8E6
    sta.w $8000                 ; $83D8EA
    nop                         ; $83D8ED
    nop                         ; $83D8EE
    ldx.b $18                   ; $83D8EF
    lda.w $8000                 ; $83D8F1
    sta.l $7E0007,x             ; $83D8F4
    jmp.w $D98C                 ; $83D8F8
CODE_83D8FB:
    txy                         ; $83D8FB
    lda.l $7E0000,x             ; $83D8FC
    sta.w $8000                 ; $83D900
    lda.b $18                   ; $83D903
    sec                         ; $83D905
    sbc.w #$0008                ; $83D906
    sta.b $18                   ; $83D909
    tax                         ; $83D90B
    lda.w $8000                 ; $83D90C
    sta.l $7E0001,x             ; $83D90F
    ldx.b $14                   ; $83D913
    lda.l $7E7C73,x             ; $83D915
    sta.w $8000                 ; $83D919
    tyx                         ; $83D91C
    lda.l $7E0002,x             ; $83D91D
    sta.w $8000                 ; $83D921
    nop                         ; $83D924
    nop                         ; $83D925
    ldx.b $18                   ; $83D926
    lda.w $8000                 ; $83D928
    sta.l $7E0003,x             ; $83D92B
    ldx.b $14                   ; $83D92F
    lda.l $7E7C73,x             ; $83D931
    sta.w $8000                 ; $83D935
    tyx                         ; $83D938
    lda.l $7E0004,x             ; $83D939
    sta.w $8000                 ; $83D93D
    nop                         ; $83D940
    nop                         ; $83D941
    ldx.b $18                   ; $83D942
    lda.w $8000                 ; $83D944
    sta.l $7E0005,x             ; $83D947
    ldx.b $14                   ; $83D94B
    lda.l $7E7C73,x             ; $83D94D
    sta.w $8000                 ; $83D951
    tyx                         ; $83D954
    lda.l $7E0006,x             ; $83D955
    sta.w $8000                 ; $83D959
    nop                         ; $83D95C
    nop                         ; $83D95D
    ldx.b $18                   ; $83D95E
    lda.w $8000                 ; $83D960
    sta.l $7E0007,x             ; $83D963
    bra CODE_83D98C             ; $83D967
    ldx.b $16                   ; $83D969
    lda.l $7E456C,x             ; $83D96B
    tax                         ; $83D96F
    bmi CODE_83D980             ; $83D970
    phb                         ; $83D972
    ldy.b $18                   ; $83D973
    lda.w #$0007                ; $83D975
    mvp $7E,$7E                 ; $83D978
    sty.b $18                   ; $83D97B
    plb                         ; $83D97D
    bra CODE_83D98C             ; $83D97E
CODE_83D980:
    phb                         ; $83D980
    ldy.b $18                   ; $83D981
    lda.w #$0007                ; $83D983
    mvp $7E,$90                 ; $83D986
    sty.b $18                   ; $83D989
    plb                         ; $83D98B
CODE_83D98C:
    ply                         ; $83D98C
CODE_83D98D:
    lda.w $C000                 ; $83D98D
    bpl CODE_83D98D             ; $83D990
    dey                         ; $83D992
    beq CODE_83D9A7             ; $83D993
CODE_83D995:
    ldx.w $8000                 ; $83D995
    lda.w $8000                 ; $83D998
    sta.w $11B8,x               ; $83D99B
    lda.w $8000                 ; $83D99E
    sta.w $11B6,x               ; $83D9A1
    dey                         ; $83D9A4
    bne CODE_83D995             ; $83D9A5
CODE_83D9A7:
    ldx.w $8000                 ; $83D9A7
    lda.w $8000                 ; $83D9AA
    sta.w $11B8,x               ; $83D9AD
    lda.w $8000                 ; $83D9B0
    sta.w $11B6,x               ; $83D9B3
    dec.b $22                   ; $83D9B6
    lda.b $14                   ; $83D9B8
    clc                         ; $83D9BA
    adc.w #$0080                ; $83D9BB
    sta.b $14                   ; $83D9BE
    cmp.w #$0E00                ; $83D9C0
    bcs CODE_83D9C9             ; $83D9C3
    tax                         ; $83D9C5
    jmp.w $D7D5                 ; $83D9C6
CODE_83D9C9:
    lda.w #$8000                ; $83D9C9
    sta.w $8000                 ; $83D9CC
    lda.b $24                   ; $83D9CF
    sta.b $96                   ; $83D9D1
    ldx.w #$0023                ; $83D9D3
    lda.w $00B6                 ; $83D9D6
    bne CODE_83D9DE             ; $83D9D9
    ldx.w #$0045                ; $83D9DB
CODE_83D9DE:
    ldy.w #$0000                ; $83D9DE
    lda.w $1EFC                 ; $83D9E1
    bpl CODE_83D9F3             ; $83D9E4
    incbin !baserom:$1D9E6..$1D9F3
CODE_83D9F3:
    tya                         ; $83D9F3
    sta.l $7E4DE1,x             ; $83D9F4
    lda.b $22                   ; $83D9F8
    sta.w $016A                 ; $83D9FA
    ldy.b $18                   ; $83D9FD
    iny                         ; $83D9FF
    sty.w $0162                 ; $83DA00
    lda.b $1A                   ; $83DA03
    sta.w $1E44                 ; $83DA05
    plb                         ; $83DA08
    rtl                         ; $83DA09
mode1_road_project2_view1:
    pea.w $8330                 ; $83DA0A
    plb                         ; $83DA0D
    ldy.w #$0023                ; $83DA0E
    ldx.w #$292B                ; $83DA11
    lda.w #$019C                ; $83DA14
    sta.b $1A                   ; $83DA17
    lda.w #$0004                ; $83DA19
    sta.b $1C                   ; $83DA1C
    lda.w $00B4                 ; $83DA1E
    bne CODE_83DA33             ; $83DA21
    ldy.w #$0045                ; $83DA23
    ldx.w #$2A2B                ; $83DA26
    lda.w #$033C                ; $83DA29
    sta.b $1A                   ; $83DA2C
    lda.w #$01A4                ; $83DA2E
    sta.b $1C                   ; $83DA31
CODE_83DA33:
    stx.b $18                   ; $83DA33
    lda.w $0225                 ; $83DA35
    lsr a                       ; $83DA38
    and.w #$007E                ; $83DA39
    eor.w #$FFFF                ; $83DA3C
    sec                         ; $83DA3F
    adc.w #$0080                ; $83DA40
    sta.b $14                   ; $83DA43
    lda.w $0226                 ; $83DA45
    asl a                       ; $83DA48
    and.w #$003E                ; $83DA49
    sta.b $16                   ; $83DA4C
    ldx.b $16                   ; $83DA4E
    lda.l $7E3E6C,x             ; $83DA50
    sta.l $7E376C               ; $83DA54
    lda.l $7E3DEC,x             ; $83DA58
    sta.l $7E356C               ; $83DA5C
    stz.w $8000                 ; $83DA60
    lda.l $7E3E6E,x             ; $83DA63
    sec                         ; $83DA67
    sbc.l $7E3E6C,x             ; $83DA68
    asl a                       ; $83DA6C
    sta.w $8000                 ; $83DA6D
    lda.w $0224                 ; $83DA70
    lsr a                       ; $83DA73
    and.w #$FF00                ; $83DA74
    sta.w $8000                 ; $83DA77
    nop                         ; $83DA7A
    lda.w $8000                 ; $83DA7B
    lda.w $0242                 ; $83DA7E
    clc                         ; $83DA81
    adc.w $8000                 ; $83DA82
    clc                         ; $83DA85
    adc.l $7E3E6C,x             ; $83DA86
    sta.w $1EF2                 ; $83DA8A
    stz.w $8000                 ; $83DA8D
    lda.l $7E3AEC,x             ; $83DA90
    sta.w $8000                 ; $83DA94
    lda.w #$FC00                ; $83DA97
    sta.w $8000                 ; $83DA9A
    nop                         ; $83DA9D
    lda.w $8000                 ; $83DA9E
    clc                         ; $83DAA1
    adc.w #$8000                ; $83DAA2
    lda.w $8000                 ; $83DAA5
    adc.w #$0000                ; $83DAA8
    sta.w $017C                 ; $83DAAB
    stz.w $8000                 ; $83DAAE
    lda.l $7E3B6E,x             ; $83DAB1
    sta.w $8000                 ; $83DAB5
    lda.w #$0080                ; $83DAB8
    sec                         ; $83DABB
    sbc.b $14                   ; $83DABC
    lsr a                       ; $83DABE
    xba                         ; $83DABF
    sta.w $8000                 ; $83DAC0
    nop                         ; $83DAC3
    lda.w $8000                 ; $83DAC4
    sta.b $1E                   ; $83DAC7
    lda.w $8000                 ; $83DAC9
    asl.b $1E                   ; $83DACC
    rol a                       ; $83DACE
    asl.b $1E                   ; $83DACF
    rol a                       ; $83DAD1
    eor.w #$FFFF                ; $83DAD2
    inc a                       ; $83DAD5
    sta.b $1E                   ; $83DAD6
    lda.w $1EDA                 ; $83DAD8
    clc                         ; $83DADB
    adc.b $1E                   ; $83DADC
    sta.b $1E                   ; $83DADE
    stz.w $8000                 ; $83DAE0
    lda.l $7E3AEE,x             ; $83DAE3
    sta.w $8000                 ; $83DAE7
    lda.w #$0080                ; $83DAEA
    sec                         ; $83DAED
    sbc.b $14                   ; $83DAEE
    lsr a                       ; $83DAF0
    xba                         ; $83DAF1
    sta.w $8000                 ; $83DAF2
    nop                         ; $83DAF5
    lda.w $8000                 ; $83DAF6
    sta.b $26                   ; $83DAF9
    lda.w $8000                 ; $83DAFB
    asl.b $26                   ; $83DAFE
    rol a                       ; $83DB00
    asl.b $26                   ; $83DB01
    rol a                       ; $83DB03
    eor.w #$FFFF                ; $83DB04
    inc a                       ; $83DB07
    sta.b $26                   ; $83DB08
    stz.w $8000                 ; $83DB0A
    lda.b $26                   ; $83DB0D
    sta.w $8000                 ; $83DB0F
    lda.b $14                   ; $83DB12
    lsr a                       ; $83DB14
    xba                         ; $83DB15
    sta.w $8000                 ; $83DB16
    nop                         ; $83DB19
    lda.w $8000                 ; $83DB1A
    sta.b $2A                   ; $83DB1D
    lda.w $8000                 ; $83DB1F
    asl.b $2A                   ; $83DB22
    rol a                       ; $83DB24
    asl.b $2A                   ; $83DB25
    rol a                       ; $83DB27
    sta.b $2A                   ; $83DB28
    tyx                         ; $83DB2A
    lda.w #$0000                ; $83DB2B
    sta.l $7E4D49,x             ; $83DB2E
    lda.w #$0007                ; $83DB32
    bit.b $E4                   ; $83DB35
    bmi CODE_83DB41             ; $83DB37
    lda.w #$0010                ; $83DB39
    sta.w $8000                 ; $83DB3C
    lda.b $E4                   ; $83DB3F
CODE_83DB41:
    sta.w $8000                 ; $83DB41
    nop                         ; $83DB44
    nop                         ; $83DB45
    lda.w #$0000                ; $83DB46
    sta.w $8000                 ; $83DB49
    lda.w $1EF2                 ; $83DB4C
    bpl CODE_83DB5E             ; $83DB4F
    incbin !baserom:$1DB51..$1DB5E
CODE_83DB5E:
    clc                         ; $83DB5E
    adc.w #$0033                ; $83DB5F
    sta.w $8000                 ; $83DB62
    lda.w #$006C                ; $83DB65
    sta.l $7E4D49,x             ; $83DB68
    dex                         ; $83DB6C
    lda.w $1EF2                 ; $83DB6D
    bpl CODE_83DB78             ; $83DB70
    incbin !baserom:$1DB72..$1DB78
CODE_83DB78:
    lda.w #$0033                ; $83DB78
    sta.w $8000                 ; $83DB7B
    lda.w #$FFCB                ; $83DB7E
    sta.w $8000                 ; $83DB81
    lda.w #$00BF                ; $83DB84
    sta.w $8000                 ; $83DB87
    lda.w $1EF2                 ; $83DB8A
    bpl CODE_83DB95             ; $83DB8D
    incbin !baserom:$1DB8F..$1DB95
CODE_83DB95:
    lda.w #$006C                ; $83DB95
    sta.w $8000                 ; $83DB98
    lda.w $1EF4                 ; $83DB9B
    sta.w $8000                 ; $83DB9E
    lda.w $1EF6                 ; $83DBA1
    sta.w $8000                 ; $83DBA4
    lda.w #$0180                ; $83DBA7
    sta.w $8000                 ; $83DBAA
    lda.w $1EF2                 ; $83DBAD
    bpl CODE_83DBB9             ; $83DBB0
    incbin !baserom:$1DBB2..$1DBB9
CODE_83DBB9:
    lda.b $1A                   ; $83DBB9
    sta.w $8000                 ; $83DBBB
    lda.w #$00C0                ; $83DBBE
    sta.w $8000                 ; $83DBC1
    stx.b $22                   ; $83DBC4
    lda.w #$0002                ; $83DBC6
    sta.b $24                   ; $83DBC9
    ldx.b $14                   ; $83DBCB
    lda.l $83ACEC,x             ; $83DBCD
    sta.w $8000                 ; $83DBD1
    sta.b $30                   ; $83DBD4
    ldx.b $24                   ; $83DBD6
    lda.l $7E37EC,x             ; $83DBD8
    sta.w $8000                 ; $83DBDC
    ldx.b $16                   ; $83DBDF
    lda.l $7E3E6E,x             ; $83DBE1
    sta.w $8000                 ; $83DBE5
    ldx.b $24                   ; $83DBE8
    lda.l $7E35EC,x             ; $83DBEA
    sta.w $8000                 ; $83DBEE
    ldx.b $16                   ; $83DBF1
    lda.l $7E3DEE,x             ; $83DBF3
    sta.w $8000                 ; $83DBF7
    ldx.b $16                   ; $83DBFA
    lda.l $7E3BEC,x             ; $83DBFC
    sta.w $8000                 ; $83DC00
    inx                         ; $83DC03
    inx                         ; $83DC04
    stx.b $16                   ; $83DC05
    lda.l $7E3E6C,x             ; $83DC07
    clc                         ; $83DC0B
    ldx.b $24                   ; $83DC0C
    adc.l $7E36EC,x             ; $83DC0E
    sta.l $7E376C,x             ; $83DC12
    ldx.b $16                   ; $83DC16
    lda.l $7E3DEC,x             ; $83DC18
    clc                         ; $83DC1C
    ldx.b $24                   ; $83DC1D
    adc.l $7E34EC,x             ; $83DC1F
    sta.l $7E356C,x             ; $83DC23
    ldx.b $24                   ; $83DC27
    lda.w $8000                 ; $83DC29
    sta.l $7E366C,x             ; $83DC2C
    lda.w $8000                 ; $83DC30
    sta.l $7E386C,x             ; $83DC33
    lda.b $30                   ; $83DC37
    sta.l $7E396C,x             ; $83DC39
    lda.b $14                   ; $83DC3D
    sta.l $7E3A6C,x             ; $83DC3F
    inx                         ; $83DC43
    inx                         ; $83DC44
    stx.b $24                   ; $83DC45
CODE_83DC47:
    lda.w $C000                 ; $83DC47
    bpl CODE_83DC47             ; $83DC4A
    lda.w $8000                 ; $83DC4C
    bne CODE_83DC54             ; $83DC4F
    jmp.w $DDB0                 ; $83DC51
CODE_83DC54:
    tay                         ; $83DC54
    ldx.b $22                   ; $83DC55
    sep.b #$20                  ; $83DC57
    eor.b #$FF                  ; $83DC59
    sec                         ; $83DC5B
    adc.l $7E4D4A,x             ; $83DC5C
    sta.l $7E4D49,x             ; $83DC60
    rep.b #$20                  ; $83DC64
    phy                         ; $83DC66
    lda.b $E4                   ; $83DC67
    bpl CODE_83DC6E             ; $83DC69
    jmp.w $DD61                 ; $83DC6B
CODE_83DC6E:
    ldx.b $14                   ; $83DC6E
    lda.l $83ACEC,x             ; $83DC70
    sta.w $8000                 ; $83DC74
    ldx.b $16                   ; $83DC77
    lda.l $7E3FEC,x             ; $83DC79
    sec                         ; $83DC7D
    sbc.w #$0007                ; $83DC7E
    tax                         ; $83DC81
    bpl CODE_83DCF3             ; $83DC82
    txy                         ; $83DC84
    lda.l $900000,x             ; $83DC85
    sta.w $8000                 ; $83DC89
    lda.b $18                   ; $83DC8C
    sec                         ; $83DC8E
    sbc.w #$0008                ; $83DC8F
    sta.b $18                   ; $83DC92
    tax                         ; $83DC94
    lda.w $8000                 ; $83DC95
    sta.l $7E0001,x             ; $83DC98
    ldx.b $14                   ; $83DC9C
    lda.l $7E6C33,x             ; $83DC9E
    sta.w $8000                 ; $83DCA2
    tyx                         ; $83DCA5
    lda.l $900002,x             ; $83DCA6
    sta.w $8000                 ; $83DCAA
    nop                         ; $83DCAD
    nop                         ; $83DCAE
    ldx.b $18                   ; $83DCAF
    lda.w $8000                 ; $83DCB1
    sta.l $7E0003,x             ; $83DCB4
    ldx.b $14                   ; $83DCB8
    lda.l $7E6C33,x             ; $83DCBA
    sta.w $8000                 ; $83DCBE
    tyx                         ; $83DCC1
    lda.l $900004,x             ; $83DCC2
    sta.w $8000                 ; $83DCC6
    nop                         ; $83DCC9
    nop                         ; $83DCCA
    ldx.b $18                   ; $83DCCB
    lda.w $8000                 ; $83DCCD
    sta.l $7E0005,x             ; $83DCD0
    ldx.b $14                   ; $83DCD4
    lda.l $7E6C33,x             ; $83DCD6
    sta.w $8000                 ; $83DCDA
    tyx                         ; $83DCDD
    lda.l $900006,x             ; $83DCDE
    sta.w $8000                 ; $83DCE2
    nop                         ; $83DCE5
    nop                         ; $83DCE6
    ldx.b $18                   ; $83DCE7
    lda.w $8000                 ; $83DCE9
    sta.l $7E0007,x             ; $83DCEC
    jmp.w $DD84                 ; $83DCF0
CODE_83DCF3:
    txy                         ; $83DCF3
    lda.l $7E0000,x             ; $83DCF4
    sta.w $8000                 ; $83DCF8
    lda.b $18                   ; $83DCFB
    sec                         ; $83DCFD
    sbc.w #$0008                ; $83DCFE
    sta.b $18                   ; $83DD01
    tax                         ; $83DD03
    lda.w $8000                 ; $83DD04
    sta.l $7E0001,x             ; $83DD07
    ldx.b $14                   ; $83DD0B
    lda.l $7E6C33,x             ; $83DD0D
    sta.w $8000                 ; $83DD11
    tyx                         ; $83DD14
    lda.l $7E0002,x             ; $83DD15
    sta.w $8000                 ; $83DD19
    nop                         ; $83DD1C
    nop                         ; $83DD1D
    ldx.b $18                   ; $83DD1E
    lda.w $8000                 ; $83DD20
    sta.l $7E0003,x             ; $83DD23
    ldx.b $14                   ; $83DD27
    lda.l $7E6C33,x             ; $83DD29
    sta.w $8000                 ; $83DD2D
    tyx                         ; $83DD30
    lda.l $7E0004,x             ; $83DD31
    sta.w $8000                 ; $83DD35
    nop                         ; $83DD38
    nop                         ; $83DD39
    ldx.b $18                   ; $83DD3A
    lda.w $8000                 ; $83DD3C
    sta.l $7E0005,x             ; $83DD3F
    ldx.b $14                   ; $83DD43
    lda.l $7E6C33,x             ; $83DD45
    sta.w $8000                 ; $83DD49
    tyx                         ; $83DD4C
    lda.l $7E0006,x             ; $83DD4D
    sta.w $8000                 ; $83DD51
    nop                         ; $83DD54
    nop                         ; $83DD55
    ldx.b $18                   ; $83DD56
    lda.w $8000                 ; $83DD58
    sta.l $7E0007,x             ; $83DD5B
    bra CODE_83DD84             ; $83DD5F
    ldx.b $16                   ; $83DD61
    lda.l $7E3FEC,x             ; $83DD63
    tax                         ; $83DD67
    bmi CODE_83DD78             ; $83DD68
    phb                         ; $83DD6A
    ldy.b $18                   ; $83DD6B
    lda.w #$0007                ; $83DD6D
    mvp $7E,$7E                 ; $83DD70
    sty.b $18                   ; $83DD73
    plb                         ; $83DD75
    bra CODE_83DD84             ; $83DD76
CODE_83DD78:
    phb                         ; $83DD78
    ldy.b $18                   ; $83DD79
    lda.w #$0007                ; $83DD7B
    mvp $7E,$90                 ; $83DD7E
    sty.b $18                   ; $83DD81
    plb                         ; $83DD83
CODE_83DD84:
    ply                         ; $83DD84
CODE_83DD85:
    lda.w $C000                 ; $83DD85
    bpl CODE_83DD85             ; $83DD88
    dey                         ; $83DD8A
    beq CODE_83DD9F             ; $83DD8B
CODE_83DD8D:
    ldx.w $8000                 ; $83DD8D
    lda.w $8000                 ; $83DD90
    sta.w $0E58,x               ; $83DD93
    lda.w $8000                 ; $83DD96
    sta.w $0E56,x               ; $83DD99
    dey                         ; $83DD9C
    bne CODE_83DD8D             ; $83DD9D
CODE_83DD9F:
    ldx.w $8000                 ; $83DD9F
    lda.w $8000                 ; $83DDA2
    sta.w $0E58,x               ; $83DDA5
    lda.w $8000                 ; $83DDA8
    sta.w $0E56,x               ; $83DDAB
    dec.b $22                   ; $83DDAE
    lda.b $14                   ; $83DDB0
    clc                         ; $83DDB2
    adc.w #$0080                ; $83DDB3
    sta.b $14                   ; $83DDB6
    cmp.w #$0E00                ; $83DDB8
    bcs CODE_83DDC1             ; $83DDBB
    tax                         ; $83DDBD
    jmp.w $DBCD                 ; $83DDBE
CODE_83DDC1:
    lda.w #$8000                ; $83DDC1
    sta.w $8000                 ; $83DDC4
    lda.b $24                   ; $83DDC7
    sta.b $94                   ; $83DDC9
    ldx.w #$0023                ; $83DDCB
    lda.w $00B4                 ; $83DDCE
    bne CODE_83DDD6             ; $83DDD1
    ldx.w #$0045                ; $83DDD3
CODE_83DDD6:
    ldy.w #$0000                ; $83DDD6
    lda.w $1EF2                 ; $83DDD9
    bpl CODE_83DDEB             ; $83DDDC
    incbin !baserom:$1DDDE..$1DDEB
CODE_83DDEB:
    tya                         ; $83DDEB
    sta.l $7E4D49,x             ; $83DDEC
    lda.b $22                   ; $83DDF0
    sta.w $0168                 ; $83DDF2
    ldy.b $18                   ; $83DDF5
    iny                         ; $83DDF7
    sty.w $0160                 ; $83DDF8
    lda.b $1A                   ; $83DDFB
    sta.w $1E40                 ; $83DDFD
    plb                         ; $83DE00
    rtl                         ; $83DE01
mode1_road_lists:
    ldy.w $00E8                 ; $83DE02
    lda.w $00E6                 ; $83DE05
    sta.w $00E8                 ; $83DE08
    lda.w $00E4                 ; $83DE0B
    sta.w $00E6                 ; $83DE0E
    lda.w $00E0                 ; $83DE11
    sta.w $00E4                 ; $83DE14
    sty.w $00E0                 ; $83DE17
    ldy.w $1E3A                 ; $83DE1A
    lda.w $1E36                 ; $83DE1D
    sta.w $1E3A                 ; $83DE20
    lda.w $1E32                 ; $83DE23
    sta.w $1E36                 ; $83DE26
    lda.w $1E2E                 ; $83DE29
    sta.w $1E32                 ; $83DE2C
    sty.w $1E2E                 ; $83DE2F
    sty.w $00D0                 ; $83DE32
    ldy.w $1E3C                 ; $83DE35
    lda.w $1E38                 ; $83DE38
    sta.w $1E3C                 ; $83DE3B
    lda.w $1E34                 ; $83DE3E
    sta.w $1E38                 ; $83DE41
    lda.w $1E30                 ; $83DE44
    sta.w $1E34                 ; $83DE47
    sty.w $1E30                 ; $83DE4A
    sty.w $00D2                 ; $83DE4D
    ldy.w $00D0                 ; $83DE50
    lda.w #$0000                ; $83DE53
    sta.w $0000,y               ; $83DE56
    dey                         ; $83DE59
    dey                         ; $83DE5A
    dey                         ; $83DE5B
    lda.w #$0068                ; $83DE5C
    sta.b $14                   ; $83DE5F
    lda.w $02F5                 ; $83DE61
    bpl CODE_83DE86             ; $83DE64
    incbin !baserom:$1DE66..$1DE86
CODE_83DE86:
    lda.b $14                   ; $83DE86
    sta.b $16                   ; $83DE88
    ldx.w $0166                 ; $83DE8A
    lda.l $7E4D96,x             ; $83DE8D
    and.w #$00FF                ; $83DE91
    bne CODE_83DE99             ; $83DE94
    incbin !baserom:$1DE96..$1DE99
CODE_83DE99:
    sec                         ; $83DE99
    sbc.w #$0074                ; $83DE9A
    bpl CODE_83DEA2             ; $83DE9D
    incbin !baserom:$1DE9F..$1DEA2
CODE_83DEA2:
    cmp.b $14                   ; $83DEA2
    bpl CODE_83DECF             ; $83DEA4
    sta.b $14                   ; $83DEA6
    eor.w #$FFFF                ; $83DEA8
    sec                         ; $83DEAB
    adc.b $16                   ; $83DEAC
    beq CODE_83DECF             ; $83DEAE
    ora.w #$0080                ; $83DEB0
    sta.w $0000,y               ; $83DEB3
    lda.w #$0067                ; $83DEB6
    sec                         ; $83DEB9
    sbc.b $14                   ; $83DEBA
    asl a                       ; $83DEBC
    asl a                       ; $83DEBD
    eor.w #$FFFF                ; $83DEBE
    sec                         ; $83DEC1
    adc.w $1E42                 ; $83DEC2
    clc                         ; $83DEC5
    adc.w #$0AF6                ; $83DEC6
    sta.w $0001,y               ; $83DEC9
    dey                         ; $83DECC
    dey                         ; $83DECD
    dey                         ; $83DECE
CODE_83DECF:
    lda.w $02EA                 ; $83DECF
    clc                         ; $83DED2
    adc.w #$0040                ; $83DED3
    bpl CODE_83DEDB             ; $83DED6
    incbin !baserom:$1DED8..$1DEDB
CODE_83DEDB:
    sta.b $16                   ; $83DEDB
    sec                         ; $83DEDD
    sbc.b $14                   ; $83DEDE
    bpl CODE_83DEFC             ; $83DEE0
    eor.w #$FFFF                ; $83DEE2
    inc a                       ; $83DEE5
    ora.w #$0080                ; $83DEE6
    sta.w $0000,y               ; $83DEE9
    lda.b $16                   ; $83DEEC
    sta.b $14                   ; $83DEEE
    asl a                       ; $83DEF0
    asl a                       ; $83DEF1
    clc                         ; $83DEF2
    adc.w #$AB28                ; $83DEF3
    sta.w $0001,y               ; $83DEF6
    dey                         ; $83DEF9
    dey                         ; $83DEFA
    dey                         ; $83DEFB
CODE_83DEFC:
    lda.w $02EA                 ; $83DEFC
    bpl CODE_83DF04             ; $83DEFF
    lda.w #$0000                ; $83DF01
CODE_83DF04:
    sta.b $16                   ; $83DF04
    sec                         ; $83DF06
    sbc.b $14                   ; $83DF07
    bpl CODE_83DF23             ; $83DF09
    eor.w #$FFFF                ; $83DF0B
    inc a                       ; $83DF0E
    sta.w $0000,y               ; $83DF0F
    lda.w $00E0                 ; $83DF12
    clc                         ; $83DF15
    adc.w #$1546                ; $83DF16
    sta.w $0001,y               ; $83DF19
    dey                         ; $83DF1C
    dey                         ; $83DF1D
    dey                         ; $83DF1E
    lda.b $16                   ; $83DF1F
    sta.b $14                   ; $83DF21
CODE_83DF23:
    lda.b $14                   ; $83DF23
    beq CODE_83DF36             ; $83DF25
    ora.w #$0080                ; $83DF27
    sta.w $0000,y               ; $83DF2A
    lda.w #$A79C                ; $83DF2D
    sta.w $0001,y               ; $83DF30
    dey                         ; $83DF33
    dey                         ; $83DF34
    dey                         ; $83DF35
CODE_83DF36:
    lda.w #$0000                ; $83DF36
    sta.w $0000,y               ; $83DF39
    sty.w $00D0                 ; $83DF3C
    ldy.w $00E0                 ; $83DF3F
    lda.w #$004B                ; $83DF42
    sec                         ; $83DF45
    sbc.w $02EA                 ; $83DF46
    sta.w $1548,y               ; $83DF49
    ldy.w $00D0                 ; $83DF4C
    lda.w #$0008                ; $83DF4F
    sta.w $0000,y               ; $83DF52
    dey                         ; $83DF55
    dey                         ; $83DF56
    dey                         ; $83DF57
    lda.w #$0068                ; $83DF58
    sta.b $14                   ; $83DF5B
    lda.w $0242                 ; $83DF5D
    bpl CODE_83DF82             ; $83DF60
    incbin !baserom:$1DF62..$1DF82
CODE_83DF82:
    lda.b $14                   ; $83DF82
    sta.b $16                   ; $83DF84
    ldx.w $0164                 ; $83DF86
    lda.l $7E4CFE,x             ; $83DF89
    and.w #$00FF                ; $83DF8D
    bne CODE_83DF95             ; $83DF90
    incbin !baserom:$1DF92..$1DF95
CODE_83DF95:
    sec                         ; $83DF95
    sbc.w #$0004                ; $83DF96
    bpl CODE_83DF9E             ; $83DF99
    incbin !baserom:$1DF9B..$1DF9E
CODE_83DF9E:
    cmp.b $14                   ; $83DF9E
    bpl CODE_83DFCB             ; $83DFA0
    sta.b $14                   ; $83DFA2
    eor.w #$FFFF                ; $83DFA4
    sec                         ; $83DFA7
    adc.b $16                   ; $83DFA8
    beq CODE_83DFCB             ; $83DFAA
    ora.w #$0080                ; $83DFAC
    sta.w $0000,y               ; $83DFAF
    lda.w #$0067                ; $83DFB2
    sec                         ; $83DFB5
    sbc.b $14                   ; $83DFB6
    asl a                       ; $83DFB8
    asl a                       ; $83DFB9
    eor.w #$FFFF                ; $83DFBA
    sec                         ; $83DFBD
    adc.w $1E3E                 ; $83DFBE
    clc                         ; $83DFC1
    adc.w #$0796                ; $83DFC2
    sta.w $0001,y               ; $83DFC5
    dey                         ; $83DFC8
    dey                         ; $83DFC9
    dey                         ; $83DFCA
CODE_83DFCB:
    lda.w $0237                 ; $83DFCB
    clc                         ; $83DFCE
    adc.w #$0040                ; $83DFCF
    bpl CODE_83DFD7             ; $83DFD2
    incbin !baserom:$1DFD4..$1DFD7
CODE_83DFD7:
    sta.b $16                   ; $83DFD7
    sec                         ; $83DFD9
    sbc.b $14                   ; $83DFDA
    bpl CODE_83DFF8             ; $83DFDC
    eor.w #$FFFF                ; $83DFDE
    inc a                       ; $83DFE1
    ora.w #$0080                ; $83DFE2
    sta.w $0000,y               ; $83DFE5
    lda.b $16                   ; $83DFE8
    sta.b $14                   ; $83DFEA
    asl a                       ; $83DFEC
    asl a                       ; $83DFED
    clc                         ; $83DFEE
    adc.w #$A968                ; $83DFEF
    sta.w $0001,y               ; $83DFF2
    dey                         ; $83DFF5
    dey                         ; $83DFF6
    dey                         ; $83DFF7
CODE_83DFF8:
    lda.w $0237                 ; $83DFF8
    bpl CODE_83E000             ; $83DFFB
    lda.w #$0000                ; $83DFFD
CODE_83E000:
    sta.b $16                   ; $83E000
    sec                         ; $83E002
    sbc.b $14                   ; $83E003
    bpl CODE_83E01F             ; $83E005
    eor.w #$FFFF                ; $83E007
    inc a                       ; $83E00A
    sta.w $0000,y               ; $83E00B
    lda.w $00E0                 ; $83E00E
    clc                         ; $83E011
    adc.w #$1536                ; $83E012
    sta.w $0001,y               ; $83E015
    dey                         ; $83E018
    dey                         ; $83E019
    dey                         ; $83E01A
    lda.b $16                   ; $83E01B
    sta.b $14                   ; $83E01D
CODE_83E01F:
    lda.b $14                   ; $83E01F
    beq CODE_83E032             ; $83E021
    ora.w #$0080                ; $83E023
    sta.w $0000,y               ; $83E026
    lda.w #$A5DC                ; $83E029
    sta.w $0001,y               ; $83E02C
    dey                         ; $83E02F
    dey                         ; $83E030
    dey                         ; $83E031
CODE_83E032:
    lda.w #$0004                ; $83E032
    sta.w $0000,y               ; $83E035
    sty.w $00D0                 ; $83E038
    ldy.w $00E0                 ; $83E03B
    lda.w #$00BB                ; $83E03E
    sec                         ; $83E041
    sbc.w $0237                 ; $83E042
    sta.w $1538,y               ; $83E045
    ldy.w $00D2                 ; $83E048
    lda.w #$0008                ; $83E04B
    sta.w $0000,y               ; $83E04E
    dey                         ; $83E051
    dey                         ; $83E052
    dey                         ; $83E053
    lda.w #$0068                ; $83E054
    sta.b $14                   ; $83E057
    lda.w $1EFC                 ; $83E059
    bpl CODE_83E07E             ; $83E05C
    incbin !baserom:$1E05E..$1E07E
CODE_83E07E:
    lda.b $14                   ; $83E07E
    sta.b $16                   ; $83E080
    ldx.w $016A                 ; $83E082
    lda.l $7E4DE2,x             ; $83E085
    and.w #$00FF                ; $83E089
    bne CODE_83E091             ; $83E08C
    incbin !baserom:$1E08E..$1E091
CODE_83E091:
    sec                         ; $83E091
    sbc.w #$0074                ; $83E092
    bpl CODE_83E09A             ; $83E095
    incbin !baserom:$1E097..$1E09A
CODE_83E09A:
    cmp.b $14                   ; $83E09A
    bpl CODE_83E0C7             ; $83E09C
    sta.b $14                   ; $83E09E
    eor.w #$FFFF                ; $83E0A0
    sec                         ; $83E0A3
    adc.b $16                   ; $83E0A4
    beq CODE_83E0C7             ; $83E0A6
    ora.w #$0080                ; $83E0A8
    sta.w $0000,y               ; $83E0AB
    lda.w #$0067                ; $83E0AE
    sec                         ; $83E0B1
    sbc.b $14                   ; $83E0B2
    asl a                       ; $83E0B4
    asl a                       ; $83E0B5
    eor.w #$FFFF                ; $83E0B6
    sec                         ; $83E0B9
    adc.w $1E44                 ; $83E0BA
    clc                         ; $83E0BD
    adc.w #$11B6                ; $83E0BE
    sta.w $0001,y               ; $83E0C1
    dey                         ; $83E0C4
    dey                         ; $83E0C5
    dey                         ; $83E0C6
CODE_83E0C7:
    lda.w $02EA                 ; $83E0C7
    clc                         ; $83E0CA
    adc.w #$0040                ; $83E0CB
    bpl CODE_83E0D3             ; $83E0CE
    incbin !baserom:$1E0D0..$1E0D3
CODE_83E0D3:
    sta.b $16                   ; $83E0D3
    sec                         ; $83E0D5
    sbc.b $14                   ; $83E0D6
    bpl CODE_83E0F4             ; $83E0D8
    eor.w #$FFFF                ; $83E0DA
    inc a                       ; $83E0DD
    ora.w #$0080                ; $83E0DE
    sta.w $0000,y               ; $83E0E1
    lda.b $16                   ; $83E0E4
    sta.b $14                   ; $83E0E6
    asl a                       ; $83E0E8
    asl a                       ; $83E0E9
    clc                         ; $83E0EA
    adc.w #$AB28                ; $83E0EB
    sta.w $0001,y               ; $83E0EE
    dey                         ; $83E0F1
    dey                         ; $83E0F2
    dey                         ; $83E0F3
CODE_83E0F4:
    lda.w $02EA                 ; $83E0F4
    bpl CODE_83E0FC             ; $83E0F7
    lda.w #$0000                ; $83E0F9
CODE_83E0FC:
    sta.b $16                   ; $83E0FC
    sec                         ; $83E0FE
    sbc.b $14                   ; $83E0FF
    bpl CODE_83E11B             ; $83E101
    eor.w #$FFFF                ; $83E103
    inc a                       ; $83E106
    sta.w $0000,y               ; $83E107
    lda.w $00E0                 ; $83E10A
    clc                         ; $83E10D
    adc.w #$1566                ; $83E10E
    sta.w $0001,y               ; $83E111
    dey                         ; $83E114
    dey                         ; $83E115
    dey                         ; $83E116
    lda.b $16                   ; $83E117
    sta.b $14                   ; $83E119
CODE_83E11B:
    lda.b $14                   ; $83E11B
    beq CODE_83E12E             ; $83E11D
    ora.w #$0080                ; $83E11F
    sta.w $0000,y               ; $83E122
    lda.w #$A79C                ; $83E125
    sta.w $0001,y               ; $83E128
    dey                         ; $83E12B
    dey                         ; $83E12C
    dey                         ; $83E12D
CODE_83E12E:
    lda.w #$0004                ; $83E12E
    sta.w $0000,y               ; $83E131
    sty.w $00D2                 ; $83E134
    ldy.w $00E0                 ; $83E137
    lda.w #$004B                ; $83E13A
    sec                         ; $83E13D
    sbc.w $02EA                 ; $83E13E
    sta.w $1568,y               ; $83E141
    ldy.w $00D2                 ; $83E144
    lda.w #$0008                ; $83E147
    sta.w $0000,y               ; $83E14A
    dey                         ; $83E14D
    dey                         ; $83E14E
    dey                         ; $83E14F
    lda.w #$0068                ; $83E150
    sta.b $14                   ; $83E153
    lda.w $1EF2                 ; $83E155
    bpl CODE_83E17A             ; $83E158
    incbin !baserom:$1E15A..$1E17A
CODE_83E17A:
    lda.b $14                   ; $83E17A
    sta.b $16                   ; $83E17C
    ldx.w $0168                 ; $83E17E
    lda.l $7E4D4A,x             ; $83E181
    and.w #$00FF                ; $83E185
    bne CODE_83E18D             ; $83E188
    incbin !baserom:$1E18A..$1E18D
CODE_83E18D:
    sec                         ; $83E18D
    sbc.w #$0004                ; $83E18E
    bpl CODE_83E196             ; $83E191
    incbin !baserom:$1E193..$1E196
CODE_83E196:
    cmp.b $14                   ; $83E196
    bpl CODE_83E1C3             ; $83E198
    sta.b $14                   ; $83E19A
    eor.w #$FFFF                ; $83E19C
    sec                         ; $83E19F
    adc.b $16                   ; $83E1A0
    beq CODE_83E1C3             ; $83E1A2
    ora.w #$0080                ; $83E1A4
    sta.w $0000,y               ; $83E1A7
    lda.w #$0067                ; $83E1AA
    sec                         ; $83E1AD
    sbc.b $14                   ; $83E1AE
    asl a                       ; $83E1B0
    asl a                       ; $83E1B1
    eor.w #$FFFF                ; $83E1B2
    sec                         ; $83E1B5
    adc.w $1E40                 ; $83E1B6
    clc                         ; $83E1B9
    adc.w #$0E56                ; $83E1BA
    sta.w $0001,y               ; $83E1BD
    dey                         ; $83E1C0
    dey                         ; $83E1C1
    dey                         ; $83E1C2
CODE_83E1C3:
    lda.w $0237                 ; $83E1C3
    clc                         ; $83E1C6
    adc.w #$0040                ; $83E1C7
    bpl CODE_83E1CF             ; $83E1CA
    incbin !baserom:$1E1CC..$1E1CF
CODE_83E1CF:
    sta.b $16                   ; $83E1CF
    sec                         ; $83E1D1
    sbc.b $14                   ; $83E1D2
    bpl CODE_83E1F0             ; $83E1D4
    eor.w #$FFFF                ; $83E1D6
    inc a                       ; $83E1D9
    ora.w #$0080                ; $83E1DA
    sta.w $0000,y               ; $83E1DD
    lda.b $16                   ; $83E1E0
    sta.b $14                   ; $83E1E2
    asl a                       ; $83E1E4
    asl a                       ; $83E1E5
    clc                         ; $83E1E6
    adc.w #$A968                ; $83E1E7
    sta.w $0001,y               ; $83E1EA
    dey                         ; $83E1ED
    dey                         ; $83E1EE
    dey                         ; $83E1EF
CODE_83E1F0:
    lda.w $0237                 ; $83E1F0
    bpl CODE_83E1F8             ; $83E1F3
    lda.w #$0000                ; $83E1F5
CODE_83E1F8:
    sta.b $16                   ; $83E1F8
    sec                         ; $83E1FA
    sbc.b $14                   ; $83E1FB
    bpl CODE_83E217             ; $83E1FD
    eor.w #$FFFF                ; $83E1FF
    inc a                       ; $83E202
    sta.w $0000,y               ; $83E203
    lda.w $00E0                 ; $83E206
    clc                         ; $83E209
    adc.w #$1556                ; $83E20A
    sta.w $0001,y               ; $83E20D
    dey                         ; $83E210
    dey                         ; $83E211
    dey                         ; $83E212
    lda.b $16                   ; $83E213
    sta.b $14                   ; $83E215
CODE_83E217:
    lda.b $14                   ; $83E217
    beq CODE_83E22A             ; $83E219
    ora.w #$0080                ; $83E21B
    sta.w $0000,y               ; $83E21E
    lda.w #$A5DC                ; $83E221
    sta.w $0001,y               ; $83E224
    dey                         ; $83E227
    dey                         ; $83E228
    dey                         ; $83E229
CODE_83E22A:
    lda.w #$0004                ; $83E22A
    sta.w $0000,y               ; $83E22D
    sty.w $00D2                 ; $83E230
    ldy.w $00E0                 ; $83E233
    lda.w #$00BB                ; $83E236
    sec                         ; $83E239
    sbc.w $0237                 ; $83E23A
    sta.w $1558,y               ; $83E23D
    rtl                         ; $83E240
player1_frame:
    lda.w $1E9E                 ; $83E241
    asl a                       ; $83E244
    tax                         ; $83E245
    lda.l $7E4CDD,x             ; $83E246
    inc a                       ; $83E24A
    sta.l $7E4CDD,x             ; $83E24B
    jsr.w player_frame_setup    ; $83E24F
    phd                         ; $83E252
    lda.w #$0200                ; $83E253
    tcd                         ; $83E256
    jsr.w player_car_frame      ; $83E257
    jsr.w car_track_step        ; $83E25A
    pld                         ; $83E25D
    rtl                         ; $83E25E
mode1_cars_frame:
    lda.w $1E9E                 ; $83E25F
    asl a                       ; $83E262
    tax                         ; $83E263
    lda.l $7E4CDD,x             ; $83E264
    inc a                       ; $83E268
    sta.l $7E4CDD,x             ; $83E269
    jsr.w player_frame_setup    ; $83E26D
    phd                         ; $83E270
    lda.w #$0200                ; $83E271
    tcd                         ; $83E274
    jsr.w player_car_frame      ; $83E275
    jsr.w car_track_step        ; $83E278
    pld                         ; $83E27B
    jsr.w mode1_player2_frame_setup; $83E27C
    phd                         ; $83E27F
    lda.w #$02B3                ; $83E280
    tcd                         ; $83E283
    jsr.w player_car_frame      ; $83E284
    jsr.w car_track_step        ; $83E287
    pld                         ; $83E28A
    rtl                         ; $83E28B
vs_cars_frame:
    lda.w $1E9E                 ; $83E28C
    asl a                       ; $83E28F
    tax                         ; $83E290
    lda.l $7E4CDD,x             ; $83E291
    inc a                       ; $83E295
    sta.l $7E4CDD,x             ; $83E296
    jsr.w vs_player1_frame_setup; $83E29A
    phd                         ; $83E29D
    lda.w #$0200                ; $83E29E
    tcd                         ; $83E2A1
    jsr.w player_car_frame      ; $83E2A2
    jsr.w car_track_step        ; $83E2A5
    pld                         ; $83E2A8
    jsr.w vs_player2_frame_setup; $83E2A9
    phd                         ; $83E2AC
    lda.w #$02B3                ; $83E2AD
    tcd                         ; $83E2B0
    jsr.w player_car_frame      ; $83E2B1
    jsr.w car_track_step        ; $83E2B4
    pld                         ; $83E2B7
    jsr.w vs_player3_frame_setup; $83E2B8
    phd                         ; $83E2BB
    lda.w #$0262                ; $83E2BC
    tcd                         ; $83E2BF
    jsr.w player_car_frame      ; $83E2C0
    jsr.w car_track_step        ; $83E2C3
    pld                         ; $83E2C6
    jsr.w vs_player4_frame_setup; $83E2C7
    phd                         ; $83E2CA
    lda.w #$0315                ; $83E2CB
    tcd                         ; $83E2CE
    jsr.w player_car_frame      ; $83E2CF
    jsr.w car_track_step        ; $83E2D2
    pld                         ; $83E2D5
    rtl                         ; $83E2D6
player_frame_setup:
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
    jsr.w func_83EA9B           ; $83E2F5
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
    jsr.w player_road_limits    ; $83E37E
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
    lda.w #$0002                ; $83E39C
    tsb.w $1E9C                 ; $83E39F
    ldx.w #$109F                ; $83E3A2
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
    lda.w $014C                 ; $83E3CE
    asl a                       ; $83E3D1
    tax                         ; $83E3D2
    lda.l $86DD59,x             ; $83E3D3
    tsb.w $1E9C                 ; $83E3D7
CODE_83E3DA:
    lda.w $03A1                 ; $83E3DA
    bit.w $000C                 ; $83E3DD
    beq CODE_83E3EE             ; $83E3E0
    lda.w $014C                 ; $83E3E2
    asl a                       ; $83E3E5
    tax                         ; $83E3E6
    lda.l $86DD63,x             ; $83E3E7
    tsb.w $1E9C                 ; $83E3EB
CODE_83E3EE:
    lda.w $03A5                 ; $83E3EE
    trb.w $000E                 ; $83E3F1
    beq CODE_83E407             ; $83E3F4
    ldx.b $4C                   ; $83E3F6
CODE_83E3F8:
    dex                         ; $83E3F8
    beq CODE_83E405             ; $83E3F9
    bmi CODE_83E407             ; $83E3FB
    lda.w $0397,x               ; $83E3FD
    and.w #$00FF                ; $83E400
    beq CODE_83E3F8             ; $83E403
CODE_83E405:
    stx.b $4C                   ; $83E405
CODE_83E407:
    lda.w $03A3                 ; $83E407
    trb.w $000E                 ; $83E40A
    beq CODE_83E422             ; $83E40D
    ldx.b $4C                   ; $83E40F
CODE_83E411:
    inx                         ; $83E411
    cpx.w #$0005                ; $83E412
    beq CODE_83E422             ; $83E415
    lda.l $810397,x             ; $83E417
    and.w #$00FF                ; $83E41B
    beq CODE_83E411             ; $83E41E
    stx.b $4C                   ; $83E420
CODE_83E422:
    ldx.b $4C                   ; $83E422
    lda.w $024E,x               ; $83E424
    and.w #$00FF                ; $83E427
    sta.w $025C                 ; $83E42A
    stx.w $1F2E                 ; $83E42D
    rts                         ; $83E430
mode1_player2_frame_setup:
    lda.w #$2000                ; $83E431
    sta.w $1EBE                 ; $83E434
    lda.w $03C4                 ; $83E437
    and.w #$00FF                ; $83E43A
    sta.w $1EB2                 ; $83E43D
    and.w #$007F                ; $83E440
    asl a                       ; $83E443
    tax                         ; $83E444
    lda.w $02D2                 ; $83E445
    asl a                       ; $83E448
    clc                         ; $83E449
    adc.l $86DAD9,x             ; $83E44A
    tax                         ; $83E44E
    jsr.w func_83EA9B           ; $83E44F
    sta.w $02DE                 ; $83E452
    lda.w $03C2                 ; $83E455
    and.w #$00FF                ; $83E458
    asl a                       ; $83E45B
    tax                         ; $83E45C
    lda.l $86DCE1,x             ; $83E45D
    sta.w $1EA8                 ; $83E461
    lda.l $86DCED,x             ; $83E464
    sta.w $1EC0                 ; $83E468
    lda.w $03C6                 ; $83E46B
    and.w #$00FF                ; $83E46E
    asl a                       ; $83E471
    tax                         ; $83E472
    lda.l $86DD29,x             ; $83E473
    sta.w $1EB4                 ; $83E477
    lda.l $86DD35,x             ; $83E47A
    sta.w $1EB6                 ; $83E47E
    lda.w $03C3                 ; $83E481
    and.w #$00FF                ; $83E484
    asl a                       ; $83E487
    tax                         ; $83E488
    lda.l $86DD41,x             ; $83E489
    sta.w $1EB8                 ; $83E48D
    lda.w $03C5                 ; $83E490
    and.w #$00FF                ; $83E493
    asl a                       ; $83E496
    tax                         ; $83E497
    lda.l $86DD4D,x             ; $83E498
    sta.w $1F30                 ; $83E49C
    lda.w #$0001                ; $83E49F
    sta.w $1ECA                 ; $83E4A2
    lda.w #$002F                ; $83E4A5
    sta.w $1ECC                 ; $83E4A8
    lda.w $1E68                 ; $83E4AB
    sta.w $1F2C                 ; $83E4AE
    lda.w #$0035                ; $83E4B1
    sta.w $1EBA                 ; $83E4B4
    lda.w $02D9                 ; $83E4B7
    asl a                       ; $83E4BA
    and.w #$003E                ; $83E4BB
    clc                         ; $83E4BE
    adc.w #$0580                ; $83E4BF
    sta.w $1EBC                 ; $83E4C2
    stz.w $1E9C                 ; $83E4C5
    lda.w $02F7                 ; $83E4C8
    bit.w #$0004                ; $83E4CB
    bne CODE_83E4D9             ; $83E4CE
    lda.l $8103DC               ; $83E4D0
    bit.w #$0020                ; $83E4D4
    db $D0,$0A ; bne            ; $83E4D7
CODE_83E4D9:
    phd                         ; $83E4D9
    lda.w #$02B3                ; $83E4DA
    tcd                         ; $83E4DD
    jsr.w player_road_limits    ; $83E4DE
    pld                         ; $83E4E1
    rts                         ; $83E4E2
    incbin !baserom:$1E4E3..$1E597
vs_player1_frame_setup:
    lda.w #$8000                ; $83E597
    sta.w $1EBE                 ; $83E59A
    lda.w $0418                 ; $83E59D
    and.w #$00FF                ; $83E5A0
    asl a                       ; $83E5A3
    sta.b $14                   ; $83E5A4
    ldx.b $14                   ; $83E5A6
    lda.l $86DD75,x             ; $83E5A8
    and.w #$00FF                ; $83E5AC
    sta.w $1EB2                 ; $83E5AF
    and.w #$007F                ; $83E5B2
    asl a                       ; $83E5B5
    tax                         ; $83E5B6
    lda.w $021F                 ; $83E5B7
    asl a                       ; $83E5BA
    clc                         ; $83E5BB
    adc.l $86DAD9,x             ; $83E5BC
    tax                         ; $83E5C0
    jsr.w func_83EA9B           ; $83E5C1
    sta.w $022B                 ; $83E5C4
    jsr.w vs_car_parts          ; $83E5C7
    lda.l $86DD45               ; $83E5CA
    sta.w $1EB8                 ; $83E5CE
    stz.w $1ECA                 ; $83E5D1
    lda.w #$0000                ; $83E5D4
    sta.w $1EBA                 ; $83E5D7
    lda.w $0226                 ; $83E5DA
    asl a                       ; $83E5DD
    and.w #$003E                ; $83E5DE
    clc                         ; $83E5E1
    adc.w #$0000                ; $83E5E2
    sta.w $1EBC                 ; $83E5E5
    stz.w $1E9C                 ; $83E5E8
    lda.w $0244                 ; $83E5EB
    bit.w #$0004                ; $83E5EE
    bne CODE_83E5FC             ; $83E5F1
    lda.l $8103DC               ; $83E5F3
    bit.w #$0010                ; $83E5F7
    bne CODE_83E606             ; $83E5FA
CODE_83E5FC:
    phd                         ; $83E5FC
    lda.w #$0200                ; $83E5FD
    tcd                         ; $83E600
    jsr.w player_road_limits    ; $83E601
    pld                         ; $83E604
    rts                         ; $83E605
CODE_83E606:
    lda.w $000C                 ; $83E606
    bit.w $039D                 ; $83E609
    beq CODE_83E614             ; $83E60C
    incbin !baserom:$1E60E..$1E614
CODE_83E614:
    lda.w $000C                 ; $83E614
    bit.w $039F                 ; $83E617
    beq CODE_83E622             ; $83E61A
    incbin !baserom:$1E61C..$1E622
CODE_83E622:
    lda.w $000C                 ; $83E622
    bit.w #$0200                ; $83E625
    beq CODE_83E630             ; $83E628
    incbin !baserom:$1E62A..$1E630
CODE_83E630:
    lda.w $000C                 ; $83E630
    bit.w #$0100                ; $83E633
    beq CODE_83E63E             ; $83E636
    incbin !baserom:$1E638..$1E63E
CODE_83E63E:
    lda.w $03A1                 ; $83E63E
    trb.w $000E                 ; $83E641
    beq CODE_83E64C             ; $83E644
    lda.w #$0010                ; $83E646
    tsb.w $1E9C                 ; $83E649
CODE_83E64C:
    rts                         ; $83E64C
vs_player2_frame_setup:
    lda.w #$2000                ; $83E64D
    sta.w $1EBE                 ; $83E650
    lda.w $0419                 ; $83E653
    and.w #$00FF                ; $83E656
    asl a                       ; $83E659
    sta.b $14                   ; $83E65A
    ldx.b $14                   ; $83E65C
    lda.l $86DD75,x             ; $83E65E
    and.w #$00FF                ; $83E662
    sta.w $1EB2                 ; $83E665
    and.w #$007F                ; $83E668
    asl a                       ; $83E66B
    tax                         ; $83E66C
    lda.w $02D2                 ; $83E66D
    asl a                       ; $83E670
    clc                         ; $83E671
    adc.l $86DAD9,x             ; $83E672
    tax                         ; $83E676
    jsr.w func_83EA9B           ; $83E677
    sta.w $02DE                 ; $83E67A
    jsr.w vs_car_parts          ; $83E67D
    lda.l $86DD45               ; $83E680
    sta.w $1EB8                 ; $83E684
    lda.w #$0001                ; $83E687
    sta.w $1ECA                 ; $83E68A
    lda.w #$0035                ; $83E68D
    sta.w $1EBA                 ; $83E690
    lda.w $02D9                 ; $83E693
    asl a                       ; $83E696
    and.w #$003E                ; $83E697
    clc                         ; $83E69A
    adc.w #$0580                ; $83E69B
    sta.w $1EBC                 ; $83E69E
    stz.w $1E9C                 ; $83E6A1
    lda.w $02F7                 ; $83E6A4
    bit.w #$0004                ; $83E6A7
    bne CODE_83E6B5             ; $83E6AA
    lda.l $8103DC               ; $83E6AC
    bit.w #$0020                ; $83E6B0
    db $D0,$0A ; bne            ; $83E6B3
CODE_83E6B5:
    phd                         ; $83E6B5
    lda.w #$02B3                ; $83E6B6
    tcd                         ; $83E6B9
    jsr.w player_road_limits    ; $83E6BA
    pld                         ; $83E6BD
    rts                         ; $83E6BE
    incbin !baserom:$1E6BF..$1E706
vs_player3_frame_setup:
    lda.w #$0800                ; $83E706
    sta.w $1EBE                 ; $83E709
    lda.w $041A                 ; $83E70C
    and.w #$00FF                ; $83E70F
    asl a                       ; $83E712
    sta.b $14                   ; $83E713
    ldx.b $14                   ; $83E715
    lda.l $86DD75,x             ; $83E717
    and.w #$00FF                ; $83E71B
    sta.w $1EB2                 ; $83E71E
    and.w #$007F                ; $83E721
    asl a                       ; $83E724
    tax                         ; $83E725
    lda.w $0281                 ; $83E726
    asl a                       ; $83E729
    clc                         ; $83E72A
    adc.l $86DAD9,x             ; $83E72B
    tax                         ; $83E72F
    jsr.w func_83EA9B           ; $83E730
    sta.w $028D                 ; $83E733
    jsr.w vs_car_parts          ; $83E736
    lda.l $86DD45               ; $83E739
    sta.w $1EB8                 ; $83E73D
    lda.w #$0002                ; $83E740
    sta.w $1ECA                 ; $83E743
    lda.w #$006A                ; $83E746
    sta.w $1EBA                 ; $83E749
    lda.w $0288                 ; $83E74C
    asl a                       ; $83E74F
    and.w #$003E                ; $83E750
    clc                         ; $83E753
    adc.w #$0300                ; $83E754
    sta.w $1EBC                 ; $83E757
    stz.w $1E9C                 ; $83E75A
    lda.w $02A6                 ; $83E75D
    bit.w #$0004                ; $83E760
    bne CODE_83E76E             ; $83E763
    lda.l $8103DC               ; $83E765
    bit.w #$0040                ; $83E769
    db $D0,$0A ; bne            ; $83E76C
CODE_83E76E:
    phd                         ; $83E76E
    lda.w #$0262                ; $83E76F
    tcd                         ; $83E772
    jsr.w player_road_limits    ; $83E773
    pld                         ; $83E776
    rts                         ; $83E777
    incbin !baserom:$1E778..$1E7BF
vs_player4_frame_setup:
    lda.w #$0200                ; $83E7BF
    sta.w $1EBE                 ; $83E7C2
    lda.w $041B                 ; $83E7C5
    and.w #$00FF                ; $83E7C8
    asl a                       ; $83E7CB
    sta.b $14                   ; $83E7CC
    ldx.b $14                   ; $83E7CE
    lda.l $86DD75,x             ; $83E7D0
    and.w #$00FF                ; $83E7D4
    sta.w $1EB2                 ; $83E7D7
    and.w #$007F                ; $83E7DA
    asl a                       ; $83E7DD
    tax                         ; $83E7DE
    lda.w $0334                 ; $83E7DF
    asl a                       ; $83E7E2
    clc                         ; $83E7E3
    adc.l $86DAD9,x             ; $83E7E4
    tax                         ; $83E7E8
    jsr.w func_83EA9B           ; $83E7E9
    sta.w $0340                 ; $83E7EC
    jsr.w vs_car_parts          ; $83E7EF
    lda.l $86DD45               ; $83E7F2
    sta.w $1EB8                 ; $83E7F6
    lda.w #$0003                ; $83E7F9
    sta.w $1ECA                 ; $83E7FC
    lda.w #$009F                ; $83E7FF
    sta.w $1EBA                 ; $83E802
    lda.w $033B                 ; $83E805
    asl a                       ; $83E808
    and.w #$003E                ; $83E809
    clc                         ; $83E80C
    adc.w #$0880                ; $83E80D
    sta.w $1EBC                 ; $83E810
    stz.w $1E9C                 ; $83E813
    lda.w $0359                 ; $83E816
    bit.w #$0004                ; $83E819
    bne CODE_83E827             ; $83E81C
    lda.l $8103DC               ; $83E81E
    bit.w #$0080                ; $83E822
    db $D0,$0A ; bne            ; $83E825
CODE_83E827:
    phd                         ; $83E827
    lda.w #$0315                ; $83E828
    tcd                         ; $83E82B
    jsr.w player_road_limits    ; $83E82C
    pld                         ; $83E82F
    rts                         ; $83E830
    incbin !baserom:$1E831..$1E878
vs_car_parts:
    ldx.b $14                   ; $83E878
    lda.l $86DD6D,x             ; $83E87A
    and.w #$00FF                ; $83E87E
    asl a                       ; $83E881
    tax                         ; $83E882
    lda.l $86DCE1,x             ; $83E883
    sta.w $1EA8                 ; $83E887
    lda.l $86DCED,x             ; $83E88A
    sta.w $1EC0                 ; $83E88E
    ldx.b $14                   ; $83E891
    lda.l $86DD7D,x             ; $83E893
    and.w #$00FF                ; $83E897
    asl a                       ; $83E89A
    tax                         ; $83E89B
    lda.l $86DD29,x             ; $83E89C
    sta.w $1EB4                 ; $83E8A0
    lda.l $86DD35,x             ; $83E8A3
    sta.w $1EB6                 ; $83E8A7
    rts                         ; $83E8AA
player_road_limits:
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
    sta.w $0114                 ; $83E921
CODE_83E924:
    lda.w $8000                 ; $83E924
    cmp.w $0116                 ; $83E927
    bmi CODE_83E92F             ; $83E92A
    sta.w $0116                 ; $83E92C
CODE_83E92F:
    lda.w $8000                 ; $83E92F
    cmp.w $0118                 ; $83E932
    bpl CODE_83E93A             ; $83E935
    sta.w $0118                 ; $83E937
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
    lda.l $7EFABB,x             ; $83E96C
    clc                         ; $83E970
    adc.w #$0080                ; $83E971
    cmp.w $0116                 ; $83E974
    bpl CODE_83E985             ; $83E977
    cmp.w $0114                 ; $83E979
    bmi CODE_83E9C3             ; $83E97C
    bra CODE_83E9B3             ; $83E97E
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
CODE_83E9B3:
    lda.w #$0002                ; $83E9B3
    trb.b $44                   ; $83E9B6
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
    lda.b $B1                   ; $83E9DF
    and.w #$00FF                ; $83E9E1
    asl a                       ; $83E9E4
    asl a                       ; $83E9E5
    asl a                       ; $83E9E6
    asl a                       ; $83E9E7
    bit.w #$0800                ; $83E9E8
    beq CODE_83E9F0             ; $83E9EB
    incbin !baserom:$1E9ED..$1E9F0
CODE_83E9F0:
    sta.w $0114                 ; $83E9F0
    sta.w $0116                 ; $83E9F3
CODE_83E9F6:
    stz.w $011E                 ; $83E9F6
    lda.w $1F2C                 ; $83E9F9
    lsr a                       ; $83E9FC
    bcc CODE_83EA1F             ; $83E9FD
    ldx.w $1EBC                 ; $83E9FF
    lda.b $14                   ; $83EA02
    sec                         ; $83EA04
    sbc.l $7E3DEC,x             ; $83EA05
    sta.b $14                   ; $83EA09
    lda.b $16                   ; $83EA0B
    sec                         ; $83EA0D
    sbc.l $7E3DEC,x             ; $83EA0E
    sta.b $16                   ; $83EA12
    lda.w #$0000                ; $83EA14
    sec                         ; $83EA17
    sbc.l $7E3DEC,x             ; $83EA18
    sta.w $011E                 ; $83EA1C
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
    bpl CODE_83EA4A             ; $83EA3E
    lda.w #$0002                ; $83EA40
    trb.b $44                   ; $83EA43
    lda.w $0116                 ; $83EA45
    bra CODE_83EA52             ; $83EA48
CODE_83EA4A:
    lda.w #$0002                ; $83EA4A
    tsb.b $44                   ; $83EA4D
    lda.w $0114                 ; $83EA4F
CODE_83EA52:
    sta.w $011C                 ; $83EA52
CODE_83EA55:
    lda.l $7E3AEC,x             ; $83EA55
    beq CODE_83EA72             ; $83EA59
    bpl CODE_83EA67             ; $83EA5B
    lda.w #$0002                ; $83EA5D
    trb.b $44                   ; $83EA60
    lda.w $0116                 ; $83EA62
    bra CODE_83EA6F             ; $83EA65
CODE_83EA67:
    lda.w #$0002                ; $83EA67
    tsb.b $44                   ; $83EA6A
    lda.w $0114                 ; $83EA6C
CODE_83EA6F:
    sta.w $011C                 ; $83EA6F
CODE_83EA72:
    lda.w $011C                 ; $83EA72
    sec                         ; $83EA75
    sbc.b $06                   ; $83EA76
    asl a                       ; $83EA78
    cmp.w #$FFD0                ; $83EA79
    bmi CODE_83EA86             ; $83EA7C
    cmp.w #$0030                ; $83EA7E
    bpl CODE_83EA86             ; $83EA81
    lda.w #$0000                ; $83EA83
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
func_83EA9B:
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
player_car_frame:
    pea.w $8330                 ; $83EAC6
    plb                         ; $83EAC9
    lda.w $1ECE                 ; $83EACA
    beq CODE_83EB31             ; $83EACD
    bpl CODE_83EAD4             ; $83EACF
    jsr.w follow_car_step       ; $83EAD1
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
    lda.b $31                   ; $83EB3F
    bne CODE_83EB6B             ; $83EB41
    lda.b $4E                   ; $83EB43
    and.w #$00FF                ; $83EB45
    beq CODE_83EB5F             ; $83EB48
    dec.b $4E                   ; $83EB4A
    lda.w $1EB6                 ; $83EB4C
    sta.b $31                   ; $83EB4F
    lda.w #$FF1B                ; $83EB51
    ldx.w $1EBE                 ; $83EB54
    ldy.w #$003C                ; $83EB57
    jsr.w func_83FE9A           ; $83EB5A
    bra CODE_83EB6B             ; $83EB5D
CODE_83EB5F:
    lda.w #$FF1C                ; $83EB5F
    ldx.w $1EBE                 ; $83EB62
    ldy.w #$0000                ; $83EB65
    jsr.w func_83FE9A           ; $83EB68
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
    lda.b $1A                   ; $83EB80
    bne CODE_83EBC3             ; $83EB82
    lda.b $51                   ; $83EB84
    and.w #$00FF                ; $83EB86
    beq CODE_83EBC3             ; $83EB89
    dec.b $51                   ; $83EB8B
    lda.b $44                   ; $83EB8D
    bit.w #$0020                ; $83EB8F
    bne CODE_83EBC3             ; $83EB92
    ora.w #$0120                ; $83EB94
    sta.b $44                   ; $83EB97
    lda.w #$0005                ; $83EB99
    sta.b $1A                   ; $83EB9C
    lda.w #$FF29                ; $83EB9E
    ldx.w $1EBE                 ; $83EBA1
    ldy.w #$0000                ; $83EBA4
    jsr.w func_83FE9A           ; $83EBA7
    ldx.w $1EBA                 ; $83EBAA
    lda.w #$0008                ; $83EBAD
    ora.l $7EFADE,x             ; $83EBB0
    sta.l $7EFADE,x             ; $83EBB4
    lda.w #$0020                ; $83EBB8
    sta.b $5E                   ; $83EBBB
    lda.b $10                   ; $83EBBD
    sta.b $70                   ; $83EBBF
    sta.b $72                   ; $83EBC1
CODE_83EBC3:
    lda.w #$0020                ; $83EBC3
    trb.w $1E9C                 ; $83EBC6
    beq CODE_83EBED             ; $83EBC9
    lda.b $1A                   ; $83EBCB
    bne CODE_83EBED             ; $83EBCD
    and.w #$7FFF                ; $83EBCF
    dec a                       ; $83EBD2
    beq CODE_83EBED             ; $83EBD3
    cmp.w #$0005                ; $83EBD5
    beq CODE_83EBED             ; $83EBD8
    stz.b $7E                   ; $83EBDA
    lda.w #$0001                ; $83EBDC
    sta.b $1A                   ; $83EBDF
    lda.w #$FF1D                ; $83EBE1
    ldx.w $1EBE                 ; $83EBE4
    ldy.w #$0000                ; $83EBE7
    jsr.w func_83FE9A           ; $83EBEA
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
    jsr.w func_83FE9A           ; $83EC1A
CODE_83EC1D:
    lda.w #$0040                ; $83EC1D
    bit.w $1E9C                 ; $83EC20
    beq CODE_83EC30             ; $83EC23
    lda.b $50                   ; $83EC25
    and.w #$007F                ; $83EC27
    beq CODE_83EC30             ; $83EC2A
    lda.b $81                   ; $83EC2C
    bpl CODE_83EC33             ; $83EC2E
CODE_83EC30:
    jmp.w $ECF6                 ; $83EC30
CODE_83EC33:
    sep.b #$20                  ; $83EC33
    lda.w $1E9E                 ; $83EC35
    asl a                       ; $83EC38
    asl a                       ; $83EC39
    asl a                       ; $83EC3A
    eor.b #$FF                  ; $83EC3B
    sec                         ; $83EC3D
    adc.b $80                   ; $83EC3E
    sta.b $80                   ; $83EC40
    lda.b $50                   ; $83EC42
    sbc.b #$00                  ; $83EC44
    sta.b $50                   ; $83EC46
    rep.b #$20                  ; $83EC48
    ldx.b $81                   ; $83EC4A
    beq CODE_83EC62             ; $83EC4C
    lda.l $8103DC               ; $83EC4E
    and.w #$000F                ; $83EC52
    beq CODE_83EC68             ; $83EC55
    cpx.w #$0035                ; $83EC57 static
    bne CODE_83EC68             ; $83EC5A static
    ldx.w #$00B3                ; $83EC5C static
    jmp.w $ECAA                 ; $83EC5F static
CODE_83EC62:
    ldx.w #$0000                ; $83EC62 static
    jmp.w $ECAA                 ; $83EC65 static
CODE_83EC68:
    lda.l $7EFAC9,x             ; $83EC68
    sta.w $011C                 ; $83EC6C
    lda.l $7EFACB,x             ; $83EC6F
    sta.w $011E                 ; $83EC73
    jsr.w $F6E1                 ; $83EC76
    lda.l $7EFAC9,x             ; $83EC79
    sec                         ; $83EC7D
    sbc.w $0115                 ; $83EC7E
    sta.l $7EFAC9,x             ; $83EC81
    lda.l $7EFACB,x             ; $83EC85
    sbc.w $0117                 ; $83EC89
    sta.l $7EFACB,x             ; $83EC8C
    bpl CODE_83EC9D             ; $83EC90
    lda.w #$0000                ; $83EC92 static
    sta.l $7EFAC9,x             ; $83EC95 static
    sta.l $7EFACB,x             ; $83EC99 static
CODE_83EC9D:
    lda.w $013C                 ; $83EC9D
    sec                         ; $83ECA0
    sbc.w $0116                 ; $83ECA1
    sta.w $013C                 ; $83ECA4
    jmp.w $ECF6                 ; $83ECA7
    lda.w $020E,x               ; $83ECAA static
    sta.w $011C                 ; $83ECAD static
    lda.w $0210,x               ; $83ECB0 static
    sta.w $011E                 ; $83ECB3 static
    jsr.w $F6E1                 ; $83ECB6 static
    lda.w #$0000                ; $83ECB9 static
    sta.l $308000               ; $83ECBC static
    lda.w $0116                 ; $83ECC0 static
    sta.l $308000               ; $83ECC3 static
    lda.w $0295,x               ; $83ECC7 static
    sta.l $308000               ; $83ECCA static
    nop                         ; $83ECCE static
    lda.w $0216,x               ; $83ECCF static
    sec                         ; $83ECD2 static
    sbc.l $308000               ; $83ECD3 static
    sta.w $0216,x               ; $83ECD7 static
    lda.w $0218,x               ; $83ECDA static
    sbc.l $308000               ; $83ECDD static
    bpl CODE_83ECE9             ; $83ECE1 static
    stz.w $0216                 ; $83ECE3 static
    lda.w #$0000                ; $83ECE6 static
CODE_83ECE9:
    sta.w $0218,x               ; $83ECE9 static
    lda.w $013C                 ; $83ECEC static
    sec                         ; $83ECEF static
    sbc.w $0117                 ; $83ECF0 static
    sta.w $013C                 ; $83ECF3 static
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
    lda.w #$0000                ; $83ED7F static
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
    clc                         ; $83ED99
    adc.w #$2400                ; $83ED9A
CODE_83ED9D:
    ldx.b $31                   ; $83ED9D
    beq CODE_83EDA5             ; $83ED9F
    clc                         ; $83EDA1
    adc.w $1EB4                 ; $83EDA2
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
    lda.w #$007F                ; $83EDD3 static
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
    stz.b $14                   ; $83EED5
    stz.b $12                   ; $83EED7
    lda.w #$0000                ; $83EED9
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
    jsr.w func_83EA9B           ; $83EF30
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
    stz.w $8000                 ; $83F0F9
    lda.w $1E9E                 ; $83F0FC
    sta.w $8000                 ; $83F0FF
    lda.w #$0100                ; $83F102
    sta.w $8000                 ; $83F105
    lda.w $013C                 ; $83F108
    clc                         ; $83F10B
    adc.w $8000                 ; $83F10C
    sta.w $013C                 ; $83F10F
    lda.w $8000                 ; $83F112
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
    stz.b $16                   ; $83F134
    lda.w #$0000                ; $83F136
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
    stz.w $8000                 ; $83F1F0
    lda.b $1C                   ; $83F1F3
    sta.w $8000                 ; $83F1F5
    lda.w $1E9E                 ; $83F1F8
    xba                         ; $83F1FB
    sta.w $8000                 ; $83F1FC
    lda.b $04                   ; $83F1FF
    clc                         ; $83F201
    adc.w $8000                 ; $83F202
    sta.b $04                   ; $83F205
    lda.b $06                   ; $83F207
    adc.w $8000                 ; $83F209
    sta.b $06                   ; $83F20C
    stz.w $8000                 ; $83F20E
    lda.w #$00A0                ; $83F211
    sta.w $8000                 ; $83F214
    lda.w $1E9E                 ; $83F217
    sta.w $8000                 ; $83F21A
    lda.b $1C                   ; $83F21D
    bpl CODE_83F230             ; $83F21F
    clc                         ; $83F221
    adc.w $8000                 ; $83F222
    sta.b $1C                   ; $83F225
    sta.w $8000                 ; $83F227
    bmi CODE_83F23F             ; $83F22A
    stz.b $1C                   ; $83F22C
    bra CODE_83F23F             ; $83F22E
CODE_83F230:
    sec                         ; $83F230
    sbc.w $8000                 ; $83F231
    sta.b $1C                   ; $83F234
    sta.w $8000                 ; $83F236
    bpl CODE_83F23F             ; $83F239
    stz.b $1C                   ; $83F23B
    bra CODE_83F23F             ; $83F23D
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
    jsr.w func_83FE9A           ; $83F355
    bra CODE_83F35C             ; $83F358
CODE_83F35A:
    stz.b $46                   ; $83F35A
CODE_83F35C:
    bit.w $1F31                 ; $83F35C
    bpl CODE_83F364             ; $83F35F
    jmp.w $F407                 ; $83F361
CODE_83F364:
    lda.b $06                   ; $83F364
    bpl CODE_83F36C             ; $83F366
    eor.w #$FFFF                ; $83F368
    inc a                       ; $83F36B
CODE_83F36C:
    sec                         ; $83F36C
    sbc.w #$0280                ; $83F36D
    bcc CODE_83F3B0             ; $83F370
    lda.b $06                   ; $83F372
    sec                         ; $83F374
    sbc.b $9E                   ; $83F375
    bpl CODE_83F37D             ; $83F377
    eor.w #$FFFF                ; $83F379
    inc a                       ; $83F37C
CODE_83F37D:
    sec                         ; $83F37D
    sbc.w #$0280                ; $83F37E
    bcc CODE_83F3B0             ; $83F381
    lda.w $1F2C                 ; $83F383
    bit.w #$8001                ; $83F386
    bne CODE_83F399             ; $83F389
    lda.b $06                   ; $83F38B
    bpl CODE_83F394             ; $83F38D
    lda.w #$FD80                ; $83F38F
    bra CODE_83F3AE             ; $83F392
CODE_83F394:
    lda.w #$0280                ; $83F394
    bra CODE_83F3AE             ; $83F397
CODE_83F399:
    lda.b $06                   ; $83F399
    sec                         ; $83F39B
    sbc.b $9E                   ; $83F39C
    bpl CODE_83F3A8             ; $83F39E
    lda.b $9E                   ; $83F3A0
    clc                         ; $83F3A2
    adc.w #$FD80                ; $83F3A3
    bra CODE_83F3AE             ; $83F3A6
CODE_83F3A8:
    lda.b $9E                   ; $83F3A8
    clc                         ; $83F3AA
    adc.w #$0280                ; $83F3AB
CODE_83F3AE:
    sta.b $06                   ; $83F3AE
CODE_83F3B0:
    lda.b $60                   ; $83F3B0
    and.w #$00FF                ; $83F3B2
    tax                         ; $83F3B5
    jmp.w ($F3B9,x)             ; $83F3B6
    incbin !baserom:$1F3B9..$1F3C5
    lda.b $26                   ; $83F3C5
    and.w #$003F                ; $83F3C7
    cmp.w #$0038                ; $83F3CA
    bcs CODE_83F423             ; $83F3CD
    cmp.w #$0018                ; $83F3CF
    bcc CODE_83F423             ; $83F3D2
    lda.w $1F2C                 ; $83F3D4
    bpl CODE_83F3F0             ; $83F3D7
    lda.b $9E                   ; $83F3D9
    bmi CODE_83F3E6             ; $83F3DB
    lsr a                       ; $83F3DD
    cmp.b $06                   ; $83F3DE
    bmi CODE_83F423             ; $83F3E0
    sta.b $06                   ; $83F3E2 static
    bra CODE_83F423             ; $83F3E4 static
CODE_83F3E6:
    sec                         ; $83F3E6
    ror a                       ; $83F3E7
    cmp.b $06                   ; $83F3E8
    bpl CODE_83F423             ; $83F3EA
    sta.b $06                   ; $83F3EC static
    bra CODE_83F423             ; $83F3EE static
CODE_83F3F0:
    lda.b $9E                   ; $83F3F0
    bmi CODE_83F3FD             ; $83F3F2
    lsr a                       ; $83F3F4
    cmp.b $06                   ; $83F3F5
    bpl CODE_83F423             ; $83F3F7
    sta.b $06                   ; $83F3F9 static
    bra CODE_83F423             ; $83F3FB static
CODE_83F3FD:
    sec                         ; $83F3FD
    ror a                       ; $83F3FE
    cmp.b $06                   ; $83F3FF
    bmi CODE_83F423             ; $83F401
    incbin !baserom:$1F403..$1F407
    lda.b $06                   ; $83F407
    bpl CODE_83F40F             ; $83F409
    eor.w #$FFFF                ; $83F40B
    inc a                       ; $83F40E
CODE_83F40F:
    sec                         ; $83F40F
    sbc.w #$0280                ; $83F410
    bcc CODE_83F423             ; $83F413
    incbin !baserom:$1F415..$1F423
CODE_83F423:
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
    lda.w #$0040                ; $83F47B
    tsb.b $44                   ; $83F47E
    lda.l $86DD03,x             ; $83F480
    bra CODE_83F48A             ; $83F484
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
    jsr.w car_message_request   ; $83F558
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
    jsr.w func_83FE9A           ; $83F5B2
CODE_83F5B5:
    jmp.w $F647                 ; $83F5B5
    lda.w #$003B                ; $83F5B8
    jsr.w car_message_request   ; $83F5BB
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
    jsr.w func_83FE9A           ; $83F60C
    bra CODE_83F647             ; $83F60F
    lda.w #$0258                ; $83F611
    sta.b $48                   ; $83F614
    bra CODE_83F647             ; $83F616
    lda.b $44                   ; $83F618
    bit.w #$0020                ; $83F61A
    bne CODE_83F647             ; $83F61D
    ora.w #$0020                ; $83F61F
    sta.b $44                   ; $83F622
    lda.w #$0005                ; $83F624
    sta.b $1A                   ; $83F627
    ldx.w $1EBA                 ; $83F629
    lda.w #$0008                ; $83F62C
    ora.l $7EFADE,x             ; $83F62F
    sta.l $7EFADE,x             ; $83F633
    lda.w #$FF29                ; $83F637
    ldx.w $1EBE                 ; $83F63A
    ldy.w #$0000                ; $83F63D
    jsr.w func_83FE9A           ; $83F640
    bra CODE_83F647             ; $83F643
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
    jsr.w func_83FE9A           ; $83F68A
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
    lda.w #$0000                ; $83F6E1
    sta.l $308000               ; $83F6E4
    lda.w $011C                 ; $83F6E8
    sta.l $308000               ; $83F6EB
    lda.w $1E9E                 ; $83F6EF
    sta.l $308000               ; $83F6F2
    nop                         ; $83F6F6
    stz.w $0118                 ; $83F6F7
    lda.l $308000               ; $83F6FA
    sta.w $0114                 ; $83F6FE
    lda.l $308000               ; $83F701
    sta.w $0116                 ; $83F705
    bpl CODE_83F70D             ; $83F708
    dec.w $0118                 ; $83F70A
CODE_83F70D:
    lda.w #$0000                ; $83F70D
    sta.l $308000               ; $83F710
    lda.w $011E                 ; $83F714
    bit.w $011C                 ; $83F717
    bpl CODE_83F71D             ; $83F71A
    inc a                       ; $83F71C
CODE_83F71D:
    sta.l $308000               ; $83F71D
    lda.w $1E9E                 ; $83F721
    sta.l $308000               ; $83F724
    lda.w $0116                 ; $83F728
    clc                         ; $83F72B
    adc.l $308000               ; $83F72C
    sta.w $0116                 ; $83F730
    lda.w $0118                 ; $83F733
    adc.l $308000               ; $83F736
    sta.w $0118                 ; $83F73A
    rts                         ; $83F73D
follow_car_step:
    lda.w #$0001                ; $83F73E
    bit.b $44                   ; $83F741
    bne CODE_83F748             ; $83F743
    jmp.w $F7F8                 ; $83F745 static
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
car_track_step:
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
    ldx.w $1EBA                 ; $83F82B
    lda.w #$FFF7                ; $83F82E
    and.l $7EFADE,x             ; $83F831
    sta.l $7EFADE,x             ; $83F835
CODE_83F839:
    lda.b $44                   ; $83F839
    and.w #$0020                ; $83F83B
    cmp.w #$0020                ; $83F83E
    bne CODE_83F892             ; $83F841
    lda.b $44                   ; $83F843
    bit.w #$0001                ; $83F845
    bne CODE_83F895             ; $83F848
    lda.b $44                   ; $83F84A
    bit.w #$0100                ; $83F84C
    beq CODE_83F87D             ; $83F84F
    lda.b $5E                   ; $83F851
    beq CODE_83F866             ; $83F853
    sec                         ; $83F855
    sbc.w $1E9E                 ; $83F856
    bcs CODE_83F85E             ; $83F859
    lda.w #$0000                ; $83F85B
CODE_83F85E:
    sta.b $5E                   ; $83F85E
    jsr.w $F898                 ; $83F860
    jmp.w $F914                 ; $83F863
CODE_83F866:
    lda.w $1E9E                 ; $83F866
    asl a                       ; $83F869
    asl a                       ; $83F86A
    eor.w #$FFFF                ; $83F86B
    sec                         ; $83F86E
    adc.b $70                   ; $83F86F
    cmp.b $72                   ; $83F871
    bmi CODE_83F8D0             ; $83F873
    sta.b $70                   ; $83F875
    jsr.w $F8AA                 ; $83F877
    jmp.w $F914                 ; $83F87A
CODE_83F87D:
    jsr.w $F898                 ; $83F87D
    lda.b $26                   ; $83F880
    cmp.b $5E                   ; $83F882
    bcc CODE_83F88C             ; $83F884
    lda.b $5E                   ; $83F886
    sta.b $26                   ; $83F888
    bra CODE_83F8D0             ; $83F88A
CODE_83F88C:
    dec.w $0120                 ; $83F88C
    jmp.w $F914                 ; $83F88F
CODE_83F892:
    jmp.w $F8F0                 ; $83F892
CODE_83F895:
    jmp.w $F914                 ; $83F895
    lda.w $1E9E                 ; $83F898
    asl a                       ; $83F89B
    asl a                       ; $83F89C
    clc                         ; $83F89D
    adc.b $70                   ; $83F89E
    cmp.w #$0100                ; $83F8A0
    bcc CODE_83F8A8             ; $83F8A3
    lda.w #$0100                ; $83F8A5
CODE_83F8A8:
    sta.b $70                   ; $83F8A8
    lda.w #$0000                ; $83F8AA
    sta.l $308000               ; $83F8AD
    lda.w $1E9E                 ; $83F8B1
    sta.l $308000               ; $83F8B4
    lda.b $70                   ; $83F8B8
    sta.l $308000               ; $83F8BA
    lda.b $25                   ; $83F8BE
    clc                         ; $83F8C0
    adc.l $308000               ; $83F8C1
    sta.b $25                   ; $83F8C5
    lda.b $27                   ; $83F8C7
    adc.l $308000               ; $83F8C9
    sta.b $27                   ; $83F8CD
    rts                         ; $83F8CF
CODE_83F8D0:
    lda.w #$0120                ; $83F8D0
    trb.b $44                   ; $83F8D3
    lda.w #$0080                ; $83F8D5
    tsb.b $44                   ; $83F8D8
    lda.w #$0004                ; $83F8DA
    sta.b $1A                   ; $83F8DD
    lda.w #$FF2A                ; $83F8DF
    ldx.w $1EBE                 ; $83F8E2
    ldy.w #$0000                ; $83F8E5
    jsr.w func_83FE9A           ; $83F8E8
    dec.w $0120                 ; $83F8EB
    bra CODE_83F914             ; $83F8EE
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
    jsr.w car_lap_check         ; $83F914
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
car_lap_check:
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
    lda.w #$0001                ; $83F9EA
    ora.b $7C                   ; $83F9ED
    sta.b $7C                   ; $83F9EF
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
clc_83FA32:
    clc                         ; $83FA32
    rts                         ; $83FA33
track_object_test:
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
    jmp.w $FA63                 ; $83FA59
CODE_83FA5C:
    lda.w $0128                 ; $83FA5C
    jsr.w func_83FD8A           ; $83FA5F
    rts                         ; $83FA62
    lda.w $0128                 ; $83FA63
    jsr.w func_83FDCD           ; $83FA66
    rts                         ; $83FA69
track_object_bump:
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
    lda.b $1A                   ; $83FA94
    bne CODE_83FAD5             ; $83FA96
    lda.w #$0008                ; $83FA98
    sta.b $7E                   ; $83FA9B
    lda.w #$0006                ; $83FA9D
    sta.b $1A                   ; $83FAA0
    lda.w #$FF1D                ; $83FAA2
    ldx.w $1EBE                 ; $83FAA5
    ldy.w #$0000                ; $83FAA8
    jsr.w func_83FE9A           ; $83FAAB
    bra CODE_83FAD5             ; $83FAAE
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
    jsr.w func_83FE9A           ; $83FAD0
    bra CODE_83FAD5             ; $83FAD3
CODE_83FAD5:
    clc                         ; $83FAD5
    rts                         ; $83FAD6
track_object_money:
    jsr.w func_83FC05           ; $83FAD7
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
    jsr.w func_83FE9A           ; $83FB07
    lda.w #$801E                ; $83FB0A
    sta.b $4A                   ; $83FB0D
    lda.w #$0047                ; $83FB0F
    sta.b $33                   ; $83FB12
CODE_83FB14:
    rts                         ; $83FB14
track_object_count:
    jsr.w func_83FC05           ; $83FB15
    bcc CODE_83FB49             ; $83FB18
    ldx.w $1F2E                 ; $83FB1A
    lda.b $4E                   ; $83FB1D
    and.w #$00FF                ; $83FB1F
    cmp.w #$000A                ; $83FB22
    beq CODE_83FB49             ; $83FB25
    inc.b $4E                   ; $83FB27
    ldx.w $011A                 ; $83FB29
    lda.w #$0000                ; $83FB2C
    sta.l $7E8CB3,x             ; $83FB2F
    ldx.w $1EBE                 ; $83FB33
    lda.w #$FF19                ; $83FB36
    ldy.w #$0000                ; $83FB39
    jsr.w func_83FE9A           ; $83FB3C
    lda.w #$801E                ; $83FB3F
    sta.b $4A                   ; $83FB42
    lda.w #$004C                ; $83FB44
    sta.b $33                   ; $83FB47
CODE_83FB49:
    rts                         ; $83FB49
track_object_random:
    jsr.w func_83FC05           ; $83FB4A
    bcc CODE_83FB75             ; $83FB4D
    ldx.w $011A                 ; $83FB4F
    lda.w #$0000                ; $83FB52
    sta.l $7E8CB3,x             ; $83FB55
    ldx.w $1EBE                 ; $83FB59
    lda.w #$FF19                ; $83FB5C
    ldy.w #$0000                ; $83FB5F
    jsr.w func_83FE9A           ; $83FB62
    lda.w #$801E                ; $83FB65
    sta.b $4A                   ; $83FB68
    jsl.l rng_next_long         ; $83FB6A
    and.w #$001E                ; $83FB6E
    tax                         ; $83FB71
    jmp.w ($FB76,x)             ; $83FB72
CODE_83FB75:
    rts                         ; $83FB75
    incbin !baserom:$1FB76..$1FB96
    lda.b $4E                   ; $83FB96 static
    and.w #$00FF                ; $83FB98 static
    cmp.w #$000A                ; $83FB9B static
    beq CODE_83FBA7             ; $83FB9E static
    inc.b $4E                   ; $83FBA0 static
    lda.w #$004C                ; $83FBA2 static
    sta.b $33                   ; $83FBA5 static
CODE_83FBA7:
    rts                         ; $83FBA7 static
    ldx.w #$0047                ; $83FBA8 static
    lda.w #$4000                ; $83FBAB static
    ldy.w #$0000                ; $83FBAE static
    jmp.w $FBCC                 ; $83FBB1 static
    ldx.w #$004D                ; $83FBB4 static
    lda.w #$0000                ; $83FBB7 static
    ldy.w #$0001                ; $83FBBA static
    jmp.w $FBCC                 ; $83FBBD static
    ldx.w #$0048                ; $83FBC0 static
    lda.w #$1000                ; $83FBC3 static
    ldy.w #$0000                ; $83FBC6 static
    jmp.w $FBCC                 ; $83FBC9 static
    stx.b $33                   ; $83FBCC static
    sed                         ; $83FBCE static
    ldx.w $1ECC                 ; $83FBCF static
    clc                         ; $83FBD2 static
    adc.w $038E,x               ; $83FBD3 static
    sta.w $038E,x               ; $83FBD6 static
    tya                         ; $83FBD9 static
    adc.w $0390,x               ; $83FBDA static
    sta.w $0390,x               ; $83FBDD static
    cld                         ; $83FBE0 static
    rts                         ; $83FBE1 static
    lda.w #$0028                ; $83FBE2
    sta.b $58                   ; $83FBE5
    lda.w #$FFFF                ; $83FBE7
    sta.b $56                   ; $83FBEA
    lda.w #$004A                ; $83FBEC
    sta.b $33                   ; $83FBEF
    rts                         ; $83FBF1
    stz.b $5A                   ; $83FBF2 static
    lda.w #$004B                ; $83FBF4 static
    sta.b $33                   ; $83FBF7 static
    rts                         ; $83FBF9 static
    lda.w #$0300                ; $83FBFA static
    sta.b $48                   ; $83FBFD static
    lda.w #$004E                ; $83FBFF static
    sta.b $33                   ; $83FC02 static
    rts                         ; $83FC04 static
func_83FC05:
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
    cmp.l $83826C,x             ; $83FC52
    bmi CODE_83FC37             ; $83FC56
    sec                         ; $83FC58
    rts                         ; $83FC59
CODE_83FC5A:
    ldx.w $011C                 ; $83FC5A
    cmp.l $83826E,x             ; $83FC5D
    bpl CODE_83FC37             ; $83FC61
    sec                         ; $83FC63
    rts                         ; $83FC64
track_object_test_sides:
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
    jsr.w func_83FD8A           ; $83FC9B
    bcs CODE_83FCCB             ; $83FC9E
    lda.w $0128                 ; $83FCA0
    eor.w #$FFFF                ; $83FCA3
    inc a                       ; $83FCA6
    sta.w $0128                 ; $83FCA7
    jsr.w func_83FD8A           ; $83FCAA
    bcs CODE_83FCCB             ; $83FCAD
    lda.w $0128                 ; $83FCAF
    sec                         ; $83FCB2
    sbc.w #$0080                ; $83FCB3
    sta.w $0128                 ; $83FCB6
    jsr.w func_83FD8A           ; $83FCB9
    bcs CODE_83FCCB             ; $83FCBC
    lda.w $0128                 ; $83FCBE
    eor.w #$FFFF                ; $83FCC1
    inc a                       ; $83FCC4
    sta.w $0128                 ; $83FCC5
    jsr.w func_83FD8A           ; $83FCC8
CODE_83FCCB:
    rts                         ; $83FCCB
    lda.w $0128                 ; $83FCCC
    jsr.w func_83FD8A           ; $83FCCF
    bcs CODE_83FCF1             ; $83FCD2
    lda.w $0128                 ; $83FCD4
    jsr.w func_83FDCD           ; $83FCD7
    bcs CODE_83FCF1             ; $83FCDA
    lda.w $0128                 ; $83FCDC
    eor.w #$FFFF                ; $83FCDF
    inc a                       ; $83FCE2
    sta.w $0128                 ; $83FCE3
    jsr.w func_83FD8A           ; $83FCE6
    bcs CODE_83FCF1             ; $83FCE9
    lda.w $0128                 ; $83FCEB
    jsr.w func_83FDCD           ; $83FCEE
CODE_83FCF1:
    rts                         ; $83FCF1
    lda.w $0128                 ; $83FCF2
    jsr.w func_83FDCD           ; $83FCF5
    bcs CODE_83FD25             ; $83FCF8
    lda.w $0128                 ; $83FCFA
    eor.w #$FFFF                ; $83FCFD
    inc a                       ; $83FD00
    sta.w $0128                 ; $83FD01
    jsr.w func_83FDCD           ; $83FD04
    bcs CODE_83FD25             ; $83FD07
    lda.w $0128                 ; $83FD09
    sec                         ; $83FD0C
    sbc.w #$0080                ; $83FD0D
    sta.w $0128                 ; $83FD10
    jsr.w func_83FDCD           ; $83FD13
    bcs CODE_83FD25             ; $83FD16
    lda.w $0128                 ; $83FD18
    eor.w #$FFFF                ; $83FD1B
    inc a                       ; $83FD1E
    sta.w $0128                 ; $83FD1F
    jsr.w func_83FDCD           ; $83FD22
CODE_83FD25:
    rts                         ; $83FD25
car_set_53_1:
    sep.b #$20                  ; $83FD26
    lda.b #$01                  ; $83FD28
    sta.b $53                   ; $83FD2A
    rep.b #$20                  ; $83FD2C
    jmp.w car_set_53_tail       ; $83FD2E
car_set_53_2:
    sep.b #$20                  ; $83FD31
    lda.b #$02                  ; $83FD33
    sta.b $53                   ; $83FD35
    rep.b #$20                  ; $83FD37
    jmp.w car_set_53_tail       ; $83FD39
car_clear_53:
    sep.b #$20                  ; $83FD3C
    lda.b #$00                  ; $83FD3E
    sta.b $53                   ; $83FD40
    rep.b #$20                  ; $83FD42
    rts                         ; $83FD44
car_set_53_tail:
    ldx.w $011A                 ; $83FD45
    lda.l $7E8CB3,x             ; $83FD48
    and.w #$00FF                ; $83FD4C
    sta.b $B1                   ; $83FD4F
    lda.l $7E8CB4,x             ; $83FD51
    and.w #$FF00                ; $83FD55
    ora.b $B1                   ; $83FD58
    sta.b $B1                   ; $83FD5A
    rts                         ; $83FD5C
    sep.b #$20                  ; $83FD5D
    lda.b #$03                  ; $83FD5F
    sta.b $53                   ; $83FD61
    rep.b #$20                  ; $83FD63
    jmp.w car_set_53_tail       ; $83FD65
car_object_stop:
    lda.b $44                   ; $83FD68
    bit.w #$0020                ; $83FD6A
    bne CODE_83FD89             ; $83FD6D
    sep.b #$20                  ; $83FD6F
    lda.b #$04                  ; $83FD71
    sta.b $53                   ; $83FD73
    rep.b #$20                  ; $83FD75
    ldx.w $011A                 ; $83FD77
    lda.l $7E8CB3,x             ; $83FD7A
    and.w #$00FF                ; $83FD7E
    sta.b $B1                   ; $83FD81
    lda.l $7E8CB5,x             ; $83FD83
    sta.b $5E                   ; $83FD87
CODE_83FD89:
    rts                         ; $83FD89
func_83FD8A:
    bit.w $0120                 ; $83FD8A
    bmi CODE_83FDAC             ; $83FD8D
    sec                         ; $83FD8F
    sbc.b $06                   ; $83FD90
    bpl CODE_83FDA0             ; $83FD92
    ldx.w $011C                 ; $83FD94
    cmp.l $8380CC,x             ; $83FD97
    bmi CODE_83FDAC             ; $83FD9B
    jmp.w $FDAE                 ; $83FD9D
CODE_83FDA0:
    ldx.w $011C                 ; $83FDA0
    cmp.l $8380CE,x             ; $83FDA3
    bpl CODE_83FDAC             ; $83FDA7
    jmp.w $FDAE                 ; $83FDA9
CODE_83FDAC:
    clc                         ; $83FDAC
    rts                         ; $83FDAD
    lda.w $012A                 ; $83FDAE
    sec                         ; $83FDB1
    sbc.l $81012C               ; $83FDB2
    bpl CODE_83FDC1             ; $83FDB6
    incbin !baserom:$1FDB8..$1FDC1
CODE_83FDC1:
    ldx.w $011C                 ; $83FDC1
    cmp.l $83826E,x             ; $83FDC4
    bpl CODE_83FDAC             ; $83FDC8
    jmp.w $FE16                 ; $83FDCA
func_83FDCD:
    bit.w $0120                 ; $83FDCD
    bmi CODE_83FDF5             ; $83FDD0
    bra CODE_83FDF5             ; $83FDD2
    incbin !baserom:$1FDD4..$1FDF5
CODE_83FDF5:
    clc                         ; $83FDF5
    rts                         ; $83FDF6
    incbin !baserom:$1FDF7..$1FE16
    lda.w #$0008                ; $83FE16
    trb.b $7C                   ; $83FE19
    ldx.w $1EBE                 ; $83FE1B
    lda.w #$FF23                ; $83FE1E
    ldy.w #$0000                ; $83FE21
    jsr.w func_83FE9A           ; $83FE24
    bit.w $1F31                 ; $83FE27
    bmi CODE_83FE55             ; $83FE2A
    lda.w #$0000                ; $83FE2C
    sta.l $308000               ; $83FE2F
    lda.b $0F                   ; $83FE33
    lsr a                       ; $83FE35
    sta.l $308000               ; $83FE36
    lda.w $1F30                 ; $83FE3A
    sta.l $308000               ; $83FE3D
    nop                         ; $83FE41
    lda.l $308000               ; $83FE42
    asl a                       ; $83FE46
    lda.l $308000               ; $83FE47
    rol a                       ; $83FE4B
    adc.b $5A                   ; $83FE4C
    bcc CODE_83FE53             ; $83FE4E
    lda.w #$FFFF                ; $83FE50
CODE_83FE53:
    sta.b $5A                   ; $83FE53
CODE_83FE55:
    lda.w $0128                 ; $83FE55
    bpl CODE_83FE7A             ; $83FE58
    lda.b $10                   ; $83FE5A
    cmp.w #$0060                ; $83FE5C
    bmi CODE_83FE6A             ; $83FE5F
    lda.b $1A                   ; $83FE61
    bne CODE_83FE6A             ; $83FE63
    lda.w #$0002                ; $83FE65
    sta.b $1A                   ; $83FE68
CODE_83FE6A:
    lda.w #$1000                ; $83FE6A
    sta.b $1C                   ; $83FE6D
    lda.b $18                   ; $83FE6F
    cmp.w #$8000                ; $83FE71
    ror.b $18                   ; $83FE74
    ror.b $16                   ; $83FE76
    sec                         ; $83FE78
    rts                         ; $83FE79
CODE_83FE7A:
    lda.b $10                   ; $83FE7A
    cmp.w #$0060                ; $83FE7C
    bmi CODE_83FE8A             ; $83FE7F
    lda.b $1A                   ; $83FE81
    bne CODE_83FE8A             ; $83FE83
    lda.w #$0003                ; $83FE85
    sta.b $1A                   ; $83FE88
CODE_83FE8A:
    lda.w #$F000                ; $83FE8A
    sta.b $1C                   ; $83FE8D
    lda.b $18                   ; $83FE8F
    cmp.w #$8000                ; $83FE91
    ror.b $18                   ; $83FE94
    ror.b $16                   ; $83FE96
    sec                         ; $83FE98
    rts                         ; $83FE99
func_83FE9A:
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
car_message_request:
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
