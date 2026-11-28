; bank $82 (file offset $010000)
org $828000

    incbin !baserom:$10000..$10342
race_wrapper_head:
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
    jsr.w wait_frames_82        ; $828359
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
    jsl.l track_settings_load   ; $82838B
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
race_wrapper_tail:
    lda.w $027C                 ; $8283BB
    sta.w $03AB                 ; $8283BE
    lda.w $032F                 ; $8283C1
    sta.w $03DA                 ; $8283C4
    lda.w #$0001                ; $8283C7
    jsr.w wait_frames_82        ; $8283CA
    stz.w $0026                 ; $8283CD
    stz.w $0196                 ; $8283D0
    stz.w $0198                 ; $8283D3
    stz.w $019A                 ; $8283D6
    stz.w $002A                 ; $8283D9
    stz.w $0028                 ; $8283DC
    stz.w $01FB                 ; $8283DF
    stz.w $01FC                 ; $8283E2
    stz.w $02FB                 ; $8283E5
    stz.w $02FC                 ; $8283E8
    stz.w $1B24                 ; $8283EB
    stz.w $1B26                 ; $8283EE
    lda.w #$8934                ; $8283F1
    sta.w $0008                 ; $8283F4
    lda.w #$8807                ; $8283F7
    sta.w $0001                 ; $8283FA
    sep.b #$20                  ; $8283FD
    lda.b #$80                  ; $8283FF
    sta.w $0003                 ; $828401
    rep.b #$20                  ; $828404
    lda.w #$89B2                ; $828406
    sta.w $0005                 ; $828409
    sep.b #$20                  ; $82840C
    lda.b #$80                  ; $82840E
    sta.w $0007                 ; $828410
    rep.b #$20                  ; $828413
    plb                         ; $828415
    pld                         ; $828416
    rtl                         ; $828417
    incbin !baserom:$10418..$1041E
wait_frames_82:
    clc                         ; $82841E
    adc.w $0024                 ; $82841F
CODE_828422:
    cmp.w $0024                 ; $828422
    bne CODE_828422             ; $828425
    rts                         ; $828427
race_load:
    ldx.w #$96DD                ; $828428
    lda.w #$0083                ; $82842B
    jsl.l video_load_screen_long; $82842E
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
    jsl.l track_settings_load   ; $828465
    lda.w $047B                 ; $828469
    jsl.l track_tiles_load      ; $82846C
    jsr.w race_state_reset      ; $828470
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
    jsr.w track_segments_load   ; $828493
    ldx.w #$85F5                ; $828496
    ldy.w #$5AC0                ; $828499
    lda.w #$008F                ; $82849C
    jsl.l decompress_to_vram_long; $82849F
    lda.w $01E4                 ; $8284A3
    bmi CODE_8284C8             ; $8284A6
    bne CODE_8284B9             ; $8284A8
    ldx.w #$9093                ; $8284AA
    ldy.w #$0000                ; $8284AD
    lda.w #$009C                ; $8284B0
    jsl.l decompress_to_vram_long; $8284B3
    bra CODE_8284D5             ; $8284B7
CODE_8284B9:
    ldx.w #$A04F                ; $8284B9
    ldy.w #$0000                ; $8284BC
    lda.w #$009C                ; $8284BF
    jsl.l decompress_to_vram_long; $8284C2
    bra CODE_8284D5             ; $8284C6
CODE_8284C8:
    ldx.w #$9F2A                ; $8284C8
    ldy.w #$0000                ; $8284CB
    lda.w #$009D                ; $8284CE
    jsl.l decompress_to_vram_long; $8284D1
CODE_8284D5:
    ldx.w #$8B3D                ; $8284D5
    ldy.w #$0000                ; $8284D8
    lda.w #$7F87                ; $8284DB
    jsl.l decompress            ; $8284DE
    lda.w $1F06                 ; $8284E2
    asl a                       ; $8284E5
    tax                         ; $8284E6
    lda.l $83A484,x             ; $8284E7
    sta.l $7F0078               ; $8284EB
    lda.w #$30F4                ; $8284EF
    jsr.w tile_block_5x5        ; $8284F2
    lda.w #$007F                ; $8284F5
    sta.b $14                   ; $8284F8
    ldx.w #$0000                ; $8284FA
    ldy.w #$7C00                ; $8284FD
    lda.w #$0700                ; $828500
    jsl.l upload_vram_now       ; $828503
    jsr.w track_backdrop_load   ; $828507
    jsr.w track_block_triple    ; $82850A
    lda.w #$007E                ; $82850D
    sta.b $14                   ; $828510
    ldx.w #$217C                ; $828512
    ldy.w #$57A0                ; $828515
    lda.w #$0190                ; $828518
    jsl.l upload_vram_now       ; $82851B
    lda.w #$007E                ; $82851F
    sta.b $14                   ; $828522
    ldx.w #$217C                ; $828524
    ldy.w #$5868                ; $828527
    lda.w #$0190                ; $82852A
    jsl.l upload_vram_now       ; $82852D
    lda.w #$8D00                ; $828531
    ldx.w #$8B70                ; $828534
    ldy.w #$0040                ; $828537
    jsl.l upload_cgram_now      ; $82853A
    ldx.w #$B3D5                ; $82853E
    ldy.w #$0EE0                ; $828541
    lda.w #$0099                ; $828544
    jsl.l decompress_to_vram_long; $828547
race_sound:
    lda.w $047C                 ; $82854B
    bmi race_track_build        ; $82854E
    jsl.l wait_03e0             ; $828550
    jsl.l sound_load_race       ; $828554
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
race_track_build:
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
    jsl.l track_map_load        ; $8285C5
    jsr.w track_setup_load      ; $8285C9
    jsl.l track_rows_build      ; $8285CC
    jsr.w track_objects_stream  ; $8285D0
    jsr.w hud_markers_write     ; $8285D3
race_track_start:
    lda.w #$0001                ; $8285D6
    jsr.w wait_frames_82        ; $8285D9
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
    jsr.w road_rows_build_call  ; $828605
    jsl.l race_road_frame_alt   ; $828608
race_track_start_tail:
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
    jsr.w wait_frames_82        ; $82862A
    lda.w #$0012                ; $82862D
    ldx.w #$4000                ; $828630
    jsr.w func_82AD12           ; $828633
    cli                         ; $828636
    lda.w #$003D                ; $828637
    bit.w $047C                 ; $82863A
    bpl CODE_828642             ; $82863D
    lda.w #$0043                ; $82863F
CODE_828642:
    sta.w $0233                 ; $828642
race_frame_head:
    jsr.w race_countdown        ; $828645
    jsl.l player1_frame         ; $828648
    jsl.l race_cars_frame       ; $82864C
    jsr.w func_82AC52           ; $828650
    lda.w #$0001                ; $828653
race_frame_wait:
    bit.w $00B8                 ; $828656
    bne race_frame_wait         ; $828659
race_frame_body:
    jsr.w player_segment_step   ; $82865B
    jsr.w road_rows_build_call  ; $82865E
    jsr.w road_shift_frame_call ; $828661
    jsr.w race_positions_update ; $828664
    lda.w $00B8                 ; $828667
    bit.w #$0004                ; $82866A
    bne CODE_828678             ; $82866D
    jsr.w race_hud_digits       ; $82866F
    lda.w #$0004                ; $828672
    tsb.w $00B8                 ; $828675
CODE_828678:
    jsr.w race_frame_draw       ; $828678
race_frame_rest:
    lda.w #$0001                ; $82867B
    tsb.w $00B8                 ; $82867E
    ldx.w $00BA                 ; $828681
    bit.w $047C                 ; $828684
    bpl CODE_8286AB             ; $828687
    lda.w $000C                 ; $828689
    ora.w $0010                 ; $82868C
    ora.w $0014                 ; $82868F
    ora.w $0018                 ; $828692
    bit.w #$FFF0                ; $828695
    beq CODE_8286BE             ; $828698
    lda.w #$007F                ; $82869A
    trb.w $047D                 ; $82869D
    stz.w $0148                 ; $8286A0
    lda.w #$FF80                ; $8286A3
    sta.w $0146                 ; $8286A6
    bra CODE_8286BE             ; $8286A9
CODE_8286AB:
    lda.w #$1000                ; $8286AB
    trb.w $000E                 ; $8286AE
    beq CODE_8286BE             ; $8286B1
    lda.w $014A                 ; $8286B3
    cmp.w #$0F00                ; $8286B6
    bne CODE_8286BE             ; $8286B9
    jsr.w race_pause            ; $8286BB
CODE_8286BE:
    lda.w $03E2                 ; $8286BE
    bit.w #$0004                ; $8286C1
    beq CODE_8286D1             ; $8286C4
    lda.w #$2000                ; $8286C6
    trb.w $000E                 ; $8286C9
    beq CODE_8286D1             ; $8286CC
    incbin !baserom:$106CE..$106D1
CODE_8286D1:
    stz.w $00BA                 ; $8286D1
    stx.w $1E9E                 ; $8286D4
    lda.w $0244                 ; $8286D7
    bit.w #$0004                ; $8286DA
    bne CODE_8286E7             ; $8286DD
    lda.w $1F08                 ; $8286DF
    cmp.w #$000A                ; $8286E2
    bcc CODE_828766             ; $8286E5
CODE_8286E7:
    lda.w $1ED0                 ; $8286E7
    bne CODE_828736             ; $8286EA
    lda.w $0244                 ; $8286EC
    bit.w #$0004                ; $8286EF
    bne CODE_8286FA             ; $8286F2
    lda.w #$003C                ; $8286F4
    sta.w $0233                 ; $8286F7
CODE_8286FA:
    pea.w $837E                 ; $8286FA
    plb                         ; $8286FD
    ldy.w #$0000                ; $8286FE
    ldx.w #$0000                ; $828701
CODE_828704:
    lda.w $FAB5,x               ; $828704
    sta.w $6991,y               ; $828707
    lda.w $FAB7,x               ; $82870A
    sta.w $69B9,y               ; $82870D
    lda.w $FAD3,x               ; $828710
    sta.w $69E1,y               ; $828713
    lda.w $FADE,x               ; $828716
    sta.w $6A09,y               ; $828719
    iny                         ; $82871C
    iny                         ; $82871D
    txa                         ; $82871E
    clc                         ; $82871F
    adc.w #$0035                ; $828720
    tax                         ; $828723
    cpy.w #$0028                ; $828724
    bne CODE_828704             ; $828727
    lda.w $1F08                 ; $828729
    sta.w $6A31                 ; $82872C
    plb                         ; $82872F
    lda.w #$7F10                ; $828730
    sta.w $1ED0                 ; $828733
CODE_828736:
    bmi CODE_828759             ; $828736
    lda.w $1ED0                 ; $828738
    clc                         ; $82873B
    adc.w $1E9E                 ; $82873C
    sta.w $1ED0                 ; $82873F
    bpl CODE_828759             ; $828742
    stz.w $0148                 ; $828744
    lda.w #$FF80                ; $828747
    sta.w $0146                 ; $82874A
    bit.w $047C                 ; $82874D
    bmi CODE_828759             ; $828750
    lda.w #$FFFF                ; $828752
    sta.l $8103E0               ; $828755
CODE_828759:
    lda.w $0146                 ; $828759
    bne CODE_828766             ; $82875C
    lda.w $014A                 ; $82875E
    and.w #$0F00                ; $828761
    beq race_end                ; $828764
CODE_828766:
    lda.w $0146                 ; $828766
    bne CODE_828773             ; $828769
    lda.w $014A                 ; $82876B
    and.w #$0F00                ; $82876E
    beq race_end                ; $828771
CODE_828773:
    jmp.w race_frame_head       ; $828773
race_end:
    bit.w $047C                 ; $828776
    bmi CODE_82877F             ; $828779
    jsl.l race_order_sort       ; $82877B
CODE_82877F:
    stz.w $420B                 ; $82877F
    sei                         ; $828782
    lda.w #$0001                ; $828783
    jsr.w wait_frames_82        ; $828786
    rts                         ; $828789
    incbin !baserom:$1078A..$1084B
race_pause:
    phx                         ; $82884B
    lda.w #$0010                ; $82884C
    tsb.w $0026                 ; $82884F
    lda.w #$FFFC                ; $828852
    sta.w $03E0                 ; $828855
    lda.w $03E2                 ; $828858
    bit.w #$0004                ; $82885B
    beq CODE_82886E             ; $82885E
    ldx.w #$8000                ; $828860
    lda.w $1E94                 ; $828863
    bpl CODE_82886B             ; $828866
    ldx.w $047B                 ; $828868
CODE_82886B:
    stx.w $1E94                 ; $82886B
CODE_82886E:
    lda.l $7E4E4D               ; $82886E
    pha                         ; $828872
    lda.w #$0000                ; $828873
    sta.l $7E4E4D               ; $828876
CODE_82887A:
    ldx.w #$0088                ; $82887A
    lda.w #$4E61                ; $82887D
    jsr.w text_centred_line_at  ; $828880
    lda.w #$0004                ; $828883
    tsb.w $00B8                 ; $828886
    ldx.w #$0040                ; $828889
CODE_82888C:
    lda.w #$1000                ; $82888C
    trb.w $000E                 ; $82888F
    bne CODE_8288F2             ; $828892
    lda.w #$0001                ; $828894
    jsr.w wait_frames_82        ; $828897
    lda.w #$0030                ; $82889A
    and.w $000C                 ; $82889D
    eor.w #$0030                ; $8288A0
    db $F0,$33 ; beq            ; $8288A3
    dex                         ; $8288A5
    bne CODE_82888C             ; $8288A6
    ldx.w #$008C                ; $8288A8
    lda.w #$4E61                ; $8288AB
    jsr.w text_centred_line_at  ; $8288AE
    lda.w #$0004                ; $8288B1
    tsb.w $00B8                 ; $8288B4
    ldx.w #$0040                ; $8288B7
CODE_8288BA:
    lda.w #$1000                ; $8288BA
    trb.w $000E                 ; $8288BD
    bne CODE_8288F2             ; $8288C0
    lda.w #$0030                ; $8288C2
    and.w $000C                 ; $8288C5
    eor.w #$0030                ; $8288C8
    db $F0,$0B ; beq            ; $8288CB
    lda.w #$0001                ; $8288CD
    jsr.w wait_frames_82        ; $8288D0
    dex                         ; $8288D3
    bne CODE_8288BA             ; $8288D4
    bra CODE_82887A             ; $8288D6
    incbin !baserom:$108D8..$108F2
CODE_8288F2:
    lda.l $810233               ; $8288F2
    asl a                       ; $8288F6
    tax                         ; $8288F7
    lda.w #$4E61                ; $8288F8
    jsr.w text_centred_line_at  ; $8288FB
    pla                         ; $8288FE
    sta.l $7E4E4D               ; $8288FF
    lda.w #$0004                ; $828903
    sta.w $03E0                 ; $828906
    lda.w #$0010                ; $828909
    trb.w $0026                 ; $82890C
    plx                         ; $82890F
    rts                         ; $828910
text_centred_line_at:
    pea.w $837E                 ; $828911
    plb                         ; $828914
    sta.b $18                   ; $828915
    stx.b $3C                   ; $828917
    lda.w #$0C00                ; $828919
    sta.b $3A                   ; $82891C
    jsr.w text_centred_line     ; $82891E
    plb                         ; $828921
    rts                         ; $828922
race_frame_draw:
    stz.b $86                   ; $828923
    stz.b $8E                   ; $828925
    lda.w $00B4                 ; $828927
    beq CODE_828933             ; $82892A
    lda.w #$0220                ; $82892C
    sta.b $86                   ; $82892F
    sta.b $8E                   ; $828931
CODE_828933:
    jsl.l race_road_frame       ; $828933
    jsl.l road_rows_stream      ; $828937
    jsr.w shake_step            ; $82893B
    jsr.w object_sprites_stream ; $82893E
    jsr.w func_82A9B7           ; $828941
    jmp.w oam_high_load         ; $828944
    incbin !baserom:$10947..$10961
hud_markers_write:
    lda.w #$7C57                ; $828961
    sta.w $2116                 ; $828964
    ldx.w #$0000                ; $828967
    ldy.w #$0157                ; $82896A
CODE_82896D:
    lda.l $810398,x             ; $82896D
    and.w #$00FF                ; $828971
    beq CODE_82897B             ; $828974
    lda.w $2139                 ; $828976
    bra CODE_82897E             ; $828979
CODE_82897B:
    sty.w $2118                 ; $82897B
CODE_82897E:
    inx                         ; $82897E
    cpx.w #$0004                ; $82897F
    bne CODE_82896D             ; $828982
    lda.w $03A7                 ; $828984
    and.w #$00FF                ; $828987
    beq CODE_8289B2             ; $82898A
    lda.w #$7F45                ; $82898C
    sta.w $2116                 ; $82898F
    lda.l $83A4FC               ; $828992
    sta.l $002118               ; $828996
    lda.l $83A4FE               ; $82899A
    sta.l $002118               ; $82899E
    lda.l $83A500               ; $8289A2
    sta.l $002118               ; $8289A6
    lda.l $83A502               ; $8289AA
    sta.l $002118               ; $8289AE
CODE_8289B2:
    jmp.w race_hud_digits       ; $8289B2
    ldx.w #$96DD                ; $8289B5
    lda.w #$0083                ; $8289B8
    jsl.l video_load_screen_long; $8289BB
    lda.w #$0001                ; $8289BF
    jsr.w wait_frames_82        ; $8289C2
    lda.w $4210                 ; $8289C5
    lda.w #$8B05                ; $8289C8
    sta.w $0008                 ; $8289CB
    sep.b #$20                  ; $8289CE
    ldx.w #$95BD                ; $8289D0
    stx.w $0005                 ; $8289D3
    lda.b #$85                  ; $8289D6
    sta.w $0007                 ; $8289D8
    ldx.w #$8AF2                ; $8289DB
    stx.w $0001                 ; $8289DE
    lda.b #$85                  ; $8289E1
    sta.w $0003                 ; $8289E3
    rep.b #$20                  ; $8289E6
    sep.b #$20                  ; $8289E8
    lda.b #$25                  ; $8289EA
    sta.w $2132                 ; $8289EC
    lda.b #$4B                  ; $8289EF
    sta.w $2132                 ; $8289F1
    lda.b #$8A                  ; $8289F4
    sta.w $2132                 ; $8289F6
    rep.b #$20                  ; $8289F9
    jsl.l track_settings_load   ; $8289FB
    jsr.w race_state_reset      ; $8289FF
    jsr.w track_segments_load   ; $828A02
    ldx.w #$85F5                ; $828A05
    ldy.w #$5AC0                ; $828A08
    lda.w #$008F                ; $828A0B
    jsl.l decompress_to_vram_long; $828A0E
    lda.w $01E4                 ; $828A12
    bmi CODE_828A37             ; $828A15
    bne CODE_828A28             ; $828A17
    incbin !baserom:$10A19..$10A28
CODE_828A28:
    ldx.w #$A04F                ; $828A28
    ldy.w #$0000                ; $828A2B
    lda.w #$009C                ; $828A2E
    jsl.l decompress_to_vram_long; $828A31
    bra CODE_828A44             ; $828A35
CODE_828A37:
    ldx.w #$9F2A                ; $828A37
    ldy.w #$0000                ; $828A3A
    lda.w #$009D                ; $828A3D
    jsl.l decompress_to_vram_long; $828A40
CODE_828A44:
    ldx.w #$852E                ; $828A44
    ldy.w #$0000                ; $828A47
    lda.w #$7F8F                ; $828A4A
    jsl.l decompress            ; $828A4D
    lda.w $1F06                 ; $828A51
    asl a                       ; $828A54
    tax                         ; $828A55
    lda.l $83A484,x             ; $828A56
    sta.l $7F0078               ; $828A5A
    lda.w #$30F4                ; $828A5E
    jsr.w tile_block_5x5        ; $828A61
    lda.w #$007F                ; $828A64
    sta.b $14                   ; $828A67
    ldx.w #$0000                ; $828A69
    ldy.w #$7C00                ; $828A6C
    lda.w #$0380                ; $828A6F
    jsl.l upload_vram_now       ; $828A72
    lda.w #$30F4                ; $828A76
    jsr.w tile_block_5x5        ; $828A79
    lda.w #$007F                ; $828A7C
    sta.b $14                   ; $828A7F
    ldx.w #$0000                ; $828A81
    ldy.w #$7DC0                ; $828A84
    lda.w #$0380                ; $828A87
    jsl.l upload_vram_now       ; $828A8A
    jsr.w track_backdrop_load   ; $828A8E
    jsr.w track_block_triple    ; $828A91
    lda.w #$007E                ; $828A94
    sta.b $14                   ; $828A97
    ldx.w #$217C                ; $828A99
    ldy.w #$57A0                ; $828A9C
    lda.w #$0190                ; $828A9F
    jsl.l upload_vram_now       ; $828AA2
    lda.w #$007E                ; $828AA6
    sta.b $14                   ; $828AA9
    ldx.w #$217C                ; $828AAB
    ldy.w #$5868                ; $828AAE
    lda.w #$0190                ; $828AB1
    jsl.l upload_vram_now       ; $828AB4
    lda.w #$8D00                ; $828AB8
    ldx.w #$8B70                ; $828ABB
    ldy.w #$0040                ; $828ABE
    jsl.l upload_cgram_now      ; $828AC1
    ldx.w #$B3D5                ; $828AC5
    ldy.w #$0EE0                ; $828AC8
    lda.w #$0099                ; $828ACB
    jsl.l decompress_to_vram_long; $828ACE
    lda.w $047B                 ; $828AD2
    jsl.l track_tiles_load      ; $828AD5
    lda.w $047C                 ; $828AD9
    bmi CODE_828AFB             ; $828ADC
    incbin !baserom:$10ADE..$10AFB
CODE_828AFB:
    lda.w #$0010                ; $828AFB
    sta.w $002E                 ; $828AFE
    lda.w #$0000                ; $828B01
    sta.w $0030                 ; $828B04
    stz.w $0212                 ; $828B07
    stz.w $02C5                 ; $828B0A
    stz.w $003E                 ; $828B0D
    stz.w $0040                 ; $828B10
    stz.b $3E                   ; $828B13
    stz.b $40                   ; $828B15
    stz.w $005A                 ; $828B17
    stz.w $005C                 ; $828B1A
    stz.w $005D                 ; $828B1D
    stz.w $005F                 ; $828B20
    lda.w #$FFFF                ; $828B23
    sta.w $0235                 ; $828B26
    sta.w $02E8                 ; $828B29
    lda.w #$0003                ; $828B2C
    sta.w $0060                 ; $828B2F
    sta.w $0062                 ; $828B32
    lda.w #$000C                ; $828B35
    sta.w $0142                 ; $828B38
    lda.w #$000B                ; $828B3B
    sta.w $0144                 ; $828B3E
    stz.b $50                   ; $828B41
    stz.b $52                   ; $828B43
    stz.b $54                   ; $828B45
    stz.b $56                   ; $828B47
    stz.b $58                   ; $828B49
    jsl.l track_map_load        ; $828B4B
    jsl.l $859DAD               ; $828B4F
    jsr.w track_setup_load      ; $828B53
    jsl.l track_rows_build      ; $828B56
    jsr.w track_objects_stream  ; $828B5A
    jsr.w $B026                 ; $828B5D
    jsr.w $B234                 ; $828B60
    stz.w $4315                 ; $828B63
    stz.w $4325                 ; $828B66
    stz.w $4335                 ; $828B69
    lda.w #$0001                ; $828B6C
    jsr.w wait_frames_82        ; $828B6F
    lda.w $4210                 ; $828B72
    lda.w #$8B49                ; $828B75
    sta.w $0008                 ; $828B78
    sep.b #$20                  ; $828B7B
    ldx.w #$95BD                ; $828B7D
    stx.w $0005                 ; $828B80
    lda.b #$85                  ; $828B83
    sta.w $0007                 ; $828B85
    ldx.w #$8AF2                ; $828B88
    stx.w $0001                 ; $828B8B
    lda.b #$85                  ; $828B8E
    sta.w $0003                 ; $828B90
    rep.b #$20                  ; $828B93
    jsr.w $DFA2                 ; $828B95
    jsl.l $85BED1               ; $828B98
    lda.w #$0F00                ; $828B9C
    sta.w $0148                 ; $828B9F
    lda.w #$0080                ; $828BA2
    sta.w $0146                 ; $828BA5
    lda.w #$8002                ; $828BA8
    sta.w $1ECE                 ; $828BAB
    lda.w #$0001                ; $828BAE
    jsr.w wait_frames_82        ; $828BB1
    stz.w $00BA                 ; $828BB4
    lda.w #$0001                ; $828BB7
    sta.w $1E9E                 ; $828BBA
    lda.w #$0011                ; $828BBD
    ldx.w #$1000                ; $828BC0
    jsr.w func_82AD12           ; $828BC3
    lda.w #$0001                ; $828BC6
    jsr.w wait_frames_82        ; $828BC9
    lda.w #$0011                ; $828BCC
    ldx.w #$4000                ; $828BCF
    jsr.w func_82AD12           ; $828BD2
    jsr.w mode1_frame_draw_view2; $828BD5
    lda.w #$0002                ; $828BD8
    tsb.w $00B8                 ; $828BDB
CODE_828BDE:
    bit.w $00B8                 ; $828BDE
    bne CODE_828BDE             ; $828BE1
    jsr.w mode1_frame_draw_view1; $828BE3
    lda.w #$0001                ; $828BE6
    tsb.w $00B8                 ; $828BE9
CODE_828BEC:
    bit.w $00B8                 ; $828BEC
    bne CODE_828BEC             ; $828BEF
    cli                         ; $828BF1
    bit.w $047C                 ; $828BF2
    bpl CODE_828C00             ; $828BF5
    lda.w #$0043                ; $828BF7
    sta.w $0233                 ; $828BFA
    sta.w $02E6                 ; $828BFD
CODE_828C00:
    jsr.w race_countdown        ; $828C00
    jsl.l $83E25F               ; $828C03
    jsl.l race_cars_frame       ; $828C07
    jsr.w mode1_car_bands       ; $828C0B
    jsr.w player_segment_step   ; $828C0E
    jsr.w player2_segment_step  ; $828C11
    jsr.w $DFA2                 ; $828C14
    jsr.w $EC2B                 ; $828C17
    jsr.w race_positions_update ; $828C1A
    lda.w $00B8                 ; $828C1D
    bit.w #$0004                ; $828C20
    bne CODE_828C2E             ; $828C23
    jsr.w $B2D6                 ; $828C25
    lda.w #$0004                ; $828C28
    tsb.w $00B8                 ; $828C2B
CODE_828C2E:
    jsr.w mode1_frame_draw_view2; $828C2E
    lda.w #$0002                ; $828C31
    tsb.w $00B8                 ; $828C34
    lda.w #$0001                ; $828C37
CODE_828C3A:
    bit.w $00B8                 ; $828C3A
    bne CODE_828C3A             ; $828C3D
    jsr.w mode1_frame_draw_view1; $828C3F
    lda.w #$0001                ; $828C42
    tsb.w $00B8                 ; $828C45
    lda.w #$0002                ; $828C48
CODE_828C4B:
    bit.w $00B8                 ; $828C4B
    bne CODE_828C4B             ; $828C4E
    ldx.w $00BA                 ; $828C50
    bit.w $047C                 ; $828C53
    db $10,$22 ; bpl            ; $828C56
    lda.w $000C                 ; $828C58
    ora.w $0010                 ; $828C5B
    ora.w $0014                 ; $828C5E
    ora.w $0018                 ; $828C61
    bit.w #$FFF0                ; $828C64
    beq CODE_828C92             ; $828C67
    lda.w #$007F                ; $828C69
    trb.w $047D                 ; $828C6C
    stz.w $0148                 ; $828C6F
    lda.w #$FF80                ; $828C72
    sta.w $0146                 ; $828C75
    bra CODE_828C92             ; $828C78
    incbin !baserom:$10C7A..$10C92
CODE_828C92:
    lda.w $03E2                 ; $828C92
    bit.w #$0004                ; $828C95
    beq CODE_828CA5             ; $828C98
    incbin !baserom:$10C9A..$10CA5
CODE_828CA5:
    stz.w $00BA                 ; $828CA5
    stx.w $1E9E                 ; $828CA8
    lda.w $0244                 ; $828CAB
    eor.w $02F7                 ; $828CAE
    eor.w #$0004                ; $828CB1
    and.w $0244                 ; $828CB4
    bit.w #$0004                ; $828CB7
    db $D0,$0B ; bne            ; $828CBA
    lda.w $1F08                 ; $828CBC
    cmp.w #$000A                ; $828CBF
    db $B0,$03 ; bcs            ; $828CC2
    jmp.w $8D54                 ; $828CC4
    incbin !baserom:$10CC7..$10D54
    lda.w $0146                 ; $828D54
    bne CODE_828D61             ; $828D57
    lda.w $014A                 ; $828D59
    and.w #$0F00                ; $828D5C
    beq CODE_828D64             ; $828D5F
CODE_828D61:
    jmp.w $8C00                 ; $828D61
CODE_828D64:
    bit.w $047C                 ; $828D64
    bmi CODE_828D6D             ; $828D67
    incbin !baserom:$10D69..$10D6D
CODE_828D6D:
    stz.w $420B                 ; $828D6D
    sei                         ; $828D70
    lda.w #$0001                ; $828D71
    jsr.w wait_frames_82        ; $828D74
    rts                         ; $828D77
    incbin !baserom:$10D78..$10F47
mode1_frame_draw_view1:
    stz.b $86                   ; $828F47
    stz.b $8E                   ; $828F49
    lda.w $00B4                 ; $828F4B
    beq CODE_828F57             ; $828F4E
    lda.w #$0220                ; $828F50
    sta.b $86                   ; $828F53
    sta.b $8E                   ; $828F55
CODE_828F57:
    jsl.l mode1_road_frame_view1; $828F57
    jsl.l mode1_window_view1    ; $828F5B
    jsr.w shake_step            ; $828F5F
    jsr.w mode1_view1_sprites   ; $828F62
    jsr.w func_82A9B7           ; $828F65
    jmp.w oam_high_load         ; $828F68
mode1_frame_draw_view2:
    stz.b $88                   ; $828F6B
    stz.b $8E                   ; $828F6D
    lda.w $00B6                 ; $828F6F
    beq CODE_828F7B             ; $828F72
    lda.w #$0220                ; $828F74
    sta.b $88                   ; $828F77
    sta.b $8E                   ; $828F79
CODE_828F7B:
    jsl.l mode1_road_frame_view2; $828F7B
    jsl.l mode1_window_view2    ; $828F7F
    jsr.w shake_step2           ; $828F83
    jsr.w mode1_view2_sprites   ; $828F86
    jsr.w func_82AA7B           ; $828F89
    jmp.w oam_high_load2        ; $828F8C
shake_step:
    lda.w $0231                 ; $828F8F
    beq CODE_828FAE             ; $828F92
    lda.w $1F18                 ; $828F94
    bpl CODE_828FB5             ; $828F97
    ldx.w #$0000                ; $828F99
    lda.w $0397                 ; $828F9C
    and.w #$00FF                ; $828F9F
    cmp.w #$0004                ; $828FA2
    bcc CODE_828FAA             ; $828FA5
    ldx.w #$0030                ; $828FA7
CODE_828FAA:
    stx.w $1F18                 ; $828FAA
    rts                         ; $828FAD
CODE_828FAE:
    lda.w #$8000                ; $828FAE
    sta.w $1F18                 ; $828FB1
    rts                         ; $828FB4
CODE_828FB5:
    cmp.w #$0098                ; $828FB5
    bcs CODE_828FF9             ; $828FB8
    lda.w #$FFFD                ; $828FBA
    sta.w $1F1C                 ; $828FBD
    lda.w #$0002                ; $828FC0
    sta.w $1F20                 ; $828FC3
    lda.w $1F18                 ; $828FC6
    and.w #$FFF8                ; $828FC9
    tax                         ; $828FCC
    lda.w $1E9E                 ; $828FCD
    asl a                       ; $828FD0
    clc                         ; $828FD1
    adc.w $1F18                 ; $828FD2
    sta.w $1F18                 ; $828FD5
    and.w #$FFF8                ; $828FD8
    cmp.w #$0098                ; $828FDB
    bcs CODE_828FF2             ; $828FDE
    cmp.w #$0030                ; $828FE0
    bcc CODE_828FEA             ; $828FE3
    cpx.w #$0030                ; $828FE5
    bcc CODE_828FEB             ; $828FE8
CODE_828FEA:
    rts                         ; $828FEA
CODE_828FEB:
    lda.w #$0098                ; $828FEB
    sta.w $1F18                 ; $828FEE
    rts                         ; $828FF1
CODE_828FF2:
    lda.w #$00B0                ; $828FF2
    sta.w $1F18                 ; $828FF5
    rts                         ; $828FF8
CODE_828FF9:
    lda.w #$0000                ; $828FF9
    sta.l $308000               ; $828FFC
    jsl.l rng_next_long         ; $829000
    lsr a                       ; $829004
    sta.l $308000               ; $829005
    lda.w #$FFF6                ; $829009
    sta.l $308000               ; $82900C
    nop                         ; $829010
    lda.l $308000               ; $829011
    lda.l $308000               ; $829015
    sta.w $1F1C                 ; $829019
    lda.w #$0000                ; $82901C
    sta.l $308000               ; $82901F
    jsl.l rng_next_long         ; $829023
    lsr a                       ; $829027
    sta.l $308000               ; $829028
    lda.w #$000A                ; $82902C
    sta.l $308000               ; $82902F
    nop                         ; $829033
    lda.l $308000               ; $829034
    lda.l $308000               ; $829038
    sta.w $1F20                 ; $82903C
    jsl.l rng_next_long         ; $82903F
    and.w #$0006                ; $829043
    cmp.w #$0006                ; $829046
    beq CODE_828FF9             ; $829049
    tax                         ; $82904B
    lda.w $1F18                 ; $82904C
    cmp.w #$00B0                ; $82904F
    bcs CODE_82905C             ; $829052
    lda.l $829139,x             ; $829054
    sta.w $1F18                 ; $829058
    rts                         ; $82905B
CODE_82905C:
    lda.l $82913F,x             ; $82905C
    sta.w $1F18                 ; $829060
    rts                         ; $829063
shake_step2:
    lda.w $02E4                 ; $829064
    beq CODE_829083             ; $829067
    incbin !baserom:$11069..$11083
CODE_829083:
    lda.w #$8000                ; $829083
    sta.w $1F1A                 ; $829086
    rts                         ; $829089
    incbin !baserom:$1108A..$11145
mode1_car_bands:
    lda.w #$0000                ; $829145
    sta.l $308000               ; $829148
    lda.w $0213                 ; $82914C
    sta.l $308000               ; $82914F
    lda.w #$0300                ; $829153
    sta.l $308000               ; $829156
    lda.w #$FF00                ; $82915A
    and.l $308000               ; $82915D
    xba                         ; $829161
    sta.b $14                   ; $829162
    lda.l $308000               ; $829164
    xba                         ; $829168
    and.w #$FF00                ; $829169
    ora.b $14                   ; $82916C
    sec                         ; $82916E
    sbc.w #$0B00                ; $82916F
    cmp.w #$3000                ; $829172
    bmi CODE_82917A             ; $829175
    incbin !baserom:$11177..$1117A
CODE_82917A:
    sta.l $7E4E2D               ; $82917A
    lda.w $0244                 ; $82917E
    lsr a                       ; $829181
    bcc CODE_8291FF             ; $829182
    lda.l $7EFAB4               ; $829184
    cmp.w $0224                 ; $829188
    lda.l $7EFAB6               ; $82918B
    sbc.w $0226                 ; $82918F
    bpl CODE_829197             ; $829192
    incbin !baserom:$11194..$11197
CODE_829197:
    sta.b $14                   ; $829197
    lda.w #$0058                ; $829199
    sec                         ; $82919C
    sbc.b $14                   ; $82919D
    bpl CODE_8291A4             ; $82919F
    incbin !baserom:$111A1..$111A4
CODE_8291A4:
    and.w #$00F8                ; $8291A4
    sta.l $7E4E35               ; $8291A7
    lda.w $0244                 ; $8291AB
    bit.w #$0004                ; $8291AE
    beq CODE_8291FF             ; $8291B1
    incbin !baserom:$111B3..$111FF
CODE_8291FF:
    lda.w #$0000                ; $8291FF
    sta.l $308000               ; $829202
    lda.w $02C6                 ; $829206
    sta.l $308000               ; $829209
    lda.w #$0300                ; $82920D
    sta.l $308000               ; $829210
    lda.w #$FF00                ; $829214
    and.l $308000               ; $829217
    xba                         ; $82921B
    sta.b $14                   ; $82921C
    lda.l $308000               ; $82921E
    xba                         ; $829222
    and.w #$FF00                ; $829223
    ora.b $14                   ; $829226
    sec                         ; $829228
    sbc.w #$0B00                ; $829229
    cmp.w #$3000                ; $82922C
    bmi CODE_829234             ; $82922F
    incbin !baserom:$11231..$11234
CODE_829234:
    sta.l $7E4E2F               ; $829234
    lda.w $02F7                 ; $829238
    lsr a                       ; $82923B
    bcc CODE_8292B9             ; $82923C
    lda.l $7EFAE9               ; $82923E
    cmp.w $02D7                 ; $829242
    lda.l $7EFAEB               ; $829245
    sbc.w $02D9                 ; $829249
    bpl CODE_829251             ; $82924C
    incbin !baserom:$1124E..$11251
CODE_829251:
    sta.b $14                   ; $829251
    lda.w #$0058                ; $829253
    sec                         ; $829256
    sbc.b $14                   ; $829257
    bpl CODE_82925E             ; $829259
    lda.w #$0000                ; $82925B
CODE_82925E:
    and.w #$00F8                ; $82925E
    sta.l $7E4E37               ; $829261
    lda.w $02F7                 ; $829265
    bit.w #$0004                ; $829268
    beq CODE_8292B9             ; $82926B
    incbin !baserom:$1126D..$112B9
CODE_8292B9:
    rts                         ; $8292B9
vs_race_load:
    ldx.w #$9EEA                ; $8292BA
    lda.w #$0083                ; $8292BD
    jsl.l video_load_screen_long; $8292C0
    lda.w $047B                 ; $8292C4
    clc                         ; $8292C7
    adc.w #$002F                ; $8292C8
    sta.w $047B                 ; $8292CB
    jsl.l track_settings_load   ; $8292CE
    jsr.w vs_race_reset         ; $8292D2
    ldx.w #$8125                ; $8292D5
    ldy.w #$0000                ; $8292D8
    lda.w #$009B                ; $8292DB
    jsl.l decompress_to_vram_long; $8292DE
    ldx.w #$8080                ; $8292E2
    ldy.w #$0000                ; $8292E5
    lda.w #$7F9B                ; $8292E8
    jsl.l decompress            ; $8292EB
    jsr.w track_backdrop_load   ; $8292EF
    lda.w #$8E00                ; $8292F2
    ldx.w #$8000                ; $8292F5
    ldy.w #$0040                ; $8292F8
    jsl.l upload_cgram_now      ; $8292FB
    ldx.w #$8E04                ; $8292FF
    ldy.w #$0E40                ; $829302
    lda.w #$0087                ; $829305
    jsl.l decompress_to_vram_long; $829308
    lda.w $047B                 ; $82930C
    jsl.l track_tiles_load      ; $82930F
    ldx.w #$8080                ; $829313
    ldy.w #$7C00                ; $829316
    lda.w #$009B                ; $829319
    jsl.l decompress_to_vram_long; $82931C
    ldx.w #$8C10                ; $829320
    ldy.w #$57A0                ; $829323
    lda.w #$0087                ; $829326
    jsl.l decompress_to_vram_long; $829329
vs_race_sound:
    lda.w $047C                 ; $82932D
    bmi vs_race_track           ; $829330
    jsl.l wait_03e0             ; $829332
    jsl.l sound_load_vs         ; $829336
    sep.b #$20                  ; $82933A
    lda.b #$7F                  ; $82933C
    sta.w $045C                 ; $82933E
    lda.b #$00                  ; $829341
    sta.w $045F                 ; $829343
    rep.b #$20                  ; $829346
vs_race_track:
    lda.w #$0010                ; $829348
    sta.w $002E                 ; $82934B
    lda.w #$0000                ; $82934E
    sta.w $0030                 ; $829351
    stz.w $003E                 ; $829354
    stz.w $0040                 ; $829357
    stz.b $3E                   ; $82935A
    stz.b $40                   ; $82935C
    stz.w $005A                 ; $82935E
    stz.w $005C                 ; $829361
    stz.w $005D                 ; $829364
    stz.w $005F                 ; $829367
    stz.b $35                   ; $82936A
    stz.b $E8                   ; $82936C
    lda.w #$0003                ; $82936E
    sta.w $0060                 ; $829371
    sta.w $0062                 ; $829374
    stz.b $56                   ; $829377
    stz.b $58                   ; $829379
    stz.b $09                   ; $82937B
    stz.b $0B                   ; $82937D
    stz.b $5A                   ; $82937F
    stz.b $0D                   ; $829381
    stz.b $4C                   ; $829383
    stz.b $4E                   ; $829385
    stz.b $50                   ; $829387
    stz.b $52                   ; $829389
    stz.b $56                   ; $82938B
    stz.b $58                   ; $82938D
    stz.b $54                   ; $82938F
    jsr.w track_setup_load      ; $829391
vs_race_track_tail:
    jsl.l track_rows_build      ; $829394
    jsr.w vs_track_objects_view0; $829398
    jsr.w vs_track_objects_view1; $82939B
    jsr.w vs_track_objects_view2; $82939E
    jsr.w vs_track_objects_view3; $8293A1
    lda.w #$0010                ; $8293A4
    ldx.w #$4000                ; $8293A7
    jsr.w func_82AD12           ; $8293AA
    lda.w #$0001                ; $8293AD
    jsr.w wait_frames_82        ; $8293B0
    lda.w #$0011                ; $8293B3
    ldx.w #$1000                ; $8293B6
    jsr.w func_82AD12           ; $8293B9
    lda.w #$0001                ; $8293BC
    jsr.w wait_frames_82        ; $8293BF
    lda.w #$0012                ; $8293C2
    ldx.w #$0400                ; $8293C5
    jsr.w func_82AD12           ; $8293C8
    lda.w #$0001                ; $8293CB
    jsr.w wait_frames_82        ; $8293CE
    lda.w #$0013                ; $8293D1
    ldx.w #$0100                ; $8293D4
    jsr.w func_82AD12           ; $8293D7
    lda.w #$0001                ; $8293DA
    jsr.w wait_frames_82        ; $8293DD
    lda.w #$2202                ; $8293E0
    sta.w $4370                 ; $8293E3
    lda.w #$6A35                ; $8293E6
    sta.w $4372                 ; $8293E9
    lda.w #$009E                ; $8293EC
    sta.w $4375                 ; $8293EF
    sep.b #$20                  ; $8293F2
    lda.b #$71                  ; $8293F4
    sta.w $2121                 ; $8293F6
    lda.b #$7E                  ; $8293F9
    sta.w $4374                 ; $8293FB
    lda.b #$80                  ; $8293FE
    sta.w $420B                 ; $829400
    stz.w $2121                 ; $829403
    lda.l $7E6A33               ; $829406
    sta.w $2122                 ; $82940A
    lda.l $7E6A34               ; $82940D
    sta.w $2122                 ; $829411
    rep.b #$20                  ; $829414
    stz.w $4315                 ; $829416
    stz.w $4325                 ; $829419
    stz.w $4335                 ; $82941C
    jsr.w vs_road_rows          ; $82941F
vs_race_sums_clear:
    stz.w $01CE                 ; $829422
    stz.w $01CC                 ; $829425
    stz.w $01D2                 ; $829428
    stz.w $01D0                 ; $82942B
    stz.w $01D6                 ; $82942E
    stz.w $01D4                 ; $829431
    stz.w $01DA                 ; $829434
    stz.w $01D8                 ; $829437
    stz.w $00D4                 ; $82943A
    stz.w $00D6                 ; $82943D
    stz.w $00D7                 ; $829440
    stz.w $00D9                 ; $829443
    stz.w $00DA                 ; $829446
    stz.w $00DC                 ; $829449
    stz.w $00DD                 ; $82944C
    stz.w $00DF                 ; $82944F
    jsl.l vs_road_project       ; $829452
vs_race_start:
    jsr.w vs_car_bands          ; $829456
    lda.w #$9777                ; $829459
    sta.w $0008                 ; $82945C
    lda.w #$9721                ; $82945F
    sta.w $0001                 ; $829462
    lda.w #$8B8E                ; $829465
    sta.w $0005                 ; $829468
    sep.b #$20                  ; $82946B
    lda.b #$83                  ; $82946D
    sta.w $0003                 ; $82946F
    lda.b #$83                  ; $829472
    sta.w $0007                 ; $829474
    rep.b #$20                  ; $829477
    cli                         ; $829479
    stz.w $00BA                 ; $82947A
    lda.w #$0001                ; $82947D
    sta.w $1E9E                 ; $829480
    stz.w $014A                 ; $829483
    lda.w #$0F00                ; $829486
    sta.w $0148                 ; $829489
    lda.w #$0080                ; $82948C
    sta.w $0146                 ; $82948F
    lda.w #$0001                ; $829492
    jsr.w wait_frames_82        ; $829495
    lda.w #$8004                ; $829498
    sta.w $1ECE                 ; $82949B
    bit.w $047C                 ; $82949E
    bpl vs_race_frame_body      ; $8294A1
    lda.w #$0043                ; $8294A3
    sta.w $0233                 ; $8294A6
    sta.w $02E6                 ; $8294A9
    sta.w $0295                 ; $8294AC
    sta.w $0348                 ; $8294AF
vs_race_frame_body:
    jsr.w vs_hud_digits         ; $8294B2
    lda.w $1ECE                 ; $8294B5
    bmi CODE_8294BE             ; $8294B8
    beq CODE_82952E             ; $8294BA
    bra CODE_8294D7             ; $8294BC
CODE_8294BE:
    bit.w $047C                 ; $8294BE
    bmi CODE_8294D5             ; $8294C1
    lda.w #$003F                ; $8294C3
    sta.w $0233                 ; $8294C6
    inc a                       ; $8294C9
    sta.w $02E6                 ; $8294CA
    inc a                       ; $8294CD
    sta.w $0295                 ; $8294CE
    inc a                       ; $8294D1
    sta.w $0348                 ; $8294D2
CODE_8294D5:
    bra CODE_82952E             ; $8294D5
CODE_8294D7:
    tax                         ; $8294D7
    sec                         ; $8294D8
    sbc.w $1E9E                 ; $8294D9
    bpl CODE_8294E1             ; $8294DC
    lda.w #$0000                ; $8294DE
CODE_8294E1:
    sta.w $1ECE                 ; $8294E1
    tay                         ; $8294E4
    lda.w #$0000                ; $8294E5
    cpx.w #$0168                ; $8294E8
    bmi CODE_8294F5             ; $8294EB
    incbin !baserom:$114ED..$114F5
CODE_8294F5:
    cpx.w #$00F0                ; $8294F5
    bmi CODE_829502             ; $8294F8
    cpy.w #$00F0                ; $8294FA
    bpl CODE_829502             ; $8294FD
    lda.w #$0003                ; $8294FF
CODE_829502:
    cpx.w #$0078                ; $829502
    bmi CODE_82950F             ; $829505
    cpy.w #$0078                ; $829507
    bpl CODE_82950F             ; $82950A
    lda.w #$0004                ; $82950C
CODE_82950F:
    cpx.w #$0002                ; $82950F
    bmi CODE_829522             ; $829512
    cpy.w #$0002                ; $829514
    bpl CODE_829522             ; $829517
    lda.w #$0001                ; $829519
    sta.w $1ECE                 ; $82951C
    lda.w #$0005                ; $82951F
CODE_829522:
    sta.w $0233                 ; $829522
    sta.w $02E6                 ; $829525
    sta.w $0295                 ; $829528
    sta.w $0348                 ; $82952B
CODE_82952E:
    jsr.w vs_cars_link          ; $82952E
    jsl.l vs_cars_frame         ; $829531
    jsl.l vs_cars_order         ; $829535
    jsr.w vs_car_bands          ; $829539
    lda.w $0204                 ; $82953C
    sta.w $0200                 ; $82953F
    lda.w $0206                 ; $829542
    sta.w $0202                 ; $829545
    lda.w $02B7                 ; $829548
    sta.w $02B3                 ; $82954B
    lda.w $02B9                 ; $82954E
    sta.w $02B5                 ; $829551
    lda.w $0266                 ; $829554
    sta.w $0262                 ; $829557
    lda.w $0268                 ; $82955A
    sta.w $0264                 ; $82955D
    lda.w $0319                 ; $829560
    sta.w $0315                 ; $829563
    lda.w $031B                 ; $829566
    sta.w $0317                 ; $829569
    jsr.w vs_road_rows          ; $82956C
    jsr.w vs_steps_share        ; $82956F
    jsr.w vs_places_update      ; $829572
    stz.b $86                   ; $829575
    stz.b $88                   ; $829577
    stz.b $8E                   ; $829579
    lda.w $00B4                 ; $82957B
    beq CODE_829589             ; $82957E
    lda.w #$0220                ; $829580
    sta.b $86                   ; $829583
    sta.b $88                   ; $829585
    sta.b $8E                   ; $829587
CODE_829589:
    jsl.l vs_road_project       ; $829589
    jsr.w vs_sprites_view0      ; $82958D
    jsr.w vs_sprites_view1      ; $829590
    jsr.w oam_high_load         ; $829593
    jsr.w vs_sprites_view2      ; $829596
    jsr.w vs_sprites_view3      ; $829599
    jsr.w oam_high_load2        ; $82959C
    lda.w #$0001                ; $82959F
    tsb.w $00B8                 ; $8295A2
    lda.w $1F08                 ; $8295A5
    cmp.w #$0004                ; $8295A8
    bne CODE_8295DB             ; $8295AB
    lda.w $1ED0                 ; $8295AD
    bne CODE_8295B8             ; $8295B0
    lda.w #$7F10                ; $8295B2
    sta.w $1ED0                 ; $8295B5
CODE_8295B8:
    bmi CODE_8295DB             ; $8295B8
    lda.w $1ED0                 ; $8295BA
    clc                         ; $8295BD
    adc.w $1E9E                 ; $8295BE
    sta.w $1ED0                 ; $8295C1
    bpl CODE_8295DB             ; $8295C4
    stz.w $0148                 ; $8295C6
    lda.w #$FF80                ; $8295C9
    sta.w $0146                 ; $8295CC
    bit.w $047C                 ; $8295CF
    bmi CODE_8295DB             ; $8295D2
    incbin !baserom:$115D4..$115DB
CODE_8295DB:
    lda.w $0146                 ; $8295DB
    bne vs_race_frame_tail      ; $8295DE
    lda.w $014A                 ; $8295E0
    and.w #$0F00                ; $8295E3
    beq vs_race_end             ; $8295E6
vs_race_frame_tail:
    lda.w $00B8                 ; $8295E8
    bne vs_race_frame_tail      ; $8295EB
    ldx.w $00BA                 ; $8295ED
    bit.w $047C                 ; $8295F0
    bpl CODE_829617             ; $8295F3
    lda.w $000C                 ; $8295F5
    ora.w $0010                 ; $8295F8
    ora.w $0014                 ; $8295FB
    ora.w $0018                 ; $8295FE
    bit.w #$FFF0                ; $829601
    beq CODE_829639             ; $829604
    lda.w #$007F                ; $829606
    trb.w $047D                 ; $829609
    stz.w $0148                 ; $82960C
    lda.w #$FF80                ; $82960F
    sta.w $0146                 ; $829612
    bra CODE_829639             ; $829615
CODE_829617:
    lda.w #$1000                ; $829617
    trb.w $000E                 ; $82961A
    db $D0,$0F ; bne            ; $82961D
    trb.w $0012                 ; $82961F
    db $D0,$0A ; bne            ; $829622
    trb.w $0016                 ; $829624
    db $D0,$05 ; bne            ; $829627
    trb.w $001A                 ; $829629
    beq CODE_829639             ; $82962C
    incbin !baserom:$1162E..$11639
CODE_829639:
    stz.w $00BA                 ; $829639
    stx.w $1E9E                 ; $82963C
    jmp.w vs_race_frame_body    ; $82963F
vs_race_end:
    sei                         ; $829642
    lda.w #$FFFF                ; $829643
    sta.w $4209                 ; $829646
    stz.w $420B                 ; $829649
    lda.w $047B                 ; $82964C
    sec                         ; $82964F
    sbc.w #$002F                ; $829650
    sta.w $047B                 ; $829653
    rts                         ; $829656
    incbin !baserom:$11657..$117BE
vs_cars_link:
    ldy.w #$0000                ; $8297BE
    ldx.w $1E6A                 ; $8297C1
    lda.w $1E70                 ; $8297C4
    sta.l $7EFACF,x             ; $8297C7
    lda.w $1E6C                 ; $8297CB
    sta.l $7EFACD,x             ; $8297CE
    ldx.w $1E6C                 ; $8297D2
    lda.w $1E6A                 ; $8297D5
    sta.l $7EFACF,x             ; $8297D8
    lda.w $1E6E                 ; $8297DC
    sta.l $7EFACD,x             ; $8297DF
    ldx.w $1E6E                 ; $8297E3
    lda.w $1E6C                 ; $8297E6
    sta.l $7EFACF,x             ; $8297E9
    lda.w $1E70                 ; $8297ED
    sta.l $7EFACD,x             ; $8297F0
    ldx.w $1E70                 ; $8297F4
    lda.w $1E6E                 ; $8297F7
    sta.l $7EFACF,x             ; $8297FA
    lda.w $1E6A                 ; $8297FE
    sta.l $7EFACD,x             ; $829801
    rts                         ; $829805
vs_car_bands:
    lda.w #$0000                ; $829806
    sta.l $308000               ; $829809
    lda.w $0213                 ; $82980D
    sta.l $308000               ; $829810
    lda.w #$0300                ; $829814
    sta.l $308000               ; $829817
    lda.w #$FF00                ; $82981B
    and.l $308000               ; $82981E
    xba                         ; $829822
    sta.b $14                   ; $829823
    lda.l $308000               ; $829825
    xba                         ; $829829
    and.w #$FF00                ; $82982A
    ora.b $14                   ; $82982D
    sec                         ; $82982F
    sbc.w #$0B00                ; $829830
    cmp.w #$3100                ; $829833
    bmi CODE_829840             ; $829836
    incbin !baserom:$11838..$11840
CODE_829840:
    sta.l $7E4E2D               ; $829840
    lda.w $0244                 ; $829844
    lsr a                       ; $829847
    bcc CODE_8298C5             ; $829848
    lda.l $7EFAB4               ; $82984A
    cmp.w $0224                 ; $82984E
    lda.l $7EFAB6               ; $829851
    sbc.w $0226                 ; $829855
    bpl CODE_82985D             ; $829858
    lda.w #$0000                ; $82985A
CODE_82985D:
    sta.b $14                   ; $82985D
    lda.w #$0058                ; $82985F
    sec                         ; $829862
    sbc.b $14                   ; $829863
    bpl CODE_82986A             ; $829865
    lda.w #$0000                ; $829867
CODE_82986A:
    and.w #$00F8                ; $82986A
    sta.l $7E4E35               ; $82986D
    lda.w $0244                 ; $829871
    bit.w #$0004                ; $829874
    beq CODE_8298C5             ; $829877
    lda.l $7E4E2D               ; $829879
    clc                         ; $82987D
    adc.w #$0B00                ; $82987E
    bpl CODE_829886             ; $829881
    incbin !baserom:$11883..$11886
CODE_829886:
    sta.w $4204                 ; $829886
    lda.l $7EFAB6               ; $829889
    sec                         ; $82988D
    sbc.w $0226                 ; $82988E
    bpl CODE_829896             ; $829891
    lda.w #$0000                ; $829893
CODE_829896:
    clc                         ; $829896
    adc.w #$0010                ; $829897
    cmp.w #$00D0                ; $82989A
    bcc CODE_8298A2             ; $82989D
    lda.w #$00D0                ; $82989F
CODE_8298A2:
    sep.b #$20                  ; $8298A2
    sta.w $4206                 ; $8298A4
    rep.b #$20                  ; $8298A7
    xba                         ; $8298A9
    xba                         ; $8298AA
    xba                         ; $8298AB
    nop                         ; $8298AC
    nop                         ; $8298AD
    lda.w $4214                 ; $8298AE
    asl a                       ; $8298B1
    asl a                       ; $8298B2
    asl a                       ; $8298B3
    asl a                       ; $8298B4
    sec                         ; $8298B5
    sbc.w #$0B00                ; $8298B6
    cmp.w #$3100                ; $8298B9
    bmi CODE_8298C1             ; $8298BC
    incbin !baserom:$118BE..$118C1
CODE_8298C1:
    sta.l $7E4E2D               ; $8298C1
CODE_8298C5:
    lda.w #$0000                ; $8298C5
    sta.l $308000               ; $8298C8
    lda.w $02C6                 ; $8298CC
    sta.l $308000               ; $8298CF
    lda.w #$0300                ; $8298D3
    sta.l $308000               ; $8298D6
    lda.w #$FF00                ; $8298DA
    and.l $308000               ; $8298DD
    xba                         ; $8298E1
    sta.b $14                   ; $8298E2
    lda.l $308000               ; $8298E4
    xba                         ; $8298E8
    and.w #$FF00                ; $8298E9
    ora.b $14                   ; $8298EC
    sec                         ; $8298EE
    sbc.w #$0B00                ; $8298EF
    cmp.w #$2E00                ; $8298F2
    bmi CODE_8298FA             ; $8298F5
    incbin !baserom:$118F7..$118FA
CODE_8298FA:
    sta.l $7E4E2F               ; $8298FA
    lda.w $02F7                 ; $8298FE
    lsr a                       ; $829901
    bcc CODE_82997F             ; $829902
    lda.l $7EFAE9               ; $829904
    cmp.w $02D7                 ; $829908
    lda.l $7EFAEB               ; $82990B
    sbc.w $02D9                 ; $82990F
    bpl CODE_829917             ; $829912
    lda.w #$0000                ; $829914
CODE_829917:
    sta.b $14                   ; $829917
    lda.w #$0058                ; $829919
    sec                         ; $82991C
    sbc.b $14                   ; $82991D
    bpl CODE_829924             ; $82991F
    lda.w #$0000                ; $829921
CODE_829924:
    and.w #$00F8                ; $829924
    sta.l $7E4E37               ; $829927
    lda.w $02F7                 ; $82992B
    bit.w #$0004                ; $82992E
    beq CODE_82997F             ; $829931
    lda.l $7E4E2F               ; $829933
    clc                         ; $829937
    adc.w #$0B00                ; $829938
    bpl CODE_829940             ; $82993B
    incbin !baserom:$1193D..$11940
CODE_829940:
    sta.w $4204                 ; $829940
    lda.l $7EFAEB               ; $829943
    sec                         ; $829947
    sbc.w $02D9                 ; $829948
    bpl CODE_829950             ; $82994B
    lda.w #$0000                ; $82994D
CODE_829950:
    clc                         ; $829950
    adc.w #$0010                ; $829951
    cmp.w #$00D0                ; $829954
    bcc CODE_82995C             ; $829957
    lda.w #$00D0                ; $829959
CODE_82995C:
    sep.b #$20                  ; $82995C
    sta.w $4206                 ; $82995E
    rep.b #$20                  ; $829961
    xba                         ; $829963
    xba                         ; $829964
    xba                         ; $829965
    nop                         ; $829966
    nop                         ; $829967
    lda.w $4214                 ; $829968
    asl a                       ; $82996B
    asl a                       ; $82996C
    asl a                       ; $82996D
    asl a                       ; $82996E
    sec                         ; $82996F
    sbc.w #$0B00                ; $829970
    cmp.w #$2E00                ; $829973
    bmi CODE_82997B             ; $829976
    incbin !baserom:$11978..$1197B
CODE_82997B:
    sta.l $7E4E2F               ; $82997B
CODE_82997F:
    lda.w #$0000                ; $82997F
    sta.l $308000               ; $829982
    lda.w $0275                 ; $829986
    sta.l $308000               ; $829989
    lda.w #$0300                ; $82998D
    sta.l $308000               ; $829990
    lda.w #$FF00                ; $829994
    and.l $308000               ; $829997
    xba                         ; $82999B
    sta.b $14                   ; $82999C
    lda.l $308000               ; $82999E
    xba                         ; $8299A2
    and.w #$FF00                ; $8299A3
    ora.b $14                   ; $8299A6
    sec                         ; $8299A8
    sbc.w #$0B00                ; $8299A9
    cmp.w #$3100                ; $8299AC
    bmi CODE_8299B4             ; $8299AF
    incbin !baserom:$119B1..$119B4
CODE_8299B4:
    sta.l $7E4E31               ; $8299B4
    lda.w $02A6                 ; $8299B8
    lsr a                       ; $8299BB
    bcc CODE_829A39             ; $8299BC
    lda.l $7EFB1E               ; $8299BE
    cmp.w $0286                 ; $8299C2
    lda.l $7EFB20               ; $8299C5
    sbc.w $0288                 ; $8299C9
    bpl CODE_8299D1             ; $8299CC
    lda.w #$0000                ; $8299CE
CODE_8299D1:
    sta.b $14                   ; $8299D1
    lda.w #$0058                ; $8299D3
    sec                         ; $8299D6
    sbc.b $14                   ; $8299D7
    bpl CODE_8299DE             ; $8299D9
    lda.w #$0000                ; $8299DB
CODE_8299DE:
    and.w #$00F8                ; $8299DE
    sta.l $7E4E39               ; $8299E1
    lda.w $02A6                 ; $8299E5
    bit.w #$0004                ; $8299E8
    beq CODE_829A39             ; $8299EB
    lda.l $7E4E31               ; $8299ED
    clc                         ; $8299F1
    adc.w #$0B00                ; $8299F2
    bpl CODE_8299FA             ; $8299F5
    incbin !baserom:$119F7..$119FA
CODE_8299FA:
    sta.w $4204                 ; $8299FA
    lda.l $7EFB20               ; $8299FD
    sec                         ; $829A01
    sbc.w $0288                 ; $829A02
    bpl CODE_829A0A             ; $829A05
    lda.w #$0000                ; $829A07
CODE_829A0A:
    clc                         ; $829A0A
    adc.w #$0010                ; $829A0B
    cmp.w #$00D0                ; $829A0E
    bcc CODE_829A16             ; $829A11
    incbin !baserom:$11A13..$11A16
CODE_829A16:
    sep.b #$20                  ; $829A16
    sta.w $4206                 ; $829A18
    rep.b #$20                  ; $829A1B
    xba                         ; $829A1D
    xba                         ; $829A1E
    xba                         ; $829A1F
    nop                         ; $829A20
    nop                         ; $829A21
    lda.w $4214                 ; $829A22
    asl a                       ; $829A25
    asl a                       ; $829A26
    asl a                       ; $829A27
    asl a                       ; $829A28
    sec                         ; $829A29
    sbc.w #$0B00                ; $829A2A
    cmp.w #$3100                ; $829A2D
    bmi CODE_829A35             ; $829A30
    incbin !baserom:$11A32..$11A35
CODE_829A35:
    sta.l $7E4E31               ; $829A35
CODE_829A39:
    lda.w #$0000                ; $829A39
    sta.l $308000               ; $829A3C
    lda.w $0328                 ; $829A40
    sta.l $308000               ; $829A43
    lda.w #$0300                ; $829A47
    sta.l $308000               ; $829A4A
    lda.w #$FF00                ; $829A4E
    and.l $308000               ; $829A51
    xba                         ; $829A55
    sta.b $14                   ; $829A56
    lda.l $308000               ; $829A58
    xba                         ; $829A5C
    and.w #$FF00                ; $829A5D
    ora.b $14                   ; $829A60
    sec                         ; $829A62
    sbc.w #$0B00                ; $829A63
    cmp.w #$2D80                ; $829A66
    bmi CODE_829A6E             ; $829A69
    incbin !baserom:$11A6B..$11A6E
CODE_829A6E:
    sta.l $7E4E33               ; $829A6E
    lda.w $0359                 ; $829A72
    lsr a                       ; $829A75
    bcc CODE_829AF3             ; $829A76
    lda.l $7EFB53               ; $829A78
    cmp.w $0339                 ; $829A7C
    lda.l $7EFB55               ; $829A7F
    sbc.w $033B                 ; $829A83
    bpl CODE_829A8B             ; $829A86
    lda.w #$0000                ; $829A88
CODE_829A8B:
    sta.b $14                   ; $829A8B
    lda.w #$0058                ; $829A8D
    sec                         ; $829A90
    sbc.b $14                   ; $829A91
    bpl CODE_829A98             ; $829A93
    lda.w #$0000                ; $829A95
CODE_829A98:
    and.w #$00F8                ; $829A98
    sta.l $7E4E3B               ; $829A9B
    lda.w $0359                 ; $829A9F
    bit.w #$0004                ; $829AA2
    beq CODE_829AF3             ; $829AA5
    lda.l $7E4E33               ; $829AA7
    clc                         ; $829AAB
    adc.w #$0B00                ; $829AAC
    bpl CODE_829AB4             ; $829AAF
    incbin !baserom:$11AB1..$11AB4
CODE_829AB4:
    sta.w $4204                 ; $829AB4
    lda.l $7EFB55               ; $829AB7
    sec                         ; $829ABB
    sbc.w $033B                 ; $829ABC
    bpl CODE_829AC4             ; $829ABF
    lda.w #$0000                ; $829AC1
CODE_829AC4:
    clc                         ; $829AC4
    adc.w #$0010                ; $829AC5
    cmp.w #$00D0                ; $829AC8
    bcc CODE_829AD0             ; $829ACB
    lda.w #$00D0                ; $829ACD
CODE_829AD0:
    sep.b #$20                  ; $829AD0
    sta.w $4206                 ; $829AD2
    rep.b #$20                  ; $829AD5
    xba                         ; $829AD7
    xba                         ; $829AD8
    xba                         ; $829AD9
    nop                         ; $829ADA
    nop                         ; $829ADB
    lda.w $4214                 ; $829ADC
    asl a                       ; $829ADF
    asl a                       ; $829AE0
    asl a                       ; $829AE1
    asl a                       ; $829AE2
    sec                         ; $829AE3
    sbc.w #$0B00                ; $829AE4
    cmp.w #$2D80                ; $829AE7
    bmi CODE_829AEF             ; $829AEA
    incbin !baserom:$11AEC..$11AEF
CODE_829AEF:
    sta.l $7E4E33               ; $829AEF
CODE_829AF3:
    rts                         ; $829AF3
vs_places_update:
    phd                         ; $829AF4
    lda.w #$0200                ; $829AF5
    tcd                         ; $829AF8
    ldx.w #$0004                ; $829AF9
    jsr.w vs_cars_ahead         ; $829AFC
    cpy.w $1ED2                 ; $829AFF
    beq CODE_829B12             ; $829B02
    sty.w $1ED2                 ; $829B04
    lda.w $0233                 ; $829B07
    tya                         ; $829B0A
    clc                         ; $829B0B
    adc.w #$0006                ; $829B0C
    jsr.w race_message_request_0233; $829B0F
CODE_829B12:
    lda.w #$02B3                ; $829B12
    tcd                         ; $829B15
    ldx.w #$0003                ; $829B16
    jsr.w vs_cars_ahead         ; $829B19
    cpy.w $1ED4                 ; $829B1C
    beq CODE_829B2C             ; $829B1F
    sty.w $1ED4                 ; $829B21
    tya                         ; $829B24
    clc                         ; $829B25
    adc.w #$0006                ; $829B26
    jsr.w race_message_request_02e6; $829B29
CODE_829B2C:
    lda.w #$0262                ; $829B2C
    tcd                         ; $829B2F
    ldx.w #$0002                ; $829B30
    jsr.w vs_cars_ahead         ; $829B33
    cpy.w $1ED6                 ; $829B36
    beq CODE_829B46             ; $829B39
    sty.w $1ED6                 ; $829B3B
    tya                         ; $829B3E
    clc                         ; $829B3F
    adc.w #$0006                ; $829B40
    jsr.w race_message_request_0295; $829B43
CODE_829B46:
    lda.w #$0315                ; $829B46
    tcd                         ; $829B49
    ldx.w #$0001                ; $829B4A
    jsr.w vs_cars_ahead         ; $829B4D
    cpy.w $1ED8                 ; $829B50
    beq CODE_829B60             ; $829B53
    sty.w $1ED8                 ; $829B55
    tya                         ; $829B58
    clc                         ; $829B59
    adc.w #$0006                ; $829B5A
    jsr.w race_message_request_0348; $829B5D
CODE_829B60:
    pld                         ; $829B60
    rts                         ; $829B61
vs_cars_ahead:
    ldy.w #$0000                ; $829B62
    cpx.w #$0004                ; $829B65
    lda.b $25                   ; $829B68
    sbc.w $0225                 ; $829B6A
    lda.b $27                   ; $829B6D
    sbc.w $0227                 ; $829B6F
    lda.b $35                   ; $829B72
    sbc.w $0235                 ; $829B74
    bpl CODE_829B7A             ; $829B77
    iny                         ; $829B79
CODE_829B7A:
    cpx.w #$0003                ; $829B7A
    lda.b $25                   ; $829B7D
    sbc.w $02D8                 ; $829B7F
    lda.b $27                   ; $829B82
    sbc.w $02DA                 ; $829B84
    lda.b $35                   ; $829B87
    sbc.w $02E8                 ; $829B89
    bpl CODE_829B8F             ; $829B8C
    iny                         ; $829B8E
CODE_829B8F:
    cpx.w #$0002                ; $829B8F
    lda.b $25                   ; $829B92
    sbc.w $0287                 ; $829B94
    lda.b $27                   ; $829B97
    sbc.w $0289                 ; $829B99
    lda.b $35                   ; $829B9C
    sbc.w $0297                 ; $829B9E
    bpl CODE_829BA4             ; $829BA1
    iny                         ; $829BA3
CODE_829BA4:
    cpx.w #$0001                ; $829BA4
    lda.b $25                   ; $829BA7
    sbc.w $033A                 ; $829BA9
    lda.b $27                   ; $829BAC
    sbc.w $033C                 ; $829BAE
    lda.b $35                   ; $829BB1
    sbc.w $034A                 ; $829BB3
    bpl CODE_829BB9             ; $829BB6
    iny                         ; $829BB8
CODE_829BB9:
    rts                         ; $829BB9
    incbin !baserom:$11BBA..$11BC9
race_message_request_0233:
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
    bcs CODE_829BE3             ; $829BDC
CODE_829BDE:
    pla                         ; $829BDE
    sta.w $0233                 ; $829BDF
    rts                         ; $829BE2
CODE_829BE3:
    pla                         ; $829BE3
    rts                         ; $829BE4
race_message_request_02e6:
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
    bcs CODE_829BFF             ; $829BF8
CODE_829BFA:
    pla                         ; $829BFA
    sta.w $02E6                 ; $829BFB
    rts                         ; $829BFE
CODE_829BFF:
    pla                         ; $829BFF
    rts                         ; $829C00
race_message_request_0295:
    pha                         ; $829C01
    asl a                       ; $829C02
    pha                         ; $829C03
    lda.w $0295                 ; $829C04
    asl a                       ; $829C07
    tax                         ; $829C08
    lda.l $83A0A4,x             ; $829C09
    plx                         ; $829C0D
    cmp.l $83A0A4,x             ; $829C0E
    beq CODE_829C16             ; $829C12
    bcs CODE_829C1B             ; $829C14
CODE_829C16:
    pla                         ; $829C16
    sta.w $0295                 ; $829C17
    rts                         ; $829C1A
CODE_829C1B:
    pla                         ; $829C1B
    rts                         ; $829C1C
race_message_request_0348:
    pha                         ; $829C1D
    asl a                       ; $829C1E
    pha                         ; $829C1F
    lda.w $0348                 ; $829C20
    asl a                       ; $829C23
    tax                         ; $829C24
    lda.l $83A0A4,x             ; $829C25
    plx                         ; $829C29
    cmp.l $83A0A4,x             ; $829C2A
    beq CODE_829C32             ; $829C2E
    bcs CODE_829C37             ; $829C30
CODE_829C32:
    pla                         ; $829C32
    sta.w $0348                 ; $829C33
    rts                         ; $829C36
CODE_829C37:
    pla                         ; $829C37
    rts                         ; $829C38
vs_hud_digits:
    lda.w $0244                 ; $829C39
    bit.w #$0004                ; $829C3C
    beq CODE_829C44             ; $829C3F
    jmp.w $9D21                 ; $829C41
CODE_829C44:
    lda.w #$0000                ; $829C44
    sta.l $308000               ; $829C47
    lda.w $0213                 ; $829C4B
    sta.l $308000               ; $829C4E
    lda.w #$0160                ; $829C52
    sta.l $308000               ; $829C55
    nop                         ; $829C59
    lda.l $308000               ; $829C5A
    lda.l $308000               ; $829C5E
    bpl CODE_829C68             ; $829C62
    incbin !baserom:$11C64..$11C68
CODE_829C68:
    cmp.w #$000C                ; $829C68
    bcc CODE_829C70             ; $829C6B
    lda.w #$000C                ; $829C6D
CODE_829C70:
    sta.w $003E                 ; $829C70
    lda.w $9F52                 ; $829C73
    sta.w $0042                 ; $829C76
    sta.w $0044                 ; $829C79
    lda.w #$0000                ; $829C7C
    sta.l $308000               ; $829C7F
    lda.w $0210                 ; $829C83
    bpl CODE_829C8C             ; $829C86
    incbin !baserom:$11C88..$11C8C
CODE_829C8C:
    asl a                       ; $829C8C
    asl a                       ; $829C8D
    asl a                       ; $829C8E
    sta.l $308000               ; $829C8F
    lda.w #$4000                ; $829C93
    sta.l $308000               ; $829C96
    nop                         ; $829C9A
    lda.l $308000               ; $829C9B
    lda.l $308000               ; $829C9F
    cmp.w #$000A                ; $829CA3
    bcc CODE_829CEB             ; $829CA6
    cmp.w #$0064                ; $829CA8
    bcc CODE_829CCC             ; $829CAB
    sta.w $4204                 ; $829CAD
    sep.b #$20                  ; $829CB0
    lda.b #$64                  ; $829CB2
    sta.w $4206                 ; $829CB4
    rep.b #$20                  ; $829CB7
    xba                         ; $829CB9
    xba                         ; $829CBA
    xba                         ; $829CBB
    nop                         ; $829CBC
    nop                         ; $829CBD
    lda.w $4214                 ; $829CBE
    asl a                       ; $829CC1
    tax                         ; $829CC2
    lda.w $9F54,x               ; $829CC3
    sta.w $0042                 ; $829CC6
    lda.w $4216                 ; $829CC9
CODE_829CCC:
    sta.w $4204                 ; $829CCC
    sep.b #$20                  ; $829CCF
    lda.b #$0A                  ; $829CD1
    sta.w $4206                 ; $829CD3
    rep.b #$20                  ; $829CD6
    xba                         ; $829CD8
    xba                         ; $829CD9
    xba                         ; $829CDA
    nop                         ; $829CDB
    nop                         ; $829CDC
    lda.w $4214                 ; $829CDD
    asl a                       ; $829CE0
    tax                         ; $829CE1
    lda.w $9F54,x               ; $829CE2
    sta.w $0044                 ; $829CE5
    lda.w $4216                 ; $829CE8
CODE_829CEB:
    asl a                       ; $829CEB
    tax                         ; $829CEC
    lda.w $9F54,x               ; $829CED
    sta.w $0046                 ; $829CF0
    lda.w #$0000                ; $829CF3
    sta.l $308000               ; $829CF6
    lda.w $0240                 ; $829CFA
    sta.l $308000               ; $829CFD
    lda.w $019E                 ; $829D01
    sta.l $308000               ; $829D04
    nop                         ; $829D08
    lda.l $308000               ; $829D09
    lda.w #$0055                ; $829D0D
    sec                         ; $829D10
    sbc.l $308000               ; $829D11
    bpl CODE_829D1A             ; $829D15
    lda.w #$0000                ; $829D17
CODE_829D1A:
    clc                         ; $829D1A
    adc.w #$57A1                ; $829D1B
    sta.w $00A4                 ; $829D1E
    lda.l $7E4E3D               ; $829D21
    sta.b $14                   ; $829D25
    lda.w $0233                 ; $829D27
    sta.b $16                   ; $829D2A
    lda.w #$4E61                ; $829D2C
    sta.b $18                   ; $829D2F
    lda.l $7E4E45               ; $829D31
    sta.b $1A                   ; $829D35
    lda.l $7E4E4D               ; $829D37
    sta.b $1C                   ; $829D3B
    lda.w #$0C00                ; $829D3D
    sta.b $3A                   ; $829D40
    jsr.w vs_message_update     ; $829D42
    lda.b $14                   ; $829D45
    sta.l $7E4E3D               ; $829D47
    lda.b $16                   ; $829D4B
    sta.w $0233                 ; $829D4D
    lda.b $1A                   ; $829D50
    sta.l $7E4E45               ; $829D52
    lda.b $1C                   ; $829D56
    sta.l $7E4E4D               ; $829D58
    lda.w $02F7                 ; $829D5C
    bit.w #$0004                ; $829D5F
    beq CODE_829D67             ; $829D62
    jmp.w $9E44                 ; $829D64
CODE_829D67:
    lda.w #$0000                ; $829D67
    sta.l $308000               ; $829D6A
    lda.w $02C6                 ; $829D6E
    sta.l $308000               ; $829D71
    lda.w #$0160                ; $829D75
    sta.l $308000               ; $829D78
    nop                         ; $829D7C
    lda.l $308000               ; $829D7D
    lda.l $308000               ; $829D81
    bpl CODE_829D8B             ; $829D85
    incbin !baserom:$11D87..$11D8B
CODE_829D8B:
    cmp.w #$000C                ; $829D8B
    bcc CODE_829D93             ; $829D8E
    lda.w #$000C                ; $829D90
CODE_829D93:
    sta.w $0040                 ; $829D93
    lda.w $9F52                 ; $829D96
    sta.w $004E                 ; $829D99
    sta.w $0050                 ; $829D9C
    lda.w #$0000                ; $829D9F
    sta.l $308000               ; $829DA2
    lda.w $02C3                 ; $829DA6
    bpl CODE_829DAF             ; $829DA9
    incbin !baserom:$11DAB..$11DAF
CODE_829DAF:
    asl a                       ; $829DAF
    asl a                       ; $829DB0
    asl a                       ; $829DB1
    sta.l $308000               ; $829DB2
    lda.w #$4000                ; $829DB6
    sta.l $308000               ; $829DB9
    nop                         ; $829DBD
    lda.l $308000               ; $829DBE
    lda.l $308000               ; $829DC2
    cmp.w #$000A                ; $829DC6
    bcc CODE_829E0E             ; $829DC9
    cmp.w #$0064                ; $829DCB
    bcc CODE_829DEF             ; $829DCE
    sta.w $4204                 ; $829DD0
    sep.b #$20                  ; $829DD3
    lda.b #$64                  ; $829DD5
    sta.w $4206                 ; $829DD7
    rep.b #$20                  ; $829DDA
    xba                         ; $829DDC
    xba                         ; $829DDD
    xba                         ; $829DDE
    nop                         ; $829DDF
    nop                         ; $829DE0
    lda.w $4214                 ; $829DE1
    asl a                       ; $829DE4
    tax                         ; $829DE5
    lda.w $9F54,x               ; $829DE6
    sta.w $004E                 ; $829DE9
    lda.w $4216                 ; $829DEC
CODE_829DEF:
    sta.w $4204                 ; $829DEF
    sep.b #$20                  ; $829DF2
    lda.b #$0A                  ; $829DF4
    sta.w $4206                 ; $829DF6
    rep.b #$20                  ; $829DF9
    xba                         ; $829DFB
    xba                         ; $829DFC
    xba                         ; $829DFD
    nop                         ; $829DFE
    nop                         ; $829DFF
    lda.w $4214                 ; $829E00
    asl a                       ; $829E03
    tax                         ; $829E04
    lda.w $9F54,x               ; $829E05
    sta.w $0050                 ; $829E08
    lda.w $4216                 ; $829E0B
CODE_829E0E:
    asl a                       ; $829E0E
    tax                         ; $829E0F
    lda.w $9F54,x               ; $829E10
    sta.w $0052                 ; $829E13
    lda.w #$0000                ; $829E16
    sta.l $308000               ; $829E19
    lda.w $02F3                 ; $829E1D
    sta.l $308000               ; $829E20
    lda.w $019E                 ; $829E24
    sta.l $308000               ; $829E27
    nop                         ; $829E2B
    lda.l $308000               ; $829E2C
    lda.w #$0055                ; $829E30
    sec                         ; $829E33
    sbc.l $308000               ; $829E34
    bpl CODE_829E3D             ; $829E38
    lda.w #$0000                ; $829E3A
CODE_829E3D:
    clc                         ; $829E3D
    adc.w #$57A1                ; $829E3E
    sta.w $00AC                 ; $829E41
    lda.l $7E4E3F               ; $829E44
    sta.b $14                   ; $829E48
    lda.w $02E6                 ; $829E4A
    sta.b $16                   ; $829E4D
    lda.w #$4E95                ; $829E4F
    sta.b $18                   ; $829E52
    lda.l $7E4E47               ; $829E54
    sta.b $1A                   ; $829E58
    lda.l $7E4E4F               ; $829E5A
    sta.b $1C                   ; $829E5E
    lda.w #$0C00                ; $829E60
    sta.b $3A                   ; $829E63
    jsr.w vs_message_update     ; $829E65
    lda.b $14                   ; $829E68
    sta.l $7E4E3F               ; $829E6A
    lda.b $16                   ; $829E6E
    sta.w $02E6                 ; $829E70
    lda.b $1A                   ; $829E73
    sta.l $7E4E47               ; $829E75
    lda.b $1C                   ; $829E79
    sta.l $7E4E4F               ; $829E7B
    lda.w $02A6                 ; $829E7F
    bit.w #$0004                ; $829E82
    beq CODE_829E8A             ; $829E85
    jmp.w $9F67                 ; $829E87
CODE_829E8A:
    lda.w #$0000                ; $829E8A
    sta.l $308000               ; $829E8D
    lda.w $0275                 ; $829E91
    sta.l $308000               ; $829E94
    lda.w #$0160                ; $829E98
    sta.l $308000               ; $829E9B
    nop                         ; $829E9F
    lda.l $308000               ; $829EA0
    lda.l $308000               ; $829EA4
    bpl CODE_829EAE             ; $829EA8
    incbin !baserom:$11EAA..$11EAE
CODE_829EAE:
    cmp.w #$000C                ; $829EAE
    bcc CODE_829EB6             ; $829EB1
    lda.w #$000C                ; $829EB3
CODE_829EB6:
    sta.w $0064                 ; $829EB6
    lda.w $9F52                 ; $829EB9
    sta.w $0048                 ; $829EBC
    sta.w $004A                 ; $829EBF
    lda.w #$0000                ; $829EC2
    sta.l $308000               ; $829EC5
    lda.w $0272                 ; $829EC9
    bpl CODE_829ED2             ; $829ECC
    incbin !baserom:$11ECE..$11ED2
CODE_829ED2:
    asl a                       ; $829ED2
    asl a                       ; $829ED3
    asl a                       ; $829ED4
    sta.l $308000               ; $829ED5
    lda.w #$4000                ; $829ED9
    sta.l $308000               ; $829EDC
    nop                         ; $829EE0
    lda.l $308000               ; $829EE1
    lda.l $308000               ; $829EE5
    cmp.w #$000A                ; $829EE9
    bcc CODE_829F31             ; $829EEC
    cmp.w #$0064                ; $829EEE
    bcc CODE_829F12             ; $829EF1
    sta.w $4204                 ; $829EF3
    sep.b #$20                  ; $829EF6
    lda.b #$64                  ; $829EF8
    sta.w $4206                 ; $829EFA
    rep.b #$20                  ; $829EFD
    xba                         ; $829EFF
    xba                         ; $829F00
    xba                         ; $829F01
    nop                         ; $829F02
    nop                         ; $829F03
    lda.w $4214                 ; $829F04
    asl a                       ; $829F07
    tax                         ; $829F08
    lda.w $9F54,x               ; $829F09
    sta.w $0048                 ; $829F0C
    lda.w $4216                 ; $829F0F
CODE_829F12:
    sta.w $4204                 ; $829F12
    sep.b #$20                  ; $829F15
    lda.b #$0A                  ; $829F17
    sta.w $4206                 ; $829F19
    rep.b #$20                  ; $829F1C
    xba                         ; $829F1E
    xba                         ; $829F1F
    xba                         ; $829F20
    nop                         ; $829F21
    nop                         ; $829F22
    lda.w $4214                 ; $829F23
    asl a                       ; $829F26
    tax                         ; $829F27
    lda.w $9F54,x               ; $829F28
    sta.w $004A                 ; $829F2B
    lda.w $4216                 ; $829F2E
CODE_829F31:
    asl a                       ; $829F31
    tax                         ; $829F32
    lda.w $9F54,x               ; $829F33
    sta.w $004C                 ; $829F36
    lda.w #$0000                ; $829F39
    sta.l $308000               ; $829F3C
    lda.w $02A2                 ; $829F40
    sta.l $308000               ; $829F43
    lda.w $019E                 ; $829F47
    sta.l $308000               ; $829F4A
    nop                         ; $829F4E
    lda.l $308000               ; $829F4F
    lda.w #$0055                ; $829F53
    sec                         ; $829F56
    sbc.l $308000               ; $829F57
    bpl CODE_829F60             ; $829F5B
    lda.w #$0000                ; $829F5D
CODE_829F60:
    clc                         ; $829F60
    adc.w #$57F9                ; $829F61
    sta.w $00A6                 ; $829F64
    lda.l $7E4E41               ; $829F67
    sta.b $14                   ; $829F6B
    lda.w $0295                 ; $829F6D
    sta.b $16                   ; $829F70
    lda.w #$4EC9                ; $829F72
    sta.b $18                   ; $829F75
    lda.l $7E4E49               ; $829F77
    sta.b $1A                   ; $829F7B
    lda.l $7E4E51               ; $829F7D
    sta.b $1C                   ; $829F81
    lda.w #$0C00                ; $829F83
    sta.b $3A                   ; $829F86
    jsr.w vs_message_update     ; $829F88
    lda.b $14                   ; $829F8B
    sta.l $7E4E41               ; $829F8D
    lda.b $16                   ; $829F91
    sta.w $0295                 ; $829F93
    lda.b $1A                   ; $829F96
    sta.l $7E4E49               ; $829F98
    lda.b $1C                   ; $829F9C
    sta.l $7E4E51               ; $829F9E
    lda.w $0359                 ; $829FA2
    bit.w #$0004                ; $829FA5
    beq CODE_829FAD             ; $829FA8
    jmp.w $A08A                 ; $829FAA
CODE_829FAD:
    lda.w #$0000                ; $829FAD
    sta.l $308000               ; $829FB0
    lda.w $0328                 ; $829FB4
    sta.l $308000               ; $829FB7
    lda.w #$0160                ; $829FBB
    sta.l $308000               ; $829FBE
    nop                         ; $829FC2
    lda.l $308000               ; $829FC3
    lda.l $308000               ; $829FC7
    bpl CODE_829FD1             ; $829FCB
    incbin !baserom:$11FCD..$11FD1
CODE_829FD1:
    cmp.w #$000C                ; $829FD1
    bcc CODE_829FD9             ; $829FD4
    lda.w #$000C                ; $829FD6
CODE_829FD9:
    sta.w $0068                 ; $829FD9
    lda.w $9F52                 ; $829FDC
    sta.w $0054                 ; $829FDF
    sta.w $0056                 ; $829FE2
    lda.w #$0000                ; $829FE5
    sta.l $308000               ; $829FE8
    lda.w $0325                 ; $829FEC
    bpl CODE_829FF5             ; $829FEF
    incbin !baserom:$11FF1..$11FF5
CODE_829FF5:
    asl a                       ; $829FF5
    asl a                       ; $829FF6
    asl a                       ; $829FF7
    sta.l $308000               ; $829FF8
    lda.w #$4000                ; $829FFC
    sta.l $308000               ; $829FFF
    nop                         ; $82A003
    lda.l $308000               ; $82A004
    lda.l $308000               ; $82A008
    cmp.w #$000A                ; $82A00C
    bcc CODE_82A054             ; $82A00F
    cmp.w #$0064                ; $82A011
    bcc CODE_82A035             ; $82A014
    sta.w $4204                 ; $82A016
    sep.b #$20                  ; $82A019
    lda.b #$64                  ; $82A01B
    sta.w $4206                 ; $82A01D
    rep.b #$20                  ; $82A020
    xba                         ; $82A022
    xba                         ; $82A023
    xba                         ; $82A024
    nop                         ; $82A025
    nop                         ; $82A026
    lda.w $4214                 ; $82A027
    asl a                       ; $82A02A
    tax                         ; $82A02B
    lda.w $9F54,x               ; $82A02C
    sta.w $0054                 ; $82A02F
    lda.w $4216                 ; $82A032
CODE_82A035:
    sta.w $4204                 ; $82A035
    sep.b #$20                  ; $82A038
    lda.b #$0A                  ; $82A03A
    sta.w $4206                 ; $82A03C
    rep.b #$20                  ; $82A03F
    xba                         ; $82A041
    xba                         ; $82A042
    xba                         ; $82A043
    nop                         ; $82A044
    nop                         ; $82A045
    lda.w $4214                 ; $82A046
    asl a                       ; $82A049
    tax                         ; $82A04A
    lda.w $9F54,x               ; $82A04B
    sta.w $0056                 ; $82A04E
    lda.w $4216                 ; $82A051
CODE_82A054:
    asl a                       ; $82A054
    tax                         ; $82A055
    lda.w $9F54,x               ; $82A056
    sta.w $0058                 ; $82A059
    lda.w #$0000                ; $82A05C
    sta.l $308000               ; $82A05F
    lda.w $0355                 ; $82A063
    sta.l $308000               ; $82A066
    lda.w $019E                 ; $82A06A
    sta.l $308000               ; $82A06D
    nop                         ; $82A071
    lda.l $308000               ; $82A072
    lda.w #$0055                ; $82A076
    sec                         ; $82A079
    sbc.l $308000               ; $82A07A
    bpl CODE_82A083             ; $82A07E
    lda.w #$0000                ; $82A080
CODE_82A083:
    clc                         ; $82A083
    adc.w #$57F9                ; $82A084
    sta.w $00AE                 ; $82A087
    lda.l $7E4E43               ; $82A08A
    sta.b $14                   ; $82A08E
    lda.w $0348                 ; $82A090
    sta.b $16                   ; $82A093
    lda.w #$4EFD                ; $82A095
    sta.b $18                   ; $82A098
    lda.l $7E4E4B               ; $82A09A
    sta.b $1A                   ; $82A09E
    lda.l $7E4E53               ; $82A0A0
    sta.b $1C                   ; $82A0A4
    lda.w #$0C00                ; $82A0A6
    sta.b $3A                   ; $82A0A9
    jsr.w vs_message_update     ; $82A0AB
    lda.b $14                   ; $82A0AE
    sta.l $7E4E43               ; $82A0B0
    lda.b $16                   ; $82A0B4
    sta.w $0348                 ; $82A0B6
    lda.b $1A                   ; $82A0B9
    sta.l $7E4E4B               ; $82A0BB
    lda.b $1C                   ; $82A0BF
    sta.l $7E4E53               ; $82A0C1
    rts                         ; $82A0C5
vs_message_update:
    pea.w $837E                 ; $82A0C6
    plb                         ; $82A0C9
    lda.b $14                   ; $82A0CA
    cmp.b $16                   ; $82A0CC
    bne CODE_82A0D3             ; $82A0CE
    jmp.w $A106                 ; $82A0D0
CODE_82A0D3:
    asl a                       ; $82A0D3
    pha                         ; $82A0D4
    lda.b $16                   ; $82A0D5
    asl a                       ; $82A0D7
    sta.b $3C                   ; $82A0D8
    tax                         ; $82A0DA
    lda.l $83A0A4,x             ; $82A0DB
    plx                         ; $82A0DF
    cmp.l $83A0A4,x             ; $82A0E0
    bcs CODE_82A0EB             ; $82A0E4
    lda.b $14                   ; $82A0E6
    jmp.w $A102                 ; $82A0E8
CODE_82A0EB:
    jsr.w vs_centred_line       ; $82A0EB
    ldx.b $3C                   ; $82A0EE
    lda.l $83A142,x             ; $82A0F0
    sta.b $1A                   ; $82A0F4
    lda.l $83A1E0,x             ; $82A0F6
    sta.b $1C                   ; $82A0FA
    lda.b $16                   ; $82A0FC
    sta.b $14                   ; $82A0FE
    bra CODE_82A132             ; $82A100
    lda.b $14                   ; $82A102
    sta.b $16                   ; $82A104
    lda.b $1A                   ; $82A106
    beq CODE_82A130             ; $82A108
    sec                         ; $82A10A
    sbc.w $1E9E                 ; $82A10B
    beq CODE_82A112             ; $82A10E
    bcs CODE_82A130             ; $82A110
CODE_82A112:
    ldx.b $18                   ; $82A112
    ldy.w #$000D                ; $82A114
    lda.l $839F52               ; $82A117
CODE_82A11B:
    sta.w $0000,x               ; $82A11B
    sta.w $001A,x               ; $82A11E
    inx                         ; $82A121
    inx                         ; $82A122
    dey                         ; $82A123
    bne CODE_82A11B             ; $82A124
    lda.w #$0000                ; $82A126
    sta.b $16                   ; $82A129
    sta.b $14                   ; $82A12B
    lda.w #$0000                ; $82A12D
CODE_82A130:
    sta.b $1A                   ; $82A130
CODE_82A132:
    plb                         ; $82A132
    rts                         ; $82A133
vs_centred_line:
    ldx.b $18                   ; $82A134
    ldy.w #$000D                ; $82A136
    lda.l $839F52               ; $82A139
CODE_82A13D:
    sta.w $0000,x               ; $82A13D
    sta.w $001A,x               ; $82A140
    inx                         ; $82A143
    inx                         ; $82A144
    dey                         ; $82A145
    bne CODE_82A13D             ; $82A146
    ldx.b $3C                   ; $82A148
    lda.l $83A006,x             ; $82A14A
    tax                         ; $82A14E
    ldy.w #$000D                ; $82A14F
CODE_82A152:
    lda.l $830000,x             ; $82A152
    and.w #$00FF                ; $82A156
    beq CODE_82A164             ; $82A159
    bit.w #$0080                ; $82A15B
    bne CODE_82A161             ; $82A15E
    dey                         ; $82A160
CODE_82A161:
    inx                         ; $82A161
    bra CODE_82A152             ; $82A162
CODE_82A164:
    tya                         ; $82A164
    and.w #$FFFE                ; $82A165
    clc                         ; $82A168
    adc.b $18                   ; $82A169
    tay                         ; $82A16B
    ldx.b $3C                   ; $82A16C
    lda.l $83A006,x             ; $82A16E
    tax                         ; $82A172
CODE_82A173:
    lda.l $830000,x             ; $82A173
    and.w #$00FF                ; $82A177
    beq CODE_82A1D2             ; $82A17A
    phx                         ; $82A17C
    bit.w #$0080                ; $82A17D
    beq CODE_82A18C             ; $82A180
    incbin !baserom:$12182..$1218C
CODE_82A18C:
    cmp.w #$0020                ; $82A18C
    bne CODE_82A196             ; $82A18F
    lda.w #$0026                ; $82A191
    bra CODE_82A1B8             ; $82A194
CODE_82A196:
    cmp.w #$0021                ; $82A196
    bne CODE_82A1A0             ; $82A199
    lda.w #$0024                ; $82A19B
    bra CODE_82A1B8             ; $82A19E
CODE_82A1A0:
    cmp.w #$002B                ; $82A1A0
    bne CODE_82A1AA             ; $82A1A3
    incbin !baserom:$121A5..$121AA
CODE_82A1AA:
    cmp.w #$003A                ; $82A1AA
    bcs CODE_82A1B5             ; $82A1AD
    sec                         ; $82A1AF
    sbc.w #$0016                ; $82A1B0
    bra CODE_82A1B8             ; $82A1B3
CODE_82A1B5:
    sbc.w #$0041                ; $82A1B5
CODE_82A1B8:
    asl a                       ; $82A1B8
    tax                         ; $82A1B9
    lda.l $839F6A,x             ; $82A1BA
    ora.b $3A                   ; $82A1BE
    sta.w $0000,y               ; $82A1C0
    lda.l $839FB8,x             ; $82A1C3
    ora.b $3A                   ; $82A1C7
    sta.w $001A,y               ; $82A1C9
    iny                         ; $82A1CC
    iny                         ; $82A1CD
    plx                         ; $82A1CE
    inx                         ; $82A1CF
    bra CODE_82A173             ; $82A1D0
CODE_82A1D2:
    rts                         ; $82A1D2
track_backdrop_load:
    ldx.w #$9FEB                ; $82A1D3
    ldy.w #$0000                ; $82A1D6
    lda.w #$7F98                ; $82A1D9
    jsl.l decompress            ; $82A1DC
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
    jsl.l upload_vram_now       ; $82A211
    plx                         ; $82A215
    phx                         ; $82A216
    lda.w #$007F                ; $82A217
    sta.b $14                   ; $82A21A
    lda.l $85A34A,x             ; $82A21C
    tax                         ; $82A220
    ldy.w #$3000                ; $82A221
    lda.w #$0600                ; $82A224
    jsl.l upload_vram_now       ; $82A227
    plx                         ; $82A22B
    phx                         ; $82A22C
    lda.w #$007F                ; $82A22D
    sta.b $14                   ; $82A230
    lda.l $85A34C,x             ; $82A232
    tax                         ; $82A236
    ldy.w #$3400                ; $82A237
    lda.w #$0600                ; $82A23A
    jsl.l upload_vram_now       ; $82A23D
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
    jsl.l upload_vram_now       ; $82A271
    plx                         ; $82A275
    lda.w #$007F                ; $82A276
    sta.b $14                   ; $82A279
    lda.l $85A34C,x             ; $82A27B
    tax                         ; $82A27F
    ldy.w #$7800                ; $82A280
    lda.w #$0600                ; $82A283
    jsl.l upload_vram_now       ; $82A286
    rts                         ; $82A28A
race_state_reset:
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
    jsl.l race_grid_setup       ; $82A5B9
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
    jsr.w track_palettes_load   ; $82A62F
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
track_palettes_load:
    ldx.w #$929A                ; $82A672
    ldy.w #$0000                ; $82A675
    lda.w #$7F88                ; $82A678
    jsl.l decompress            ; $82A67B
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
    beq CODE_82A6B7             ; $82A6AE
    lda.w #$0070                ; $82A6B0
    sta.b $18                   ; $82A6B3
    sta.b $1C                   ; $82A6B5
CODE_82A6B7:
    lda.w $047B                 ; $82A6B7
    asl a                       ; $82A6BA
    tax                         ; $82A6BB
    lda.l $839583,x             ; $82A6BC
    bne CODE_82A6EC             ; $82A6C0
    lda.w $039C                 ; $82A6C2
    and.w #$00FF                ; $82A6C5
    beq CODE_82A6D7             ; $82A6C8
    lda.b $16                   ; $82A6CA
    clc                         ; $82A6CC
    adc.w #$0070                ; $82A6CD
    sta.b $16                   ; $82A6D0
    lda.w #$00E0                ; $82A6D2
    sta.b $18                   ; $82A6D5
CODE_82A6D7:
    lda.w $03CB                 ; $82A6D7
    and.w #$00FF                ; $82A6DA
    beq CODE_82A6EC             ; $82A6DD
    lda.b $1A                   ; $82A6DF static
    clc                         ; $82A6E1 static
    adc.w #$0070                ; $82A6E2 static
    sta.b $1A                   ; $82A6E5 static
    lda.w #$00E0                ; $82A6E7 static
    sta.b $1C                   ; $82A6EA static
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
track_setup_load:
    lda.w $047B                 ; $82A7F9
    asl a                       ; $82A7FC
    tax                         ; $82A7FD
    lda.l $84FD33,x             ; $82A7FE
    tax                         ; $82A802
    ldy.w #$5391                ; $82A803
    lda.w #$7E9B                ; $82A806
    jsl.l decompress            ; $82A809
    lda.w $047B                 ; $82A80D
    asl a                       ; $82A810
    tax                         ; $82A811
    lda.l $84FDA1,x             ; $82A812
    pha                         ; $82A816
    ldy.w #$8CB3                ; $82A817
    lda.l $84FE0F,x             ; $82A81A
    ora.w #$7E00                ; $82A81E
    plx                         ; $82A821
    jsl.l decompress            ; $82A822
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
    ldx.w #$8512                ; $82A84B
    ldy.w #$0000                ; $82A84E
    lda.w #$7F89                ; $82A851
    jsl.l decompress            ; $82A854
    bra CODE_82A88C             ; $82A858
CODE_82A85A:
    lda.w $01E4                 ; $82A85A
    bmi CODE_82A87F             ; $82A85D
    bne CODE_82A870             ; $82A85F
    ldx.w #$8097                ; $82A861
    ldy.w #$0000                ; $82A864
    lda.w #$7F96                ; $82A867
    jsl.l decompress            ; $82A86A
    bra CODE_82A88C             ; $82A86E
CODE_82A870:
    ldx.w #$A24D                ; $82A870
    ldy.w #$0000                ; $82A873
    lda.w #$7F97                ; $82A876
    jsl.l decompress            ; $82A879
    bra CODE_82A88C             ; $82A87D
CODE_82A87F:
    ldx.w #$8020                ; $82A87F
    ldy.w #$0000                ; $82A882
    lda.w #$7F96                ; $82A885
    jsl.l decompress            ; $82A888
CODE_82A88C:
    lda.w $047B                 ; $82A88C
    asl a                       ; $82A88F
    tax                         ; $82A890
    lda.w $039C                 ; $82A891
    and.w #$00FF                ; $82A894
    beq CODE_82A8A1             ; $82A897
    lda.l $85A3CC,x             ; $82A899
    bpl CODE_82A8B4             ; $82A89D
    bra CODE_82A8A7             ; $82A89F
CODE_82A8A1:
    lda.l $85A35E,x             ; $82A8A1
    bpl CODE_82A8B4             ; $82A8A5
CODE_82A8A7:
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
    lda.l $85A3CC,x             ; $82A8C4 static
    bpl CODE_82A8DF             ; $82A8C8 static
    bra CODE_82A8D2             ; $82A8CA static
CODE_82A8CC:
    lda.l $85A35E,x             ; $82A8CC
    bpl CODE_82A8DF             ; $82A8D0
CODE_82A8D2:
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
    lda.w #$0612                ; $82A8F7
    sta.b $14                   ; $82A8FA
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
    jsl.l decompress            ; $82A942
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
track_segments_load:
    ldx.w #$8000                ; $82A954
    ldy.w #$0000                ; $82A957
    lda.w #$7F9A                ; $82A95A
    jsl.l decompress            ; $82A95D
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
player_segment_step_long:
    jsr.w player_segment_step   ; $82A9AF
    rtl                         ; $82A9B2
player2_segment_step_long:
    jsr.w player2_segment_step  ; $82A9B3
    rtl                         ; $82A9B6
func_82A9B7:
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
    jsr.w func_82AB3F           ; $82A9D0
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
func_82AA7B:
    pea.w $837E                 ; $82AA7B
    plb                         ; $82AA7E
    lda.w $0334                 ; $82AA7F
    bpl CODE_82AA8E             ; $82AA82
    lda.w #$FFD0                ; $82AA84
    sta.b $18                   ; $82AA87
    stz.b $16                   ; $82AA89
    jmp.w $AAE3                 ; $82AA8B
CODE_82AA8E:
    ldy.w $0336                 ; $82AA8E
    ldx.w $0334                 ; $82AA91
    jsr.w func_82AB3F           ; $82AA94
    lda.w #$0000                ; $82AA97
    sta.l $308000               ; $82AA9A
    lda.b $16                   ; $82AA9E
    asl a                       ; $82AAA0
    sta.l $308000               ; $82AAA1
    lda.w $0346                 ; $82AAA5
    sta.l $308000               ; $82AAA8
    nop                         ; $82AAAC
    nop                         ; $82AAAD
    lda.l $308000               ; $82AAAE
    lda.l $308000               ; $82AAB2
    eor.w #$FFFF                ; $82AAB6
    inc a                       ; $82AAB9
    sta.b $16                   ; $82AABA
    lda.w #$0000                ; $82AABC
    sta.l $308000               ; $82AABF
    lda.b $18                   ; $82AAC3
    asl a                       ; $82AAC5
    sta.l $308000               ; $82AAC6
    lda.w $0346                 ; $82AACA
    sta.l $308000               ; $82AACD
    lda.w #$0033                ; $82AAD1
    sec                         ; $82AAD4
    sta.l $308000               ; $82AAD5
    sbc.l $308000               ; $82AAD9
    eor.w #$FFFF                ; $82AADD
    inc a                       ; $82AAE0
    sta.b $18                   ; $82AAE1
    lda.b $16                   ; $82AAE3
    cmp.w $033E                 ; $82AAE5
    bpl CODE_82AAFE             ; $82AAE8
    lda.w $1E9E                 ; $82AAEA
    asl a                       ; $82AAED
    asl a                       ; $82AAEE
    eor.w #$FFFF                ; $82AAEF
    sec                         ; $82AAF2
    adc.w $033E                 ; $82AAF3
    cmp.b $16                   ; $82AAF6
    bpl CODE_82AAFC             ; $82AAF8
    lda.b $16                   ; $82AAFA
CODE_82AAFC:
    bra CODE_82AB0D             ; $82AAFC
CODE_82AAFE:
    lda.w $1E9E                 ; $82AAFE
    asl a                       ; $82AB01
    asl a                       ; $82AB02
    clc                         ; $82AB03
    adc.w $033E                 ; $82AB04
    cmp.b $16                   ; $82AB07
    bmi CODE_82AB0D             ; $82AB09
    lda.b $16                   ; $82AB0B
CODE_82AB0D:
    sta.w $033E                 ; $82AB0D
    lda.b $18                   ; $82AB10
    cmp.w $0340                 ; $82AB12
    bpl CODE_82AB2B             ; $82AB15
    lda.w $1E9E                 ; $82AB17
    asl a                       ; $82AB1A
    asl a                       ; $82AB1B
    eor.w #$FFFF                ; $82AB1C
    sec                         ; $82AB1F
    adc.w $0340                 ; $82AB20
    cmp.b $18                   ; $82AB23
    bpl CODE_82AB29             ; $82AB25
    lda.b $18                   ; $82AB27
CODE_82AB29:
    bra CODE_82AB3A             ; $82AB29
CODE_82AB2B:
    lda.w $1E9E                 ; $82AB2B
    asl a                       ; $82AB2E
    asl a                       ; $82AB2F
    clc                         ; $82AB30
    adc.w $0340                 ; $82AB31
    cmp.b $18                   ; $82AB34
    bmi CODE_82AB3A             ; $82AB36
    lda.b $18                   ; $82AB38
CODE_82AB3A:
    sta.w $0340                 ; $82AB3A
    plb                         ; $82AB3D
    rts                         ; $82AB3E
func_82AB3F:
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
race_countdown:
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
    jsr.w func_82AD12           ; $82AC4E
CODE_82AC51:
    rts                         ; $82AC51
func_82AC52:
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
    lda.w #$3000                ; $82AC84
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
    lda.w #$0000                ; $82ACA1
CODE_82ACA4:
    asl a                       ; $82ACA4
    sta.b $14                   ; $82ACA5
    lda.w #$00B0                ; $82ACA7
    sec                         ; $82ACAA
    sbc.b $14                   ; $82ACAB
    bpl CODE_82ACB2             ; $82ACAD
    lda.w #$0000                ; $82ACAF
CODE_82ACB2:
    and.w #$00F8                ; $82ACB2
    sta.l $7E4E35               ; $82ACB5
    lda.w $0244                 ; $82ACB9
    bit.w #$0004                ; $82ACBC
    beq CODE_82AD0D             ; $82ACBF
    lda.l $7E4E2D               ; $82ACC1
    clc                         ; $82ACC5
    adc.w #$0B00                ; $82ACC6
    bpl CODE_82ACCE             ; $82ACC9
    incbin !baserom:$12CCB..$12CCE
CODE_82ACCE:
    sta.w $4204                 ; $82ACCE
    lda.l $7EFAB6               ; $82ACD1
    sec                         ; $82ACD5
    sbc.w $0226                 ; $82ACD6
    bpl CODE_82ACDE             ; $82ACD9
    lda.w #$0000                ; $82ACDB
CODE_82ACDE:
    clc                         ; $82ACDE
    adc.w #$0010                ; $82ACDF
    cmp.w #$00D0                ; $82ACE2
    bcc CODE_82ACEA             ; $82ACE5
    lda.w #$00D0                ; $82ACE7
CODE_82ACEA:
    sep.b #$20                  ; $82ACEA
    sta.w $4206                 ; $82ACEC
    rep.b #$20                  ; $82ACEF
    xba                         ; $82ACF1
    xba                         ; $82ACF2
    xba                         ; $82ACF3
    nop                         ; $82ACF4
    nop                         ; $82ACF5
    lda.w $4214                 ; $82ACF6
    asl a                       ; $82ACF9
    asl a                       ; $82ACFA
    asl a                       ; $82ACFB
    asl a                       ; $82ACFC
    sec                         ; $82ACFD
    sbc.w #$0B00                ; $82ACFE
    cmp.w #$3000                ; $82AD01
    bmi CODE_82AD09             ; $82AD04
    lda.w #$3000                ; $82AD06
CODE_82AD09:
    sta.l $7E4E2D               ; $82AD09
CODE_82AD0D:
    rts                         ; $82AD0D
    incbin !baserom:$12D0E..$12D12
func_82AD12:
    bit.w $047C                 ; $82AD12
    bmi CODE_82AD20             ; $82AD15
    stz.w $0466                 ; $82AD17
    stx.w $0465                 ; $82AD1A
    sta.w $0463                 ; $82AD1D
CODE_82AD20:
    rts                         ; $82AD20
    incbin !baserom:$12D21..$12D2C
race_positions_update:
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
    jsr.w race_message_request_0233; $82AD82
CODE_82AD85:
    lda.w $0144                 ; $82AD85
    cmp.w $1ED4                 ; $82AD88
    beq CODE_82AD97             ; $82AD8B
    sta.w $1ED4                 ; $82AD8D
    clc                         ; $82AD90
    adc.w #$0025                ; $82AD91
    jsr.w race_message_request_02e6; $82AD94
CODE_82AD97:
    plb                         ; $82AD97
    rts                         ; $82AD98
tile_block_5x5:
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
track_block_triple:
    ldx.w #$97E4                ; $82ADC2
    ldy.w #$0000                ; $82ADC5
    lda.w #$7F99                ; $82ADC8
    jsl.l decompress            ; $82ADCB
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
oam_high_load:
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
oam_high_load2:
    pea.w $8330                 ; $82AEA4
    plb                         ; $82AEA7
    lda.w #$0006                ; $82AEA8
    sta.w $8000                 ; $82AEAB
    ldy.b $8E                   ; $82AEAE
    lda.w $8000                 ; $82AEB0
    sta.w $1BB6,y               ; $82AEB3
    lda.w $8000                 ; $82AEB6
    sta.w $1BB8,y               ; $82AEB9
    lda.w $8000                 ; $82AEBC
    sta.w $1BBA,y               ; $82AEBF
    lda.w $8000                 ; $82AEC2
    sta.w $1BBC,y               ; $82AEC5
    lda.w $8000                 ; $82AEC8
    sta.w $1BBE,y               ; $82AECB
    lda.w $8000                 ; $82AECE
    sta.w $1BC0,y               ; $82AED1
    lda.w $8000                 ; $82AED4
    sta.w $1BC2,y               ; $82AED7
    lda.w $8000                 ; $82AEDA
    sta.w $1BC4,y               ; $82AEDD
    lda.w $8000                 ; $82AEE0
    sta.w $1BC6,y               ; $82AEE3
    lda.w $8000                 ; $82AEE6
    sta.w $1BC8,y               ; $82AEE9
    lda.w $8000                 ; $82AEEC
    sta.w $1BCA,y               ; $82AEEF
    lda.w $8000                 ; $82AEF2
    sta.w $1BCC,y               ; $82AEF5
    lda.w $8000                 ; $82AEF8
    sta.w $1BCE,y               ; $82AEFB
    lda.w $8000                 ; $82AEFE
    sta.w $1BD0,y               ; $82AF01
    lda.w $8000                 ; $82AF04
    sta.w $1BD2,y               ; $82AF07
    lda.w $8000                 ; $82AF0A
    sta.w $1BD4,y               ; $82AF0D
    plb                         ; $82AF10
    lda.b $88                   ; $82AF11
    tay                         ; $82AF13
    sec                         ; $82AF14
    sbc.b $8E                   ; $82AF15
    eor.w #$FFFF                ; $82AF17
    sec                         ; $82AF1A
    adc.w #$0200                ; $82AF1B
    beq CODE_82AF32             ; $82AF1E
    lsr a                       ; $82AF20
    lsr a                       ; $82AF21
    tax                         ; $82AF22
    ldy.b $88                   ; $82AF23
    lda.w #$E0E0                ; $82AF25
CODE_82AF28:
    sta.w $19B6,y               ; $82AF28
    iny                         ; $82AF2B
    iny                         ; $82AF2C
    iny                         ; $82AF2D
    iny                         ; $82AF2E
    dex                         ; $82AF2F
    bne CODE_82AF28             ; $82AF30
CODE_82AF32:
    rts                         ; $82AF32
track_objects_stream:
    lda.w #$0140                ; $82AF33
    sta.l $7E45EC               ; $82AF36
    ldx.b $A0                   ; $82AF3A
    lda.w $0226                 ; $82AF3C
    cmp.l $7E5B91,x             ; $82AF3F
    bcs CODE_82AF48             ; $82AF43
    jmp.w $B025                 ; $82AF45 static
CODE_82AF48:
    lda.l $7E5B94,x             ; $82AF48
    beq CODE_82AFA3             ; $82AF4C
    bit.w #$FF00                ; $82AF4E
    bne CODE_82AFB1             ; $82AF51
    asl a                       ; $82AF53
    tax                         ; $82AF54
    lda.l $86AE36,x             ; $82AF55
    beq CODE_82AFAF             ; $82AF59
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
CODE_82AFA3:
    lda.l $7E5B93,x             ; $82AFA3
    and.w #$00FF                ; $82AFA7
    asl a                       ; $82AFAA
    sta.l $7E45EC               ; $82AFAB
CODE_82AFAF:
    bra CODE_82B012             ; $82AFAF
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
    jsl.l upload_vram_now       ; $82B00D
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
    lda.w #$0140                ; $82B026
    sta.l $7E46C8               ; $82B029
    ldx.b $A4                   ; $82B02D
    lda.w $02D9                 ; $82B02F
    cmp.l $7E5B91,x             ; $82B032
    bcs CODE_82B03B             ; $82B036
    incbin !baserom:$13038..$1303B
CODE_82B03B:
    lda.l $7E5B94,x             ; $82B03B
    db $F0,$58 ; beq            ; $82B03F
    bit.w #$FF00                ; $82B041
    bne CODE_82B0A7             ; $82B044
    asl a                       ; $82B046
    tax                         ; $82B047
    lda.l $86AE36,x             ; $82B048
    db $F0,$57 ; beq            ; $82B04C
    tax                         ; $82B04E
    lda.l $86001E,x             ; $82B04F
    sta.b $14                   ; $82B053
    lda.l $860020,x             ; $82B055
    sta.b $16                   ; $82B059
    lda.l $860000,x             ; $82B05B
    tax                         ; $82B05F
    lda.l $7E6591,x             ; $82B060
    clc                         ; $82B064
    adc.b $14                   ; $82B065
    sta.b $14                   ; $82B067
    ldx.b $A4                   ; $82B069
    lda.l $7E5B93,x             ; $82B06B
    and.w #$00FF                ; $82B06F
    xba                         ; $82B072
    lsr a                       ; $82B073
    lsr a                       ; $82B074
    lsr a                       ; $82B075
    adc.w #$1000                ; $82B076
    sta.b $18                   ; $82B079
    lda.l $7E5B93,x             ; $82B07B
    and.w #$00FF                ; $82B07F
    asl a                       ; $82B082
    sta.b $1A                   ; $82B083
    lda.l $7E5B93,x             ; $82B085
    xba                         ; $82B089
    and.w #$00FF                ; $82B08A
    asl a                       ; $82B08D
    txy                         ; $82B08E
    tax                         ; $82B08F
    lda.b $1A                   ; $82B090
    sta.l $7E46C8,x             ; $82B092
    tyx                         ; $82B096
    bra CODE_82B0FA             ; $82B097
    incbin !baserom:$13099..$130A7
CODE_82B0A7:
    asl a                       ; $82B0A7
    tax                         ; $82B0A8
    lda.l $86AC36,x             ; $82B0A9
    beq CODE_82B10B             ; $82B0AD
    tax                         ; $82B0AF
    lda.l $860022,x             ; $82B0B0
    sta.b $14                   ; $82B0B4
    lda.l $860024,x             ; $82B0B6
    sta.l $810116               ; $82B0BA
    lda.l $860000,x             ; $82B0BE
    tax                         ; $82B0C2
    lda.l $7E6591,x             ; $82B0C3
    clc                         ; $82B0C7
    adc.b $14                   ; $82B0C8
    sta.b $14                   ; $82B0CA
    ldx.b $A4                   ; $82B0CC
    lda.l $7E5B93,x             ; $82B0CE
    and.w #$00FF                ; $82B0D2
    xba                         ; $82B0D5
    lsr a                       ; $82B0D6
    lsr a                       ; $82B0D7
    lsr a                       ; $82B0D8
    adc.w #$1000                ; $82B0D9
    sta.b $18                   ; $82B0DC
    lda.l $7E5B93,x             ; $82B0DE
    and.w #$00FF                ; $82B0E2
    asl a                       ; $82B0E5
    sta.b $1A                   ; $82B0E6
    lda.l $7E5B93,x             ; $82B0E8
    xba                         ; $82B0EC
    and.w #$00FF                ; $82B0ED
    asl a                       ; $82B0F0
    txy                         ; $82B0F1
    tax                         ; $82B0F2
    lda.b $1A                   ; $82B0F3
    sta.l $7E4A38,x             ; $82B0F5
    tyx                         ; $82B0F9
CODE_82B0FA:
    phx                         ; $82B0FA
    ldx.b $14                   ; $82B0FB
    ldy.b $18                   ; $82B0FD
    lda.w #$007F                ; $82B0FF
    sta.b $14                   ; $82B102
    lda.b $16                   ; $82B104
    jsl.l upload_vram_now       ; $82B106
    plx                         ; $82B10A
CODE_82B10B:
    inx                         ; $82B10B
    inx                         ; $82B10C
    inx                         ; $82B10D
    inx                         ; $82B10E
    inx                         ; $82B10F
    stx.b $A4                   ; $82B110
    lda.w $02D9                 ; $82B112
    cmp.l $7E5B91,x             ; $82B115
    bcc CODE_82B11E             ; $82B119
    jmp.w $B03B                 ; $82B11B
CODE_82B11E:
    rts                         ; $82B11E
race_message_update:
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
    jsr.w text_centred_line     ; $82B142
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
text_centred_line:
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
    and.w #$0007                ; $82B1D9 static
    asl a                       ; $82B1DC static
    asl a                       ; $82B1DD static
    xba                         ; $82B1DE static
    sta.b $3A                   ; $82B1DF static
    bra CODE_82B22F             ; $82B1E1 static
CODE_82B1E3:
    cmp.w #$0020                ; $82B1E3
    bne CODE_82B1ED             ; $82B1E6
    lda.w #$0027                ; $82B1E8
    bra CODE_82B219             ; $82B1EB
CODE_82B1ED:
    cmp.w #$0021                ; $82B1ED
    bne CODE_82B1F7             ; $82B1F0
    lda.w #$0024                ; $82B1F2
    bra CODE_82B219             ; $82B1F5
CODE_82B1F7:
    cmp.w #$002E                ; $82B1F7
    bne CODE_82B201             ; $82B1FA
    lda.w #$0025                ; $82B1FC static
    bra CODE_82B219             ; $82B1FF static
CODE_82B201:
    cmp.w #$002B                ; $82B201
    bne CODE_82B20B             ; $82B204
    lda.w #$0026                ; $82B206
    bra CODE_82B219             ; $82B209
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
CODE_82B22F:
    plx                         ; $82B22F
    inx                         ; $82B230
    bra CODE_82B1CA             ; $82B231
CODE_82B233:
    rts                         ; $82B233
    lda.w #$7C57                ; $82B234
    sta.w $2116                 ; $82B237
    ldx.w #$0000                ; $82B23A
    ldy.w #$0157                ; $82B23D
CODE_82B240:
    lda.l $810398,x             ; $82B240
    and.w #$00FF                ; $82B244
    beq CODE_82B24E             ; $82B247
    incbin !baserom:$13249..$1324E
CODE_82B24E:
    sty.w $2118                 ; $82B24E
    inx                         ; $82B251
    cpx.w #$0004                ; $82B252
    bne CODE_82B240             ; $82B255
    lda.w $03A7                 ; $82B257
    and.w #$00FF                ; $82B25A
    beq CODE_82B285             ; $82B25D
    incbin !baserom:$1325F..$13285
CODE_82B285:
    lda.w #$7E17                ; $82B285
    sta.w $2116                 ; $82B288
    ldx.w #$0000                ; $82B28B
    ldy.w #$0157                ; $82B28E
CODE_82B291:
    lda.l $8103C7,x             ; $82B291
    and.w #$00FF                ; $82B295
    beq CODE_82B29F             ; $82B298
    incbin !baserom:$1329A..$1329F
CODE_82B29F:
    sty.w $2118                 ; $82B29F
    inx                         ; $82B2A2
    cpx.w #$0004                ; $82B2A3
    bne CODE_82B291             ; $82B2A6
    lda.w $03D6                 ; $82B2A8
    and.w #$00FF                ; $82B2AB
    beq CODE_82B2D6             ; $82B2AE
    incbin !baserom:$132B0..$132D6
CODE_82B2D6:
    jsr.w race_hud_digits       ; $82B2D6
    lda.w $02F7                 ; $82B2D9
    bit.w #$0004                ; $82B2DC
    beq CODE_82B2E4             ; $82B2DF
    incbin !baserom:$132E1..$132E4
CODE_82B2E4:
    lda.w #$0000                ; $82B2E4
    sta.l $308000               ; $82B2E7
    lda.w $02C6                 ; $82B2EB
    sta.l $308000               ; $82B2EE
    lda.w #$0160                ; $82B2F2
    sta.l $308000               ; $82B2F5
    nop                         ; $82B2F9
    lda.l $308000               ; $82B2FA
    lda.l $308000               ; $82B2FE
    bpl CODE_82B308             ; $82B302
    incbin !baserom:$13304..$13308
CODE_82B308:
    cmp.w #$000C                ; $82B308
    bcc CODE_82B310             ; $82B30B
    lda.w #$000C                ; $82B30D
CODE_82B310:
    sta.w $0040                 ; $82B310
    lda.w #$0157                ; $82B313
    sta.w $004E                 ; $82B316
    sta.w $0050                 ; $82B319
    sta.w $0052                 ; $82B31C
    sta.w $0054                 ; $82B31F
    lda.w #$0000                ; $82B322
    sta.l $308000               ; $82B325
    lda.w $02C3                 ; $82B329
    bpl CODE_82B332             ; $82B32C
    incbin !baserom:$1332E..$13332
CODE_82B332:
    asl a                       ; $82B332
    asl a                       ; $82B333
    asl a                       ; $82B334
    sta.l $308000               ; $82B335
    ldx.w #$3000                ; $82B339
    lda.w $03D6                 ; $82B33C
    and.w #$00FF                ; $82B33F
    beq CODE_82B347             ; $82B342
    incbin !baserom:$13344..$13347
CODE_82B347:
    txa                         ; $82B347
    sta.l $308000               ; $82B348
    nop                         ; $82B34C
    lda.l $308000               ; $82B34D
    lda.l $308000               ; $82B351
    cmp.w #$000A                ; $82B355
    bcc CODE_82B3AF             ; $82B358
    cmp.w #$0064                ; $82B35A
    bcc CODE_82B387             ; $82B35D
    sta.w $4204                 ; $82B35F
    sep.b #$20                  ; $82B362
    lda.b #$64                  ; $82B364
    sta.w $4206                 ; $82B366
    rep.b #$20                  ; $82B369
    xba                         ; $82B36B
    xba                         ; $82B36C
    xba                         ; $82B36D
    nop                         ; $82B36E
    nop                         ; $82B36F
    lda.w $4214                 ; $82B370
    asl a                       ; $82B373
    asl a                       ; $82B374
    tax                         ; $82B375
    lda.l $83A4AC,x             ; $82B376
    sta.w $004E                 ; $82B37A
    lda.l $83A4AE,x             ; $82B37D
    sta.w $0050                 ; $82B381
    lda.w $4216                 ; $82B384
CODE_82B387:
    sta.w $4204                 ; $82B387
    sep.b #$20                  ; $82B38A
    lda.b #$0A                  ; $82B38C
    sta.w $4206                 ; $82B38E
    rep.b #$20                  ; $82B391
    xba                         ; $82B393
    xba                         ; $82B394
    xba                         ; $82B395
    nop                         ; $82B396
    nop                         ; $82B397
    lda.w $4214                 ; $82B398
    asl a                       ; $82B39B
    asl a                       ; $82B39C
    tax                         ; $82B39D
    lda.l $83A4AC,x             ; $82B39E
    sta.w $0052                 ; $82B3A2
    lda.l $83A4AE,x             ; $82B3A5
    sta.w $0054                 ; $82B3A9
    lda.w $4216                 ; $82B3AC
CODE_82B3AF:
    asl a                       ; $82B3AF
    asl a                       ; $82B3B0
    tax                         ; $82B3B1
    lda.l $83A4AC,x             ; $82B3B2
    sta.w $0056                 ; $82B3B6
    lda.l $83A4AE,x             ; $82B3B9
    sta.w $0058                 ; $82B3BD
    lda.w #$0157                ; $82B3C0
    sta.w $0068                 ; $82B3C3
    lda.b $44                   ; $82B3C6
    cmp.w #$000A                ; $82B3C8
    bcc CODE_82B3E7             ; $82B3CB
    cmp.w #$0014                ; $82B3CD
    bcc CODE_82B3DD             ; $82B3D0
    ldx.w $A488                 ; $82B3D2
    stx.w $0068                 ; $82B3D5
    sbc.w #$0014                ; $82B3D8
    bra CODE_82B3E7             ; $82B3DB
CODE_82B3DD:
    ldx.w $A486                 ; $82B3DD
    stx.w $0068                 ; $82B3E0
    sec                         ; $82B3E3
    sbc.w #$000A                ; $82B3E4
CODE_82B3E7:
    asl a                       ; $82B3E7
    tax                         ; $82B3E8
    lda.l $83A484,x             ; $82B3E9
    sta.w $006A                 ; $82B3ED
    lda.w $030B                 ; $82B3F0
    cmp.w #$0009                ; $82B3F3
    bcs CODE_82B400             ; $82B3F6
    ldx.w $034A                 ; $82B3F8
    bpl CODE_82B400             ; $82B3FB
    lda.w #$0000                ; $82B3FD
CODE_82B400:
    ldx.w #$295B                ; $82B400
    stx.w $007E                 ; $82B403
    cmp.w #$0008                ; $82B406
    bcs CODE_82B43D             ; $82B409
    ldy.w #$295D                ; $82B40B
    sty.w $007E                 ; $82B40E
    inc a                       ; $82B411
    sta.b $14                   ; $82B412
    lda.w #$1C3E                ; $82B414
    ldy.w #$000E                ; $82B417
CODE_82B41A:
    dec.b $14                   ; $82B41A
    bne CODE_82B421             ; $82B41C
    lda.w #$0022                ; $82B41E
CODE_82B421:
    sta.w $0090,y               ; $82B421
    dey                         ; $82B424
    dey                         ; $82B425
    bpl CODE_82B41A             ; $82B426
    lda.w #$003E                ; $82B428
    sta.w $009E                 ; $82B42B
    ldx.w #$0006                ; $82B42E
CODE_82B431:
    lda.w #$2959                ; $82B431
    sta.w $0076,x               ; $82B434
    dex                         ; $82B437
    dex                         ; $82B438
    bpl CODE_82B431             ; $82B439
    bra CODE_82B483             ; $82B43B
CODE_82B43D:
    ldx.w #$0006                ; $82B43D
CODE_82B440:
    sec                         ; $82B440
    sbc.w #$0008                ; $82B441
    cmp.w #$0008                ; $82B444
    bcs CODE_82B477             ; $82B447
    tay                         ; $82B449
    lda.w #$295D                ; $82B44A
    sta.w $0076,x               ; $82B44D
    iny                         ; $82B450
    sty.b $14                   ; $82B451
    lda.w #$1C22                ; $82B453
    ldy.w #$000E                ; $82B456
CODE_82B459:
    dec.b $14                   ; $82B459
    bne CODE_82B460             ; $82B45B
    lda.w #$0022                ; $82B45D
CODE_82B460:
    sta.w $0090,y               ; $82B460
    dey                         ; $82B463
    dey                         ; $82B464
    bpl CODE_82B459             ; $82B465
    dex                         ; $82B467
    dex                         ; $82B468
    bmi CODE_82B483             ; $82B469
CODE_82B46B:
    lda.w #$2959                ; $82B46B
    sta.w $0076,x               ; $82B46E
    dex                         ; $82B471
    dex                         ; $82B472
    bpl CODE_82B46B             ; $82B473
    bra CODE_82B483             ; $82B475
CODE_82B477:
    tay                         ; $82B477
    lda.w #$295A                ; $82B478
    sta.w $0076,x               ; $82B47B
    tya                         ; $82B47E
    dex                         ; $82B47F
    dex                         ; $82B480
    bpl CODE_82B440             ; $82B481
CODE_82B483:
    lda.w $030E                 ; $82B483
    lsr a                       ; $82B486
    lsr a                       ; $82B487
    lsr a                       ; $82B488
    and.w #$001E                ; $82B489
    tax                         ; $82B48C
    lda.w $8C35,x               ; $82B48D
    sta.w $00A2                 ; $82B490
    lda.b $4E                   ; $82B493
    asl a                       ; $82B495
    asl a                       ; $82B496
    asl a                       ; $82B497
    tax                         ; $82B498
    lda.w $A4D4,x               ; $82B499
    sta.w $00AC                 ; $82B49C
    lda.w $A4D6,x               ; $82B49F
    sta.w $00AE                 ; $82B4A2
    lda.w $A4D8,x               ; $82B4A5
    sta.w $00B0                 ; $82B4A8
    lda.w $A4DA,x               ; $82B4AB
    sta.w $00B2                 ; $82B4AE
    lda.l $7E4E3F               ; $82B4B1
    sta.b $14                   ; $82B4B5
    lda.w $02E6                 ; $82B4B7
    sta.b $16                   ; $82B4BA
    lda.w #$4E95                ; $82B4BC
    sta.b $18                   ; $82B4BF
    lda.l $7E4E47               ; $82B4C1
    sta.b $1A                   ; $82B4C5
    lda.l $7E4E4F               ; $82B4C7
    sta.b $1C                   ; $82B4CB
    lda.w #$0C00                ; $82B4CD
    sta.b $3A                   ; $82B4D0
    jsr.w race_message_update   ; $82B4D2
    lda.b $14                   ; $82B4D5
    sta.l $7E4E3F               ; $82B4D7
    lda.b $16                   ; $82B4DB
    sta.w $02E6                 ; $82B4DD
    lda.b $1A                   ; $82B4E0
    sta.l $7E4E47               ; $82B4E2
    lda.b $1C                   ; $82B4E6
    sta.l $7E4E4F               ; $82B4E8
    rts                         ; $82B4EC
race_hud_digits:
    lda.w $0244                 ; $82B4ED
    bit.w #$0004                ; $82B4F0
    beq CODE_82B4F8             ; $82B4F3
    jmp.w $B6F3                 ; $82B4F5
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
    eor.w #$FFFF                ; $82B518 static
    inc a                       ; $82B51B static
CODE_82B51C:
    cmp.w #$000C                ; $82B51C
    bcc CODE_82B524             ; $82B51F
    lda.w #$000C                ; $82B521
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
    eor.w #$FFFF                ; $82B548 static
    inc a                       ; $82B54B static
CODE_82B54C:
    asl a                       ; $82B54C
    asl a                       ; $82B54D
    asl a                       ; $82B54E
    sta.l $308000               ; $82B54F
    ldx.w #$3000                ; $82B553
    lda.w $03A7                 ; $82B556
    and.w #$00FF                ; $82B559
    beq CODE_82B561             ; $82B55C
    ldx.w #$4D3B                ; $82B55E
CODE_82B561:
    txa                         ; $82B561
    sta.l $308000               ; $82B562
    nop                         ; $82B566
    lda.l $308000               ; $82B567
    lda.l $308000               ; $82B56B
    bit.w $1E94                 ; $82B56F
    bmi CODE_82B578             ; $82B572
    lda.w $1E94                 ; $82B574
    inc a                       ; $82B577
CODE_82B578:
    ldx.w #$0000                ; $82B578
    cmp.w #$03E8                ; $82B57B
    bcc CODE_82B59E             ; $82B57E
    sbc.w #$03E8                ; $82B580 static
CODE_82B583:
    inx                         ; $82B583 static
    inx                         ; $82B584 static
    inx                         ; $82B585 static
    inx                         ; $82B586 static
    sec                         ; $82B587 static
    sbc.w #$03E8                ; $82B588 static
    bcs CODE_82B583             ; $82B58B static
    adc.w #$03E8                ; $82B58D static
    pha                         ; $82B590 static
    lda.w $A4AC,x               ; $82B591 static
    sta.w $1E96                 ; $82B594 static
    lda.w $A4AE,x               ; $82B597 static
    sta.w $1E98                 ; $82B59A static
    pla                         ; $82B59D static
CODE_82B59E:
    cmp.w #$000A                ; $82B59E
    bcc CODE_82B5F4             ; $82B5A1
    cmp.w #$0064                ; $82B5A3
    bcc CODE_82B5CE             ; $82B5A6
    sta.w $4204                 ; $82B5A8
    sep.b #$20                  ; $82B5AB
    lda.b #$64                  ; $82B5AD
    sta.w $4206                 ; $82B5AF
    rep.b #$20                  ; $82B5B2
    xba                         ; $82B5B4
    xba                         ; $82B5B5
    xba                         ; $82B5B6
    nop                         ; $82B5B7
    nop                         ; $82B5B8
    lda.w $4214                 ; $82B5B9
    asl a                       ; $82B5BC
    asl a                       ; $82B5BD
    tax                         ; $82B5BE
    lda.w $A4AC,x               ; $82B5BF
    sta.w $0042                 ; $82B5C2
    lda.w $A4AE,x               ; $82B5C5
    sta.w $0044                 ; $82B5C8
    lda.w $4216                 ; $82B5CB
CODE_82B5CE:
    sta.w $4204                 ; $82B5CE
    sep.b #$20                  ; $82B5D1
    lda.b #$0A                  ; $82B5D3
    sta.w $4206                 ; $82B5D5
    rep.b #$20                  ; $82B5D8
    xba                         ; $82B5DA
    xba                         ; $82B5DB
    xba                         ; $82B5DC
    nop                         ; $82B5DD
    nop                         ; $82B5DE
    lda.w $4214                 ; $82B5DF
    asl a                       ; $82B5E2
    asl a                       ; $82B5E3
    tax                         ; $82B5E4
    lda.w $A4AC,x               ; $82B5E5
    sta.w $0046                 ; $82B5E8
    lda.w $A4AE,x               ; $82B5EB
    sta.w $0048                 ; $82B5EE
    lda.w $4216                 ; $82B5F1
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
    ldx.w $0297                 ; $82B63A
    bpl CODE_82B642             ; $82B63D
    lda.w #$0000                ; $82B63F
CODE_82B642:
    ldx.w #$295B                ; $82B642
    stx.w $0074                 ; $82B645
    cmp.w #$0008                ; $82B648
    bcs CODE_82B67F             ; $82B64B
    ldy.w #$295C                ; $82B64D
    sty.w $0074                 ; $82B650
    inc a                       ; $82B653
    sta.b $14                   ; $82B654
    lda.w #$1C3E                ; $82B656
    ldy.w #$000E                ; $82B659
CODE_82B65C:
    dec.b $14                   ; $82B65C
    bne CODE_82B663             ; $82B65E
    lda.w #$0022                ; $82B660
CODE_82B663:
    sta.w $0080,y               ; $82B663
    dey                         ; $82B666
    dey                         ; $82B667
    bpl CODE_82B65C             ; $82B668
    lda.w #$003E                ; $82B66A
    sta.w $008E                 ; $82B66D
    ldx.w #$0006                ; $82B670
CODE_82B673:
    lda.w #$2959                ; $82B673
    sta.w $006C,x               ; $82B676
    dex                         ; $82B679
    dex                         ; $82B67A
    bpl CODE_82B673             ; $82B67B
    bra CODE_82B6C5             ; $82B67D
CODE_82B67F:
    ldx.w #$0006                ; $82B67F
CODE_82B682:
    sec                         ; $82B682
    sbc.w #$0008                ; $82B683
    cmp.w #$0008                ; $82B686
    bcs CODE_82B6B9             ; $82B689
    tay                         ; $82B68B
    lda.w #$295C                ; $82B68C
    sta.w $006C,x               ; $82B68F
    iny                         ; $82B692
    sty.b $14                   ; $82B693
    lda.w #$1C22                ; $82B695
    ldy.w #$000E                ; $82B698
CODE_82B69B:
    dec.b $14                   ; $82B69B
    bne CODE_82B6A2             ; $82B69D
    lda.w #$0022                ; $82B69F
CODE_82B6A2:
    sta.w $0080,y               ; $82B6A2
    dey                         ; $82B6A5
    dey                         ; $82B6A6
    bpl CODE_82B69B             ; $82B6A7
    dex                         ; $82B6A9
    dex                         ; $82B6AA
    bmi CODE_82B6C5             ; $82B6AB
CODE_82B6AD:
    lda.w #$2959                ; $82B6AD
    sta.w $006C,x               ; $82B6B0
    dex                         ; $82B6B3
    dex                         ; $82B6B4
    bpl CODE_82B6AD             ; $82B6B5
    bra CODE_82B6C5             ; $82B6B7
CODE_82B6B9:
    tay                         ; $82B6B9
    lda.w #$295A                ; $82B6BA
    sta.w $006C,x               ; $82B6BD
    tya                         ; $82B6C0
    dex                         ; $82B6C1
    dex                         ; $82B6C2
    bpl CODE_82B682             ; $82B6C3
CODE_82B6C5:
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
    ldx.b $50                   ; $82B6FB
    jsr.w func_82B838           ; $82B6FD
    ldx.b $52                   ; $82B700
    jsr.w func_82B838           ; $82B702
    lda.w $0226                 ; $82B705
    ldy.w $0235                 ; $82B708
    jsr.w func_82B769           ; $82B70B
    stx.b $50                   ; $82B70E
    lda.w $02D9                 ; $82B710
    ldy.w $02E8                 ; $82B713
    jsr.w func_82B769           ; $82B716
    stx.b $52                   ; $82B719
    bra CODE_82B72D             ; $82B71B
CODE_82B71D:
    ldx.b $50                   ; $82B71D
    jsr.w func_82B838           ; $82B71F
    lda.w $0226                 ; $82B722
    ldy.w $0235                 ; $82B725
    jsr.w func_82B769           ; $82B728
    stx.b $50                   ; $82B72B
CODE_82B72D:
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
    jsr.w race_message_update   ; $82B74E
    lda.b $14                   ; $82B751
    sta.l $7E4E3D               ; $82B753
    lda.b $16                   ; $82B757
    sta.w $0233                 ; $82B759
    lda.b $1A                   ; $82B75C
    sta.l $7E4E45               ; $82B75E
    lda.b $1C                   ; $82B762
    sta.l $7E4E4D               ; $82B764
    rts                         ; $82B768
func_82B769:
    pha                         ; $82B769
    ldx.w $047B                 ; $82B76A
    lda.l $82DC68,x             ; $82B76D
    bpl CODE_82B785             ; $82B771
    tya                         ; $82B773
    beq CODE_82B785             ; $82B774
    bpl CODE_82B77E             ; $82B776
    pla                         ; $82B778
    lda.w #$0000                ; $82B779
    bra CODE_82B786             ; $82B77C
CODE_82B77E:
    pla                         ; $82B77E
    lda.w $019A                 ; $82B77F
    dec a                       ; $82B782
    bra CODE_82B786             ; $82B783
CODE_82B785:
    pla                         ; $82B785
CODE_82B786:
    jsr.w func_82B78D           ; $82B786
    jsr.w func_82B7B0           ; $82B789
    rts                         ; $82B78C
func_82B78D:
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
func_82B7B0:
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
func_82B838:
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
vs_track_objects_view0:
    lda.w #$0000                ; $82B87A
    sta.b $3C                   ; $82B87D
    lda.w #$0000                ; $82B87F
    sta.b $3A                   ; $82B882
    lda.w $0226                 ; $82B884
    sta.b $36                   ; $82B887
    stz.b $34                   ; $82B889
    lda.w $01A0                 ; $82B88B
    sta.b $38                   ; $82B88E
    lda.w #$0100                ; $82B890
    jsr.w vs_track_objects_stream; $82B893
    lda.b $38                   ; $82B896
    sta.w $01A0                 ; $82B898
    rts                         ; $82B89B
vs_track_objects_view1:
    lda.w #$00DC                ; $82B89C
    sta.b $3C                   ; $82B89F
    lda.w #$0800                ; $82B8A1
    sta.b $3A                   ; $82B8A4
    lda.w $02D9                 ; $82B8A6
    sta.b $36                   ; $82B8A9
    lda.w #$0080                ; $82B8AB
    sta.b $34                   ; $82B8AE
    lda.b $A4                   ; $82B8B0
    sta.b $38                   ; $82B8B2
    lda.w #$0180                ; $82B8B4
    jsr.w vs_track_objects_stream; $82B8B7
    lda.b $38                   ; $82B8BA
    sta.b $A4                   ; $82B8BC
    rts                         ; $82B8BE
vs_track_objects_view2:
    lda.w #$01B8                ; $82B8BF
    sta.b $3C                   ; $82B8C2
    lda.w #$1000                ; $82B8C4
    sta.b $3A                   ; $82B8C7
    lda.w $0288                 ; $82B8C9
    sta.b $36                   ; $82B8CC
    stz.b $34                   ; $82B8CE
    lda.b $A8                   ; $82B8D0
    sta.b $38                   ; $82B8D2
    lda.w #$0100                ; $82B8D4
    jsr.w vs_track_objects_stream; $82B8D7
    lda.b $38                   ; $82B8DA
    sta.b $A8                   ; $82B8DC
    rts                         ; $82B8DE
vs_track_objects_view3:
    lda.w #$0294                ; $82B8DF
    sta.b $3C                   ; $82B8E2
    lda.w #$1800                ; $82B8E4
    sta.b $3A                   ; $82B8E7
    lda.w $033B                 ; $82B8E9
    sta.b $36                   ; $82B8EC
    lda.w #$0080                ; $82B8EE
    sta.b $34                   ; $82B8F1
    lda.b $AC                   ; $82B8F3
    sta.b $38                   ; $82B8F5
    lda.w #$0180                ; $82B8F7
    jsr.w vs_track_objects_stream; $82B8FA
    lda.b $38                   ; $82B8FD
    sta.b $AC                   ; $82B8FF
    rts                         ; $82B901
vs_track_objects_stream:
    ldx.b $3C                   ; $82B902
    sta.l $7E45EC,x             ; $82B904
    ldx.b $38                   ; $82B908
    lda.b $36                   ; $82B90A
    cmp.l $7E5B91,x             ; $82B90C
    bcs CODE_82B915             ; $82B910
    incbin !baserom:$13912..$13915
CODE_82B915:
    lda.l $7E5B94,x             ; $82B915
    db $F0,$5C ; beq            ; $82B919
    bit.w #$FF00                ; $82B91B
    bne CODE_82B98C             ; $82B91E
    asl a                       ; $82B920
    tax                         ; $82B921
    lda.l $86AE36,x             ; $82B922
    tax                         ; $82B926
    lda.l $86001E,x             ; $82B927
    sta.b $14                   ; $82B92B
    lda.l $860020,x             ; $82B92D
    sta.b $16                   ; $82B931
    lda.l $860000,x             ; $82B933
    tax                         ; $82B937
    lda.l $7E6591,x             ; $82B938
    clc                         ; $82B93C
    adc.b $14                   ; $82B93D
    sta.b $14                   ; $82B93F
    ldx.b $38                   ; $82B941
    lda.l $7E5B93,x             ; $82B943
    and.w #$00FF                ; $82B947
    xba                         ; $82B94A
    lsr a                       ; $82B94B
    lsr a                       ; $82B94C
    lsr a                       ; $82B94D
    clc                         ; $82B94E
    adc.b $3A                   ; $82B94F
    sta.b $18                   ; $82B951
    lda.l $7E5B93,x             ; $82B953
    and.w #$00FF                ; $82B957
    asl a                       ; $82B95A
    sta.b $1A                   ; $82B95B
    lda.l $7E5B93,x             ; $82B95D
    xba                         ; $82B961
    and.w #$00FF                ; $82B962
    asl a                       ; $82B965
    txy                         ; $82B966
    clc                         ; $82B967
    adc.b $3C                   ; $82B968
    tax                         ; $82B96A
    lda.b $1A                   ; $82B96B
    clc                         ; $82B96D
    adc.b $34                   ; $82B96E
    sta.l $7E45EC,x             ; $82B970
    tyx                         ; $82B974
    bra CODE_82B9E3             ; $82B975
    incbin !baserom:$13977..$1398C
CODE_82B98C:
    asl a                       ; $82B98C
    tax                         ; $82B98D
    lda.l $86AC36,x             ; $82B98E
    tax                         ; $82B992
    lda.l $860022,x             ; $82B993
    sta.b $14                   ; $82B997
    lda.l $860024,x             ; $82B999
    sta.l $810116               ; $82B99D
    lda.l $860000,x             ; $82B9A1
    tax                         ; $82B9A5
    lda.l $7E6591,x             ; $82B9A6
    clc                         ; $82B9AA
    adc.b $14                   ; $82B9AB
    sta.b $14                   ; $82B9AD
    ldx.b $38                   ; $82B9AF
    lda.l $7E5B93,x             ; $82B9B1
    and.w #$00FF                ; $82B9B5
    xba                         ; $82B9B8
    lsr a                       ; $82B9B9
    lsr a                       ; $82B9BA
    lsr a                       ; $82B9BB
    clc                         ; $82B9BC
    adc.b $3A                   ; $82B9BD
    sta.b $18                   ; $82B9BF
    lda.l $7E5B93,x             ; $82B9C1
    and.w #$00FF                ; $82B9C5
    asl a                       ; $82B9C8
    sta.b $1A                   ; $82B9C9
    lda.l $7E5B93,x             ; $82B9CB
    xba                         ; $82B9CF
    and.w #$00FF                ; $82B9D0
    asl a                       ; $82B9D3
    txy                         ; $82B9D4
    clc                         ; $82B9D5
    adc.b $3C                   ; $82B9D6
    tax                         ; $82B9D8
    lda.b $1A                   ; $82B9D9
    clc                         ; $82B9DB
    adc.b $34                   ; $82B9DC
    sta.l $7E495C,x             ; $82B9DE
    tyx                         ; $82B9E2
CODE_82B9E3:
    phx                         ; $82B9E3
    ldx.b $14                   ; $82B9E4
    ldy.b $18                   ; $82B9E6
    cpy.w #$1000                ; $82B9E8
    db $90,$05 ; bcc            ; $82B9EB
    cpy.w #$3000                ; $82B9ED
    bcc CODE_82B9F3             ; $82B9F0
    incbin !baserom:$139F2..$139F3
CODE_82B9F3:
    lda.w #$007F                ; $82B9F3
    sta.b $14                   ; $82B9F6
    lda.b $16                   ; $82B9F8
    jsl.l upload_vram_now       ; $82B9FA
    plx                         ; $82B9FE
    inx                         ; $82B9FF
    inx                         ; $82BA00
    inx                         ; $82BA01
    inx                         ; $82BA02
    inx                         ; $82BA03
    stx.b $38                   ; $82BA04
    lda.b $36                   ; $82BA06
    cmp.l $7E5B91,x             ; $82BA08
    bcc CODE_82BA11             ; $82BA0C
    jmp.w $B915                 ; $82BA0E
CODE_82BA11:
    rts                         ; $82BA11
vs_race_reset:
    ldx.w #$0166                ; $82BA12
    lda.w #$0000                ; $82BA15
CODE_82BA18:
    sta.l $8101FE,x             ; $82BA18
    dex                         ; $82BA1C
    dex                         ; $82BA1D
    bne CODE_82BA18             ; $82BA1E
    sep.b #$20                  ; $82BA20
    lda.b #$04                  ; $82BA22
    sta.w $024E                 ; $82BA24
    sta.w $0301                 ; $82BA27
    sta.w $02B0                 ; $82BA2A
    sta.w $0363                 ; $82BA2D
    rep.b #$20                  ; $82BA30
    stz.w $0231                 ; $82BA32
    stz.w $02E4                 ; $82BA35
    stz.w $0293                 ; $82BA38
    stz.w $0346                 ; $82BA3B
    stz.w $0216                 ; $82BA3E
    stz.w $02C9                 ; $82BA41
    stz.w $0278                 ; $82BA44
    stz.w $032B                 ; $82BA47
    lda.w #$0020                ; $82BA4A
    sta.w $02A4                 ; $82BA4D
    lda.w #$0080                ; $82BA50
    tsb.w $1F32                 ; $82BA53
    stz.w $00B4                 ; $82BA56
    stz.w $00B6                 ; $82BA59
    stz.w $00B8                 ; $82BA5C
    stz.w $022D                 ; $82BA5F
    stz.w $022F                 ; $82BA62
    stz.w $02E0                 ; $82BA65
    stz.w $02E2                 ; $82BA68
    stz.w $028F                 ; $82BA6B
    stz.w $0291                 ; $82BA6E
    stz.w $0342                 ; $82BA71
    stz.w $0344                 ; $82BA74
    stz.w $0221                 ; $82BA77
    stz.w $02D4                 ; $82BA7A
    stz.w $0283                 ; $82BA7D
    stz.w $0336                 ; $82BA80
    lda.w #$0000                ; $82BA83
    sta.l $7E4E3D               ; $82BA86
    sta.l $7E4E3F               ; $82BA8A
    sta.l $7E4E41               ; $82BA8E
    sta.l $7E4E43               ; $82BA92
    lda.w #$0001                ; $82BA96
    sta.w $0233                 ; $82BA99
    sta.w $02E6                 ; $82BA9C
    sta.w $0295                 ; $82BA9F
    sta.w $0348                 ; $82BAA2
    stz.w $1ED0                 ; $82BAA5
    lda.w #$0001                ; $82BAA8
    sta.w $0244                 ; $82BAAB
    sta.w $02F7                 ; $82BAAE
    sta.w $02A6                 ; $82BAB1
    sta.w $0359                 ; $82BAB4
    lda.w #$0000                ; $82BAB7
    sta.w $1ED2                 ; $82BABA
    inc a                       ; $82BABD
    sta.w $1ED4                 ; $82BABE
    inc a                       ; $82BAC1
    sta.w $1ED6                 ; $82BAC2
    inc a                       ; $82BAC5
    sta.w $1ED8                 ; $82BAC6
    lda.w #$FFFF                ; $82BAC9
    sta.w $0235                 ; $82BACC
    sta.w $02E8                 ; $82BACF
    sta.w $0297                 ; $82BAD2
    sta.w $034A                 ; $82BAD5
    stz.w $021F                 ; $82BAD8
    stz.w $02D2                 ; $82BADB
    stz.w $0281                 ; $82BADE
    stz.w $0334                 ; $82BAE1
    stz.w $021A                 ; $82BAE4
    stz.w $02CD                 ; $82BAE7
    stz.w $027C                 ; $82BAEA
    stz.w $032F                 ; $82BAED
    stz.w $00BE                 ; $82BAF0
    stz.w $01A0                 ; $82BAF3
    stz.w $01A4                 ; $82BAF6
    stz.w $01A8                 ; $82BAF9
    stz.w $01AC                 ; $82BAFC
    stz.w $00BE                 ; $82BAFF
    stz.w $00C0                 ; $82BB02
    stz.w $00BC                 ; $82BB05
    stz.w $0486                 ; $82BB08
    stz.w $00C4                 ; $82BB0B
    stz.w $00C6                 ; $82BB0E
    stz.w $00C2                 ; $82BB11
    stz.w $050E                 ; $82BB14
    stz.w $0176                 ; $82BB17
    stz.w $017A                 ; $82BB1A
    stz.w $0208                 ; $82BB1D
    stz.w $02BB                 ; $82BB20
    stz.w $026A                 ; $82BB23
    stz.w $031D                 ; $82BB26
    stz.w $0200                 ; $82BB29
    stz.w $0202                 ; $82BB2C
    stz.w $02B3                 ; $82BB2F
    stz.w $02B5                 ; $82BB32
    stz.w $0262                 ; $82BB35
    stz.w $0264                 ; $82BB38
    stz.w $0315                 ; $82BB3B
    stz.w $0317                 ; $82BB3E
    stz.w $0225                 ; $82BB41
    stz.w $0227                 ; $82BB44
    stz.w $02D8                 ; $82BB47
    stz.w $02DA                 ; $82BB4A
    stz.w $0287                 ; $82BB4D
    stz.w $0289                 ; $82BB50
    stz.w $033A                 ; $82BB53
    stz.w $033C                 ; $82BB56
    lda.w #$0008                ; $82BB59
    tax                         ; $82BB5C
    lda.l $85A33A,x             ; $82BB5D
    sta.w $1F28                 ; $82BB61
    lda.l $85A33C,x             ; $82BB64
    sta.w $1F2A                 ; $82BB68
    lda.w #$0020                ; $82BB6B
    sta.w $0226                 ; $82BB6E
    stz.w $023E                 ; $82BB71
    stz.w $02F1                 ; $82BB74
    stz.w $0210                 ; $82BB77
    stz.w $02C3                 ; $82BB7A
    stz.w $0272                 ; $82BB7D
    stz.w $0325                 ; $82BB80
    stz.w $0253                 ; $82BB83
    stz.w $0306                 ; $82BB86
    stz.w $0223                 ; $82BB89
    stz.w $02D6                 ; $82BB8C
    stz.w $0285                 ; $82BB8F
    stz.w $0338                 ; $82BB92
    stz.w $0240                 ; $82BB95
    stz.w $02F3                 ; $82BB98
    stz.w $02A2                 ; $82BB9B
    stz.w $0355                 ; $82BB9E
    lda.w #$57F6                ; $82BBA1
    sta.w $00A4                 ; $82BBA4
    sta.w $00AC                 ; $82BBA7
    lda.w #$584E                ; $82BBAA
    sta.w $00A6                 ; $82BBAD
    sta.w $00AE                 ; $82BBB0
    lda.w #$0000                ; $82BBB3
    sta.l $308000               ; $82BBB6
    lda.w $019A                 ; $82BBBA
    sta.l $308000               ; $82BBBD
    lda.w $1F06                 ; $82BBC1
    sta.l $308000               ; $82BBC4
    nop                         ; $82BBC8
    lda.l $308000               ; $82BBC9
    sta.b $14                   ; $82BBCD
    lda.l $308000               ; $82BBCF
    ldx.w #$0010                ; $82BBD3
    lda.w #$00B0                ; $82BBD6
CODE_82BBD9:
    cmp.b $14                   ; $82BBD9
    bcc CODE_82BBDF             ; $82BBDB
    sbc.b $14                   ; $82BBDD
CODE_82BBDF:
    rol.b $16                   ; $82BBDF
    asl a                       ; $82BBE1
    dex                         ; $82BBE2
    bne CODE_82BBD9             ; $82BBE3
    lda.b $16                   ; $82BBE5
    sta.w $019E                 ; $82BBE7
    stz.w $003E                 ; $82BBEA
    stz.w $0040                 ; $82BBED
    stz.w $0064                 ; $82BBF0
    stz.w $0068                 ; $82BBF3
    stz.w $00FE                 ; $82BBF6
    stz.w $0100                 ; $82BBF9
    stz.w $1E02                 ; $82BBFC
    stz.w $1E0A                 ; $82BBFF
    stz.w $1E12                 ; $82BC02
    stz.w $1E1A                 ; $82BC05
    jsr.w track_palettes_load   ; $82BC08
    jsr.w vs_grid_setup         ; $82BC0B
    lda.w #$FF40                ; $82BC0E
    stz.w $0204                 ; $82BC11
    sta.w $0206                 ; $82BC14
    lda.w #$FFC0                ; $82BC17
    stz.w $02B7                 ; $82BC1A
    sta.w $02B9                 ; $82BC1D
    lda.w #$0040                ; $82BC20
    stz.w $0266                 ; $82BC23
    sta.w $0268                 ; $82BC26
    lda.w #$00C0                ; $82BC29
    stz.w $0319                 ; $82BC2C
    sta.w $031B                 ; $82BC2F
    ldx.w #$0000                ; $82BC32
    lda.l $7EFAB6,x             ; $82BC35
    sec                         ; $82BC39
    sbc.w #$0030                ; $82BC3A
    sta.w $0226                 ; $82BC3D
    lda.w $0204                 ; $82BC40
    sta.l $7EFAB9,x             ; $82BC43
    lda.w $0206                 ; $82BC47
    sta.l $7EFABB,x             ; $82BC4A
    ldx.w #$0035                ; $82BC4E
    lda.l $7EFAB6,x             ; $82BC51
    sec                         ; $82BC55
    sbc.w #$0030                ; $82BC56
    sta.w $02D9                 ; $82BC59
    lda.w $02B7                 ; $82BC5C
    sta.l $7EFAB9,x             ; $82BC5F
    lda.w $02B9                 ; $82BC63
    sta.l $7EFABB,x             ; $82BC66
    ldx.w #$006A                ; $82BC6A
    lda.l $7EFAB6,x             ; $82BC6D
    sec                         ; $82BC71
    sbc.w #$0030                ; $82BC72
    sta.w $0288                 ; $82BC75
    lda.w $0266                 ; $82BC78
    sta.l $7EFAB9,x             ; $82BC7B
    lda.w $0268                 ; $82BC7F
    sta.l $7EFABB,x             ; $82BC82
    ldx.w #$009F                ; $82BC86
    lda.l $7EFAB6,x             ; $82BC89
    sec                         ; $82BC8D
    sbc.w #$0030                ; $82BC8E
    sta.w $033B                 ; $82BC91
    lda.w $0319                 ; $82BC94
    sta.l $7EFAB9,x             ; $82BC97
    lda.w $031B                 ; $82BC9B
    sta.l $7EFABB,x             ; $82BC9E
    sec                         ; $82BCA2
    lda.w $0226                 ; $82BCA3
    sbc.w #$0020                ; $82BCA6
    sta.w $023E                 ; $82BCA9
    lda.w $02D9                 ; $82BCAC
    sbc.w #$0020                ; $82BCAF
    sta.w $02F1                 ; $82BCB2
    lda.w $0288                 ; $82BCB5
    sbc.w #$0020                ; $82BCB8
    sta.w $02A0                 ; $82BCBB
    lda.w $033B                 ; $82BCBE
    sbc.w #$0020                ; $82BCC1
    sta.w $0353                 ; $82BCC4
    sep.b #$20                  ; $82BCC7
    lda.b #$C1                  ; $82BCC9
    sta.w $2121                 ; $82BCCB
    lda.b #$05                  ; $82BCCE
    sta.b $14                   ; $82BCD0
    ldx.w #$0000                ; $82BCD2
CODE_82BCD5:
    ldy.w #$001E                ; $82BCD5
CODE_82BCD8:
    lda.l $85A118,x             ; $82BCD8
    sta.w $2122                 ; $82BCDC
    inx                         ; $82BCDF
    dey                         ; $82BCE0
    bne CODE_82BCD8             ; $82BCE1
    sta.w $2122                 ; $82BCE3
    sta.w $2122                 ; $82BCE6
    inc.b $16                   ; $82BCE9
    inc.b $16                   ; $82BCEB
    dec.b $14                   ; $82BCED
    bne CODE_82BCD5             ; $82BCEF
    rep.b #$20                  ; $82BCF1
    lda.w #$0300                ; $82BCF3
    sta.w $023A                 ; $82BCF6
    sta.w $02ED                 ; $82BCF9
    sta.w $029C                 ; $82BCFC
    sta.w $034F                 ; $82BCFF
    stz.w $1F08                 ; $82BD02
    rts                         ; $82BD05
    incbin !baserom:$13D06..$13D0E
vs_grid_setup:
    ldy.w #$0004                ; $82BD0E
    ldx.w #$0000                ; $82BD11
    lda.w $019A                 ; $82BD14
    sec                         ; $82BD17
    sbc.w #$0004                ; $82BD18
    sta.b $14                   ; $82BD1B
    lda.w #$0800                ; $82BD1D
    sta.b $16                   ; $82BD20
    lda.w #$0008                ; $82BD22
    sta.b $18                   ; $82BD25
CODE_82BD27:
    lda.w #$0000                ; $82BD27
    sta.l $7EFAB5,x             ; $82BD2A
    sta.l $7EFAB7,x             ; $82BD2E
    sta.l $7EFABD,x             ; $82BD32
    sta.l $7EFABF,x             ; $82BD36
    sta.l $7EFAC5,x             ; $82BD3A
    lda.w #$0002                ; $82BD3E
    sta.l $7EFADE,x             ; $82BD41
    lda.b $14                   ; $82BD45
    sta.l $7EFAB6,x             ; $82BD47
    clc                         ; $82BD4B
    adc.w #$0001                ; $82BD4C
    sta.b $14                   ; $82BD4F
    lda.b $16                   ; $82BD51
    sta.l $7EFAD1,x             ; $82BD53
    clc                         ; $82BD57
    adc.w #$0200                ; $82BD58
    sta.b $16                   ; $82BD5B
    lda.b $18                   ; $82BD5D
    sta.l $7EFAC5,x             ; $82BD5F
    inc a                       ; $82BD63
    inc a                       ; $82BD64
    sta.b $18                   ; $82BD65
    txa                         ; $82BD67
    clc                         ; $82BD68
    adc.w #$0035                ; $82BD69
    tax                         ; $82BD6C
    dey                         ; $82BD6D
    bne CODE_82BD27             ; $82BD6E
    lda.w #$7FFF                ; $82BD70
    sta.l $7EFAB6,x             ; $82BD73
    lda.w #$0006                ; $82BD77
    sta.l $7EFAC5,x             ; $82BD7A
    clc                         ; $82BD7E
    lda.w #$0000                ; $82BD7F
    sta.w $1E6A                 ; $82BD82
    adc.w #$0035                ; $82BD85
    sta.w $1E6C                 ; $82BD88
    adc.w #$0035                ; $82BD8B
    sta.w $1E6E                 ; $82BD8E
    adc.w #$0035                ; $82BD91
    sta.w $1E70                 ; $82BD94
    adc.w #$0035                ; $82BD97
    ldx.w #$0000                ; $82BD9A
    ldy.w #$0010                ; $82BD9D
CODE_82BDA0:
    sta.w $1E72,x               ; $82BDA0
    inx                         ; $82BDA3
    inx                         ; $82BDA4
    dey                         ; $82BDA5
    bne CODE_82BDA0             ; $82BDA6
    rts                         ; $82BDA8
vs_steps_share:
    lda.w $01CE                 ; $82BDA9
    sta.w $0229                 ; $82BDAC
    jsr.w vs_step_share         ; $82BDAF
    sta.w $01CE                 ; $82BDB2
    lda.w $01D2                 ; $82BDB5
    sta.w $02DC                 ; $82BDB8
    jsr.w vs_step_share         ; $82BDBB
    sta.w $01D2                 ; $82BDBE
    lda.w $01D6                 ; $82BDC1
    sta.w $028B                 ; $82BDC4
    jsr.w vs_step_share         ; $82BDC7
    sta.w $01D6                 ; $82BDCA
    lda.w $01DA                 ; $82BDCD
    sta.w $033E                 ; $82BDD0
    jsr.w vs_step_share         ; $82BDD3
    sta.w $01DA                 ; $82BDD6
    rts                         ; $82BDD9
vs_step_share:
    bpl CODE_82BDF8             ; $82BDDA
    eor.w #$FFFF                ; $82BDDC
    inc a                       ; $82BDDF
    sta.w $4204                 ; $82BDE0
    sep.b #$20                  ; $82BDE3
    lda.w $1E9E                 ; $82BDE5
    sta.w $4206                 ; $82BDE8
    rep.b #$20                  ; $82BDEB
    nop                         ; $82BDED
    nop                         ; $82BDEE
    nop                         ; $82BDEF
    nop                         ; $82BDF0
    nop                         ; $82BDF1
    nop                         ; $82BDF2
    nop                         ; $82BDF3
    lda.w $4214                 ; $82BDF4
    rts                         ; $82BDF7
CODE_82BDF8:
    sta.w $4204                 ; $82BDF8
    sep.b #$20                  ; $82BDFB
    lda.w $1E9E                 ; $82BDFD
    sta.w $4206                 ; $82BE00
    rep.b #$20                  ; $82BE03
    nop                         ; $82BE05
    nop                         ; $82BE06
    nop                         ; $82BE07
    nop                         ; $82BE08
    lda.w #$0000                ; $82BE09
    sec                         ; $82BE0C
    sbc.w $4214                 ; $82BE0D
    rts                         ; $82BE10
    incbin !baserom:$13E11..$13E18
vs_sprites_view0:
    pea.w $8330                 ; $82BE18
    plb                         ; $82BE1B
    lda.w $0186                 ; $82BE1C
    sta.w $0184                 ; $82BE1F
    lda.w #$000E                ; $82BE22
    sta.w $8000                 ; $82BE25
    lda.b $8E                   ; $82BE28
    clc                         ; $82BE2A
    adc.w #$0100                ; $82BE2B
    sta.b $1A                   ; $82BE2E
    lda.w #$1000                ; $82BE30
    sta.b $3C                   ; $82BE33
CODE_82BE35:
    lda.w $C000                 ; $82BE35
    bpl CODE_82BE35             ; $82BE38
    lda.w #$0005                ; $82BE3A
    sta.w $8000                 ; $82BE3D
    ldy.w #$0008                ; $82BE40
    lda.l $811E70               ; $82BE43
    tax                         ; $82BE47
    lda.w $0225                 ; $82BE48
    cmp.l $7EFAB5,x             ; $82BE4B
    lda.w $0227                 ; $82BE4F
    sbc.l $7EFAB7,x             ; $82BE52
    bcs CODE_82BE75             ; $82BE56
    ldy.w #$0006                ; $82BE58
CODE_82BE5B:
    tyx                         ; $82BE5B
    lda.l $811E68,x             ; $82BE5C
    tax                         ; $82BE60
    lda.w $0225                 ; $82BE61
    cmp.l $7EFAB5,x             ; $82BE64
    lda.w $0227                 ; $82BE68
    sbc.l $7EFAB7,x             ; $82BE6B
    bcs CODE_82BE75             ; $82BE6F
    dey                         ; $82BE71
    dey                         ; $82BE72
    bne CODE_82BE5B             ; $82BE73
CODE_82BE75:
    sty.b $28                   ; $82BE75
    lda.w $0226                 ; $82BE77
    sta.b $2C                   ; $82BE7A
CODE_82BE7C:
    lda.w $C000                 ; $82BE7C
    bpl CODE_82BE7C             ; $82BE7F
    lda.w #$0009                ; $82BE81
    sta.w $8000                 ; $82BE84
    lda.w #$003C                ; $82BE87
    sta.w $8000                 ; $82BE8A
    lda.w #$0039                ; $82BE8D
    sta.w $8000                 ; $82BE90
    stz.w $8000                 ; $82BE93
    lda.w #$0000                ; $82BE96
    sta.w $8000                 ; $82BE99
    lda.w #$0078                ; $82BE9C
    sta.w $8000                 ; $82BE9F
    lda.w #$0004                ; $82BEA2
    sta.w $8000                 ; $82BEA5
    lda.w #$006C                ; $82BEA8
    sta.w $8000                 ; $82BEAB
    stz.b $14                   ; $82BEAE
    lda.w $0226                 ; $82BEB0
    asl a                       ; $82BEB3
    asl a                       ; $82BEB4
    sta.b $16                   ; $82BEB5
    lda.w $019A                 ; $82BEB7
    asl a                       ; $82BEBA
    asl a                       ; $82BEBB
    sta.b $18                   ; $82BEBC
    stz.w $1DF6                 ; $82BEBE
    stz.w $1DF8                 ; $82BEC1
    lda.l $7E34EE               ; $82BEC4
    sec                         ; $82BEC8
    sbc.w $020C                 ; $82BEC9
    sta.l $7E34EC               ; $82BECC
    lda.w #$006C                ; $82BED0
    sta.l $7E37EC               ; $82BED3
    lda.w $0209                 ; $82BED7
    and.w #$00FF                ; $82BEDA
    bit.w #$0080                ; $82BEDD
    beq CODE_82BEE5             ; $82BEE0
    incbin !baserom:$13EE2..$13EE5
CODE_82BEE5:
    eor.w #$FFFF                ; $82BEE5
    sec                         ; $82BEE8
    adc.l $7E36EE               ; $82BEE9
    sta.l $7E36EC               ; $82BEED
    lda.l $7E39EE               ; $82BEF1
    sec                         ; $82BEF5
    sbc.w #$0080                ; $82BEF6
    sta.l $7E39EC               ; $82BEF9
    jmp.w $BF6D                 ; $82BEFD
    ldx.b $16                   ; $82BF00
    txa                         ; $82BF02
    clc                         ; $82BF03
    adc.w #$0004                ; $82BF04
    cmp.b $18                   ; $82BF07
    bcc CODE_82BF0D             ; $82BF09
    sbc.b $18                   ; $82BF0B
CODE_82BF0D:
    sta.b $16                   ; $82BF0D
    lda.l $7E8CB3,x             ; $82BF0F
    beq CODE_82BF60             ; $82BF13
    stx.b $1C                   ; $82BF15
    xba                         ; $82BF17
    and.w #$007F                ; $82BF18
    asl a                       ; $82BF1B
    tax                         ; $82BF1C
    stx.b $1E                   ; $82BF1D
    lda.b $3C                   ; $82BF1F
    sta.b $3A                   ; $82BF21
    ldx.b $14                   ; $82BF23
    lda.l $7E39EC,x             ; $82BF25
    cmp.w #$03C2                ; $82BF29
    bcs CODE_82BF47             ; $82BF2C
    ldx.b $1E                   ; $82BF2E
    lda.l $86AD66,x             ; $82BF30
    tax                         ; $82BF34
    lda.l $7E45EC,x             ; $82BF35
    sta.b $26                   ; $82BF39
    ldx.b $1E                   ; $82BF3B
    jsr.w ($7FFE,x)             ; $82BF3D
    bcs CODE_82BF44             ; $82BF40
    bra CODE_82BF6D             ; $82BF42
CODE_82BF44:
    jmp.w $C112                 ; $82BF44
CODE_82BF47:
    ldx.b $1E                   ; $82BF47
    lda.l $86AD66,x             ; $82BF49
    tax                         ; $82BF4D
    lda.l $7E495C,x             ; $82BF4E
    sta.b $26                   ; $82BF52
    ldx.b $1E                   ; $82BF54
    jsr.w ($7FFE,x)             ; $82BF56
    bcs CODE_82BF44             ; $82BF59
    bra CODE_82BF6D             ; $82BF5B
    incbin !baserom:$13F5D..$13F60
CODE_82BF60:
    ldx.b $14                   ; $82BF60
    lda.l $7E37EC,x             ; $82BF62
    sta.w $8000                 ; $82BF66
    nop                         ; $82BF69
    stz.w $8000                 ; $82BF6A
CODE_82BF6D:
    ldx.b $28                   ; $82BF6D
    lda.l $811E6A,x             ; $82BF6F
    tax                         ; $82BF73
    lda.l $7EFAB6,x             ; $82BF74
    cmp.b $2C                   ; $82BF78
    beq CODE_82BF7F             ; $82BF7A
CODE_82BF7C:
    jmp.w $C0F8                 ; $82BF7C
CODE_82BF7F:
    txy                         ; $82BF7F
    ldx.b $14                   ; $82BF80
    inx                         ; $82BF82
    inx                         ; $82BF83
    cpx.b $90                   ; $82BF84
    beq CODE_82BF7C             ; $82BF86
    tyx                         ; $82BF88
    lda.l $7EFAB5,x             ; $82BF89
    lsr a                       ; $82BF8D
    and.w #$007E                ; $82BF8E
    ldx.b $14                   ; $82BF91
    clc                         ; $82BF93
    adc.l $7E39EC,x             ; $82BF94
    tax                         ; $82BF98
    lda.l $83ACEC,x             ; $82BF99
    sta.b $34                   ; $82BF9D
    lda.w $0244                 ; $82BF9F
    bit.w #$0001                ; $82BFA2
    bne CODE_82C022             ; $82BFA5
    cpy.w #$0000                ; $82BFA7
    bne CODE_82C022             ; $82BFAA
    lda.w $0231                 ; $82BFAC
    beq CODE_82BFC2             ; $82BFAF
    incbin !baserom:$13FB1..$13FC2
CODE_82BFC2:
    ldx.b $14                   ; $82BFC2
    lda.l $7E37EC,x             ; $82BFC4
    sta.w $8000                 ; $82BFC8
    lda.w #$9000                ; $82BFCB
    sta.w $8000                 ; $82BFCE
    tyx                         ; $82BFD1
    lda.l $7EFAB4,x             ; $82BFD2
    sta.w $8000                 ; $82BFD6
    lda.w #$0033                ; $82BFD9
    sta.w $8000                 ; $82BFDC
    nop                         ; $82BFDF
    sta.w $8000                 ; $82BFE0
    lda.w $0202                 ; $82BFE3
    sta.w $8000                 ; $82BFE6
    nop                         ; $82BFE9
    sta.w $8000                 ; $82BFEA
    lda.b $34                   ; $82BFED
    sta.w $8000                 ; $82BFEF
    tyx                         ; $82BFF2
    lda.w $0202                 ; $82BFF3
    sta.w $8000                 ; $82BFF6
    nop                         ; $82BFF9
    lda.w $8000                 ; $82BFFA
    sta.b $2A                   ; $82BFFD
    lda.l $7EFABF,x             ; $82BFFF
    sta.w $8000                 ; $82C003
    lda.b $3C                   ; $82C006
    ora.l $7EFAD1,x             ; $82C008
    sta.w $8000                 ; $82C00C
    lda.w $023B                 ; $82C00F
    asl a                       ; $82C012
    and.w #$00FE                ; $82C013
    tax                         ; $82C016
    lda.l $86C89E,x             ; $82C017
    tax                         ; $82C01B
    jsr.w sprite_parts_stream   ; $82C01C
    jmp.w $C0E0                 ; $82C01F
CODE_82C022:
    ldx.b $14                   ; $82C022
    lda.l $7E37EC,x             ; $82C024
    sta.w $8000                 ; $82C028
    lda.w #$9000                ; $82C02B
    sta.w $8000                 ; $82C02E
    tyx                         ; $82C031
    lda.l $7EFAB4,x             ; $82C032
    sta.w $8000                 ; $82C036
    ldx.b $14                   ; $82C039
    lda.l $7E36EE,x             ; $82C03B
    sta.w $8000                 ; $82C03F
    lda.l $7E36EC,x             ; $82C042
    sta.w $8000                 ; $82C046
    lda.l $7E34EE,x             ; $82C049
    sta.w $8000                 ; $82C04D
    lda.l $7E34EC,x             ; $82C050
    sta.w $8000                 ; $82C054
    tyx                         ; $82C057
    lda.l $7EFAB5,x             ; $82C058
    lsr a                       ; $82C05C
    and.w #$007E                ; $82C05D
    ldx.b $14                   ; $82C060
    clc                         ; $82C062
    adc.l $7E39EC,x             ; $82C063
    tax                         ; $82C067
    lda.l $83ACEC,x             ; $82C068
    sta.w $8000                 ; $82C06C
    phx                         ; $82C06F
    tyx                         ; $82C070
    lda.l $7EFABB,x             ; $82C071
    sta.w $8000                 ; $82C075
    nop                         ; $82C078
    lda.w $8000                 ; $82C079
    sta.b $2A                   ; $82C07C
    lda.l $7EFABF,x             ; $82C07E
    sta.w $8000                 ; $82C082
    lda.b $3C                   ; $82C085
    ora.l $7EFAD1,x             ; $82C087
    sta.w $8000                 ; $82C08B
    ldx.b $14                   ; $82C08E
    lda.l $7E34EC,x             ; $82C090
    sec                         ; $82C094
    sbc.l $7E34EE,x             ; $82C095
    asl a                       ; $82C099
    eor.w #$FFFF                ; $82C09A
    sec                         ; $82C09D
    adc.b $2A                   ; $82C09E
    ldx.w #$0000                ; $82C0A0
    cmp.w #$FFA0                ; $82C0A3
    bmi CODE_82C0CD             ; $82C0A6
    inx                         ; $82C0A8
    inx                         ; $82C0A9
    cmp.w #$FFC0                ; $82C0AA
    bmi CODE_82C0CD             ; $82C0AD
    inx                         ; $82C0AF
    inx                         ; $82C0B0
    cmp.w #$FFE8                ; $82C0B1
    bmi CODE_82C0CD             ; $82C0B4
    inx                         ; $82C0B6
    inx                         ; $82C0B7
    cmp.w #$0018                ; $82C0B8
    bmi CODE_82C0CD             ; $82C0BB
    inx                         ; $82C0BD
    inx                         ; $82C0BE
    cmp.w #$0040                ; $82C0BF
    bmi CODE_82C0CD             ; $82C0C2
    inx                         ; $82C0C4
    inx                         ; $82C0C5
    cmp.w #$0060                ; $82C0C6
    bmi CODE_82C0CD             ; $82C0C9
    inx                         ; $82C0CB
    inx                         ; $82C0CC
CODE_82C0CD:
    lda.l $86AFD8,x             ; $82C0CD
    plx                         ; $82C0D1
    clc                         ; $82C0D2
    adc.l $849180,x             ; $82C0D3
    tax                         ; $82C0D7
    lda.l $860000,x             ; $82C0D8
    tax                         ; $82C0DC
    jsr.w sprite_parts_stream   ; $82C0DD
    bcs CODE_82C112             ; $82C0E0
    inc.b $28                   ; $82C0E2
    inc.b $28                   ; $82C0E4
    ldx.b $28                   ; $82C0E6
    lda.l $811E6A,x             ; $82C0E8
    tax                         ; $82C0EC
    lda.l $7EFAB6,x             ; $82C0ED
    cmp.b $2C                   ; $82C0F1
    bne CODE_82C0F8             ; $82C0F3
    jmp.w $BF7F                 ; $82C0F5
CODE_82C0F8:
    inc.b $2C                   ; $82C0F8
    lda.b $2C                   ; $82C0FA
    cmp.w $019A                 ; $82C0FC
    bne CODE_82C105             ; $82C0FF
    stz.b $2C                   ; $82C101
    stz.b $28                   ; $82C103
CODE_82C105:
    inc.b $14                   ; $82C105
    inc.b $14                   ; $82C107
    lda.b $14                   ; $82C109
    cmp.b $90                   ; $82C10B
    beq CODE_82C112             ; $82C10D
    jmp.w $BF00                 ; $82C10F
CODE_82C112:
    stz.w $8000                 ; $82C112
    lda.w #$8000                ; $82C115
    sta.w $8000                 ; $82C118
    stz.w $1E02                 ; $82C11B
    ldx.w $1DF6                 ; $82C11E
    beq CODE_82C168             ; $82C121
    incbin !baserom:$14123..$14168
CODE_82C168:
    lda.w $0184                 ; $82C168
    sta.w $0186                 ; $82C16B
    plb                         ; $82C16E
    rts                         ; $82C16F
vs_sprites_view1:
    pea.w $8330                 ; $82C170
    plb                         ; $82C173
    lda.w $0186                 ; $82C174
    sta.w $0184                 ; $82C177
    lda.w #$000E                ; $82C17A
    sta.w $8000                 ; $82C17D
    lda.b $8E                   ; $82C180
    clc                         ; $82C182
    adc.w #$0200                ; $82C183
    sta.b $1A                   ; $82C186
    lda.w #$1000                ; $82C188
    sta.b $3C                   ; $82C18B
CODE_82C18D:
    lda.w $C000                 ; $82C18D
    bpl CODE_82C18D             ; $82C190
    ldy.w #$0008                ; $82C192
    lda.l $811E70               ; $82C195
    tax                         ; $82C199
    lda.w $02D8                 ; $82C19A
    cmp.l $7EFAB5,x             ; $82C19D
    lda.w $02DA                 ; $82C1A1
    sbc.l $7EFAB7,x             ; $82C1A4
    bcs CODE_82C1C7             ; $82C1A8
    ldy.w #$0006                ; $82C1AA
CODE_82C1AD:
    tyx                         ; $82C1AD
    lda.l $811E68,x             ; $82C1AE
    tax                         ; $82C1B2
    lda.w $02D8                 ; $82C1B3
    cmp.l $7EFAB5,x             ; $82C1B6
    lda.w $02DA                 ; $82C1BA
    sbc.l $7EFAB7,x             ; $82C1BD
    bcs CODE_82C1C7             ; $82C1C1
    dey                         ; $82C1C3
    dey                         ; $82C1C4
    bne CODE_82C1AD             ; $82C1C5
CODE_82C1C7:
    sty.b $28                   ; $82C1C7
    lda.w $02D9                 ; $82C1C9
    sta.b $2C                   ; $82C1CC
    lda.w #$0009                ; $82C1CE
    sta.w $8000                 ; $82C1D1
    lda.w #$00C4                ; $82C1D4
    sta.w $8000                 ; $82C1D7
    lda.w #$0039                ; $82C1DA
    sta.w $8000                 ; $82C1DD
    stz.w $8000                 ; $82C1E0
    lda.w #$0088                ; $82C1E3
    sta.w $8000                 ; $82C1E6
    lda.w #$0100                ; $82C1E9
    sta.w $8000                 ; $82C1EC
    lda.w #$0004                ; $82C1EF
    sta.w $8000                 ; $82C1F2
    lda.w #$006C                ; $82C1F5
    sta.w $8000                 ; $82C1F8
    lda.w #$0040                ; $82C1FB
    sta.b $14                   ; $82C1FE
    lda.w $0192                 ; $82C200
    clc                         ; $82C203
    adc.w #$0040                ; $82C204
    sta.w $0192                 ; $82C207
    lda.w $02D9                 ; $82C20A
    asl a                       ; $82C20D
    asl a                       ; $82C20E
    sta.b $16                   ; $82C20F
    lda.w $019A                 ; $82C211
    asl a                       ; $82C214
    asl a                       ; $82C215
    sta.b $18                   ; $82C216
    stz.w $1DF8                 ; $82C218
    stz.w $1DFA                 ; $82C21B
    lda.l $7E352E               ; $82C21E
    sec                         ; $82C222
    sbc.w $02BF                 ; $82C223
    sta.l $7E352C               ; $82C226
    lda.w #$006C                ; $82C22A
    sta.l $7E382C               ; $82C22D
    lda.w $02BC                 ; $82C231
    and.w #$00FF                ; $82C234
    bit.w #$0080                ; $82C237
    beq CODE_82C23F             ; $82C23A
    incbin !baserom:$1423C..$1423F
CODE_82C23F:
    eor.w #$FFFF                ; $82C23F
    sec                         ; $82C242
    adc.l $7E372E               ; $82C243
    sta.l $7E372C               ; $82C247
    lda.l $7E3A2E               ; $82C24B
    sec                         ; $82C24F
    sbc.w #$0080                ; $82C250
    sta.l $7E3A2C               ; $82C253
    jmp.w $C2C7                 ; $82C257
    ldx.b $16                   ; $82C25A
    txa                         ; $82C25C
    clc                         ; $82C25D
    adc.w #$0004                ; $82C25E
    cmp.b $18                   ; $82C261
    bcc CODE_82C267             ; $82C263
    sbc.b $18                   ; $82C265
CODE_82C267:
    sta.b $16                   ; $82C267
    lda.l $7E8CB3,x             ; $82C269
    beq CODE_82C2BA             ; $82C26D
    stx.b $1C                   ; $82C26F
    xba                         ; $82C271
    and.w #$007F                ; $82C272
    asl a                       ; $82C275
    tax                         ; $82C276
    stx.b $1E                   ; $82C277
    lda.b $3C                   ; $82C279
    sta.b $3A                   ; $82C27B
    ldx.b $14                   ; $82C27D
    lda.l $7E39EC,x             ; $82C27F
    cmp.w #$03C2                ; $82C283
    bcs CODE_82C2A1             ; $82C286
    ldx.b $1E                   ; $82C288
    lda.l $86AD66,x             ; $82C28A
    tax                         ; $82C28E
    lda.l $7E46C8,x             ; $82C28F
    sta.b $26                   ; $82C293
    ldx.b $1E                   ; $82C295
    jsr.w ($7FFE,x)             ; $82C297
    bcs CODE_82C29E             ; $82C29A
    bra CODE_82C2C7             ; $82C29C
CODE_82C29E:
    jmp.w $C46E                 ; $82C29E
CODE_82C2A1:
    ldx.b $1E                   ; $82C2A1
    lda.l $86AD66,x             ; $82C2A3
    tax                         ; $82C2A7
    lda.l $7E4A38,x             ; $82C2A8
    sta.b $26                   ; $82C2AC
    ldx.b $1E                   ; $82C2AE
    jsr.w ($7FFE,x)             ; $82C2B0
    bcs CODE_82C29E             ; $82C2B3
    bra CODE_82C2C7             ; $82C2B5
    incbin !baserom:$142B7..$142BA
CODE_82C2BA:
    ldx.b $14                   ; $82C2BA
    lda.l $7E37EC,x             ; $82C2BC
    sta.w $8000                 ; $82C2C0
    nop                         ; $82C2C3
    stz.w $8000                 ; $82C2C4
CODE_82C2C7:
    ldx.b $28                   ; $82C2C7
    lda.l $811E6A,x             ; $82C2C9
    tax                         ; $82C2CD
    lda.l $7EFAB6,x             ; $82C2CE
    cmp.b $2C                   ; $82C2D2
    beq CODE_82C2D9             ; $82C2D4
CODE_82C2D6:
    jmp.w $C454                 ; $82C2D6
CODE_82C2D9:
    txy                         ; $82C2D9
    ldx.b $14                   ; $82C2DA
    inx                         ; $82C2DC
    inx                         ; $82C2DD
    cpx.b $92                   ; $82C2DE
    beq CODE_82C2D6             ; $82C2E0
    tyx                         ; $82C2E2
    lda.l $7EFAB5,x             ; $82C2E3
    lsr a                       ; $82C2E7
    and.w #$007E                ; $82C2E8
    ldx.b $14                   ; $82C2EB
    clc                         ; $82C2ED
    adc.l $7E39EC,x             ; $82C2EE
    tax                         ; $82C2F2
    lda.l $83ACEC,x             ; $82C2F3
    sta.b $34                   ; $82C2F7
    lda.w $02F7                 ; $82C2F9
    bit.w #$0001                ; $82C2FC
    bne CODE_82C304             ; $82C2FF
    cpy.w #$0035                ; $82C301
CODE_82C304:
    bne CODE_82C37E             ; $82C304
    lda.w $02E4                 ; $82C306
    beq CODE_82C31C             ; $82C309
    incbin !baserom:$1430B..$1431C
CODE_82C31C:
    ldx.b $14                   ; $82C31C
    lda.l $7E37EC,x             ; $82C31E
    sta.w $8000                 ; $82C322
    lda.w #$9000                ; $82C325
    sta.w $8000                 ; $82C328
    tyx                         ; $82C32B
    lda.l $7EFAB4,x             ; $82C32C
    sta.w $8000                 ; $82C330
    ldx.b $14                   ; $82C333
    lda.w #$0033                ; $82C335
    sta.w $8000                 ; $82C338
    nop                         ; $82C33B
    sta.w $8000                 ; $82C33C
    lda.w $02B5                 ; $82C33F
    sta.w $8000                 ; $82C342
    nop                         ; $82C345
    sta.w $8000                 ; $82C346
    lda.b $34                   ; $82C349
    sta.w $8000                 ; $82C34B
    tyx                         ; $82C34E
    lda.w $02B5                 ; $82C34F
    sta.w $8000                 ; $82C352
    nop                         ; $82C355
    lda.w $8000                 ; $82C356
    sta.b $2A                   ; $82C359
    lda.l $7EFABF,x             ; $82C35B
    sta.w $8000                 ; $82C35F
    lda.b $3C                   ; $82C362
    ora.l $7EFAD1,x             ; $82C364
    sta.w $8000                 ; $82C368
    lda.w $02EE                 ; $82C36B
    asl a                       ; $82C36E
    and.w #$00FE                ; $82C36F
    tax                         ; $82C372
    lda.l $86C89E,x             ; $82C373
    tax                         ; $82C377
    jsr.w sprite_parts_stream   ; $82C378
    jmp.w $C43C                 ; $82C37B
CODE_82C37E:
    ldx.b $14                   ; $82C37E
    lda.l $7E37EC,x             ; $82C380
    sta.w $8000                 ; $82C384
    lda.w #$9000                ; $82C387
    sta.w $8000                 ; $82C38A
    tyx                         ; $82C38D
    lda.l $7EFAB4,x             ; $82C38E
    sta.w $8000                 ; $82C392
    ldx.b $14                   ; $82C395
    lda.l $7E36EE,x             ; $82C397
    sta.w $8000                 ; $82C39B
    lda.l $7E36EC,x             ; $82C39E
    sta.w $8000                 ; $82C3A2
    lda.l $7E34EE,x             ; $82C3A5
    sta.w $8000                 ; $82C3A9
    lda.l $7E34EC,x             ; $82C3AC
    sta.w $8000                 ; $82C3B0
    tyx                         ; $82C3B3
    lda.l $7EFAB5,x             ; $82C3B4
    lsr a                       ; $82C3B8
    and.w #$007E                ; $82C3B9
    ldx.b $14                   ; $82C3BC
    clc                         ; $82C3BE
    adc.l $7E39EC,x             ; $82C3BF
    tax                         ; $82C3C3
    lda.l $83ACEC,x             ; $82C3C4
    sta.w $8000                 ; $82C3C8
    phx                         ; $82C3CB
    tyx                         ; $82C3CC
    lda.l $7EFABB,x             ; $82C3CD
    sta.w $8000                 ; $82C3D1
    nop                         ; $82C3D4
    lda.w $8000                 ; $82C3D5
    sta.b $2A                   ; $82C3D8
    lda.l $7EFABF,x             ; $82C3DA
    sta.w $8000                 ; $82C3DE
    lda.b $3C                   ; $82C3E1
    ora.l $7EFAD1,x             ; $82C3E3
    sta.w $8000                 ; $82C3E7
    ldx.b $14                   ; $82C3EA
    lda.l $7E34EC,x             ; $82C3EC
    sec                         ; $82C3F0
    sbc.l $7E34EE,x             ; $82C3F1
    asl a                       ; $82C3F5
    eor.w #$FFFF                ; $82C3F6
    sec                         ; $82C3F9
    adc.b $2A                   ; $82C3FA
    ldx.w #$0000                ; $82C3FC
    cmp.w #$FFA0                ; $82C3FF
    bmi CODE_82C429             ; $82C402
    inx                         ; $82C404
    inx                         ; $82C405
    cmp.w #$FFC0                ; $82C406
    bmi CODE_82C429             ; $82C409
    inx                         ; $82C40B
    inx                         ; $82C40C
    cmp.w #$FFE8                ; $82C40D
    bmi CODE_82C429             ; $82C410
    inx                         ; $82C412
    inx                         ; $82C413
    cmp.w #$0018                ; $82C414
    bmi CODE_82C429             ; $82C417
    inx                         ; $82C419
    inx                         ; $82C41A
    cmp.w #$0040                ; $82C41B
    bmi CODE_82C429             ; $82C41E
    inx                         ; $82C420
    inx                         ; $82C421
    cmp.w #$0060                ; $82C422
    bmi CODE_82C429             ; $82C425
    inx                         ; $82C427
    inx                         ; $82C428
CODE_82C429:
    lda.l $86AFD8,x             ; $82C429
    plx                         ; $82C42D
    clc                         ; $82C42E
    adc.l $849180,x             ; $82C42F
    tax                         ; $82C433
    lda.l $860000,x             ; $82C434
    tax                         ; $82C438
    jsr.w sprite_parts_stream   ; $82C439
    bcs CODE_82C46E             ; $82C43C
    inc.b $28                   ; $82C43E
    inc.b $28                   ; $82C440
    ldx.b $28                   ; $82C442
    lda.l $811E6A,x             ; $82C444
    tax                         ; $82C448
    lda.l $7EFAB6,x             ; $82C449
    cmp.b $2C                   ; $82C44D
    bne CODE_82C454             ; $82C44F
    jmp.w $C2D9                 ; $82C451
CODE_82C454:
    inc.b $2C                   ; $82C454
    lda.b $2C                   ; $82C456
    cmp.w $019A                 ; $82C458
    bne CODE_82C461             ; $82C45B
    stz.b $2C                   ; $82C45D
    stz.b $28                   ; $82C45F
CODE_82C461:
    inc.b $14                   ; $82C461
    inc.b $14                   ; $82C463
    lda.b $14                   ; $82C465
    cmp.b $92                   ; $82C467
    beq CODE_82C46E             ; $82C469
    jmp.w $C25A                 ; $82C46B
CODE_82C46E:
    stz.w $8000                 ; $82C46E
    lda.w #$8000                ; $82C471
    sta.w $8000                 ; $82C474
    lda.w $0192                 ; $82C477
    sec                         ; $82C47A
    sbc.w #$0040                ; $82C47B
    sta.w $0192                 ; $82C47E
    lda.w $0184                 ; $82C481
    sta.w $0186                 ; $82C484
    plb                         ; $82C487
    rts                         ; $82C488
vs_sprites_view2:
    pea.w $8330                 ; $82C489
    plb                         ; $82C48C
    lda.w $0188                 ; $82C48D
    clc                         ; $82C490
    adc.w #$0440                ; $82C491
    sta.w $0184                 ; $82C494
    lda.w #$000E                ; $82C497
    sta.w $8000                 ; $82C49A
    lda.b $8E                   ; $82C49D
    clc                         ; $82C49F
    adc.w #$0100                ; $82C4A0
    clc                         ; $82C4A3
    adc.w #$0440                ; $82C4A4
    sta.b $1A                   ; $82C4A7
    lda.w #$1000                ; $82C4A9
    sta.b $3C                   ; $82C4AC
CODE_82C4AE:
    lda.w $C000                 ; $82C4AE
    bpl CODE_82C4AE             ; $82C4B1
    lda.w #$0005                ; $82C4B3
    sta.w $8000                 ; $82C4B6
    ldy.w #$0008                ; $82C4B9
    lda.l $811E70               ; $82C4BC
    tax                         ; $82C4C0
    lda.w $0287                 ; $82C4C1
    cmp.l $7EFAB5,x             ; $82C4C4
    lda.w $0289                 ; $82C4C8
    sbc.l $7EFAB7,x             ; $82C4CB
    bcs CODE_82C4EE             ; $82C4CF
    ldy.w #$0006                ; $82C4D1
CODE_82C4D4:
    tyx                         ; $82C4D4
    lda.l $811E68,x             ; $82C4D5
    tax                         ; $82C4D9
    lda.w $0287                 ; $82C4DA
    cmp.l $7EFAB5,x             ; $82C4DD
    lda.w $0289                 ; $82C4E1
    sbc.l $7EFAB7,x             ; $82C4E4
    bcs CODE_82C4EE             ; $82C4E8
    dey                         ; $82C4EA
    dey                         ; $82C4EB
    bne CODE_82C4D4             ; $82C4EC
CODE_82C4EE:
    sty.b $28                   ; $82C4EE
    lda.w $0288                 ; $82C4F0
    sta.b $2C                   ; $82C4F3
CODE_82C4F5:
    lda.w $C000                 ; $82C4F5
    bpl CODE_82C4F5             ; $82C4F8
    lda.w #$0009                ; $82C4FA
    sta.w $8000                 ; $82C4FD
    lda.w #$003C                ; $82C500
    sta.w $8000                 ; $82C503
    lda.w #$00A9                ; $82C506
    sta.w $8000                 ; $82C509
    stz.w $8000                 ; $82C50C
    lda.w #$0000                ; $82C50F
    sta.w $8000                 ; $82C512
    lda.w #$0078                ; $82C515
    sta.w $8000                 ; $82C518
    lda.w #$0074                ; $82C51B
    sta.w $8000                 ; $82C51E
    lda.w #$00DC                ; $82C521
    sta.w $8000                 ; $82C524
    lda.w #$0080                ; $82C527
    sta.b $14                   ; $82C52A
    lda.w $0194                 ; $82C52C
    clc                         ; $82C52F
    adc.w #$0080                ; $82C530
    sta.w $0194                 ; $82C533
    lda.w $0288                 ; $82C536
    asl a                       ; $82C539
    asl a                       ; $82C53A
    sta.b $16                   ; $82C53B
    lda.w $019A                 ; $82C53D
    asl a                       ; $82C540
    asl a                       ; $82C541
    sta.b $18                   ; $82C542
    stz.w $1DFA                 ; $82C544
    stz.w $1DFC                 ; $82C547
    lda.l $7E356E               ; $82C54A
    sec                         ; $82C54E
    sbc.w $026E                 ; $82C54F
    sta.l $7E356C               ; $82C552
    lda.w #$006C                ; $82C556
    sta.l $7E386C               ; $82C559
    lda.w $026B                 ; $82C55D
    and.w #$00FF                ; $82C560
    bit.w #$0080                ; $82C563
    beq CODE_82C56B             ; $82C566
    incbin !baserom:$14568..$1456B
CODE_82C56B:
    eor.w #$FFFF                ; $82C56B
    sec                         ; $82C56E
    adc.l $7E376E               ; $82C56F
    sta.l $7E376C               ; $82C573
    lda.l $7E3A6E               ; $82C577
    sec                         ; $82C57B
    sbc.w #$0080                ; $82C57C
    sta.l $7E3A6C               ; $82C57F
    jmp.w $C5F3                 ; $82C583
    ldx.b $16                   ; $82C586
    txa                         ; $82C588
    clc                         ; $82C589
    adc.w #$0004                ; $82C58A
    cmp.b $18                   ; $82C58D
    bcc CODE_82C593             ; $82C58F
    sbc.b $18                   ; $82C591
CODE_82C593:
    sta.b $16                   ; $82C593
    lda.l $7E8CB3,x             ; $82C595
    beq CODE_82C5E6             ; $82C599
    stx.b $1C                   ; $82C59B
    xba                         ; $82C59D
    and.w #$007F                ; $82C59E
    asl a                       ; $82C5A1
    tax                         ; $82C5A2
    stx.b $1E                   ; $82C5A3
    lda.b $3C                   ; $82C5A5
    sta.b $3A                   ; $82C5A7
    ldx.b $14                   ; $82C5A9
    lda.l $7E39EC,x             ; $82C5AB
    cmp.w #$03C2                ; $82C5AF
    bcs CODE_82C5CD             ; $82C5B2
    ldx.b $1E                   ; $82C5B4
    lda.l $86AD66,x             ; $82C5B6
    tax                         ; $82C5BA
    lda.l $7E47A4,x             ; $82C5BB
    sta.b $26                   ; $82C5BF
    ldx.b $1E                   ; $82C5C1
    jsr.w ($7FFE,x)             ; $82C5C3
    bcs CODE_82C5CA             ; $82C5C6
    bra CODE_82C5F3             ; $82C5C8
CODE_82C5CA:
    jmp.w $C79B                 ; $82C5CA
CODE_82C5CD:
    ldx.b $1E                   ; $82C5CD
    lda.l $86AD66,x             ; $82C5CF
    tax                         ; $82C5D3
    lda.l $7E4B14,x             ; $82C5D4
    sta.b $26                   ; $82C5D8
    ldx.b $1E                   ; $82C5DA
    jsr.w ($7FFE,x)             ; $82C5DC
    bcs CODE_82C5CA             ; $82C5DF
    bra CODE_82C5F3             ; $82C5E1
    incbin !baserom:$145E3..$145E6
CODE_82C5E6:
    ldx.b $14                   ; $82C5E6
    lda.l $7E37EC,x             ; $82C5E8
    sta.w $8000                 ; $82C5EC
    nop                         ; $82C5EF
    stz.w $8000                 ; $82C5F0
CODE_82C5F3:
    ldx.b $28                   ; $82C5F3
    lda.l $811E6A,x             ; $82C5F5
    tax                         ; $82C5F9
    lda.l $7EFAB6,x             ; $82C5FA
    cmp.b $2C                   ; $82C5FE
    beq CODE_82C605             ; $82C600
CODE_82C602:
    jmp.w $C781                 ; $82C602
CODE_82C605:
    txy                         ; $82C605
    ldx.b $14                   ; $82C606
    inx                         ; $82C608
    inx                         ; $82C609
    cpx.b $94                   ; $82C60A
    beq CODE_82C602             ; $82C60C
    tyx                         ; $82C60E
    lda.l $7EFAB5,x             ; $82C60F
    lsr a                       ; $82C613
    and.w #$007E                ; $82C614
    ldx.b $14                   ; $82C617
    clc                         ; $82C619
    adc.l $7E39EC,x             ; $82C61A
    tax                         ; $82C61E
    lda.l $83ACEC,x             ; $82C61F
    sta.b $34                   ; $82C623
    lda.w $02A6                 ; $82C625
    bit.w #$0001                ; $82C628
    bne CODE_82C630             ; $82C62B
    cpy.w #$006A                ; $82C62D
CODE_82C630:
    bne CODE_82C6AA             ; $82C630
    lda.w $0293                 ; $82C632
    beq CODE_82C648             ; $82C635
    incbin !baserom:$14637..$14648
CODE_82C648:
    ldx.b $14                   ; $82C648
    lda.l $7E37EC,x             ; $82C64A
    sta.w $8000                 ; $82C64E
    lda.w #$9000                ; $82C651
    sta.w $8000                 ; $82C654
    tyx                         ; $82C657
    lda.l $7EFAB4,x             ; $82C658
    sta.w $8000                 ; $82C65C
    ldx.b $14                   ; $82C65F
    lda.w #$0033                ; $82C661
    sta.w $8000                 ; $82C664
    nop                         ; $82C667
    sta.w $8000                 ; $82C668
    lda.w $0264                 ; $82C66B
    sta.w $8000                 ; $82C66E
    nop                         ; $82C671
    sta.w $8000                 ; $82C672
    lda.b $34                   ; $82C675
    sta.w $8000                 ; $82C677
    tyx                         ; $82C67A
    lda.w $0264                 ; $82C67B
    sta.w $8000                 ; $82C67E
    nop                         ; $82C681
    lda.w $8000                 ; $82C682
    sta.b $2A                   ; $82C685
    lda.l $7EFABF,x             ; $82C687
    sta.w $8000                 ; $82C68B
    lda.b $3C                   ; $82C68E
    ora.l $7EFAD1,x             ; $82C690
    sta.w $8000                 ; $82C694
    lda.w $029D                 ; $82C697
    asl a                       ; $82C69A
    and.w #$00FE                ; $82C69B
    tax                         ; $82C69E
    lda.l $86C89E,x             ; $82C69F
    tax                         ; $82C6A3
    jsr.w sprite_parts_stream   ; $82C6A4
    jmp.w $C769                 ; $82C6A7
CODE_82C6AA:
    ldx.b $14                   ; $82C6AA
    lda.l $7E37EC,x             ; $82C6AC
    sta.w $8000                 ; $82C6B0
    lda.w #$9000                ; $82C6B3
    sta.w $8000                 ; $82C6B6
    tyx                         ; $82C6B9
    lda.l $7EFAB4,x             ; $82C6BA
    sta.w $8000                 ; $82C6BE
    ldx.b $14                   ; $82C6C1
    lda.l $7E36EE,x             ; $82C6C3
    sta.w $8000                 ; $82C6C7
    lda.l $7E36EC,x             ; $82C6CA
    sta.w $8000                 ; $82C6CE
    lda.l $7E34EE,x             ; $82C6D1
    sta.w $8000                 ; $82C6D5
    lda.l $7E34EC,x             ; $82C6D8
    sta.w $8000                 ; $82C6DC
    tyx                         ; $82C6DF
    lda.l $7EFAB5,x             ; $82C6E0
    lsr a                       ; $82C6E4
    and.w #$007E                ; $82C6E5
    ldx.b $14                   ; $82C6E8
    clc                         ; $82C6EA
    adc.l $7E39EC,x             ; $82C6EB
    tax                         ; $82C6EF
    lda.l $83ACEC,x             ; $82C6F0
    sta.w $8000                 ; $82C6F4
    phx                         ; $82C6F7
    tyx                         ; $82C6F8
    lda.l $7EFABB,x             ; $82C6F9
    sta.w $8000                 ; $82C6FD
    nop                         ; $82C700
    nop                         ; $82C701
    lda.w $8000                 ; $82C702
    sta.b $2A                   ; $82C705
    lda.l $7EFABF,x             ; $82C707
    sta.w $8000                 ; $82C70B
    lda.b $3C                   ; $82C70E
    ora.l $7EFAD1,x             ; $82C710
    sta.w $8000                 ; $82C714
    ldx.b $14                   ; $82C717
    lda.l $7E34EC,x             ; $82C719
    sec                         ; $82C71D
    sbc.l $7E34EE,x             ; $82C71E
    asl a                       ; $82C722
    eor.w #$FFFF                ; $82C723
    sec                         ; $82C726
    adc.b $2A                   ; $82C727
    ldx.w #$0000                ; $82C729
    cmp.w #$FFA0                ; $82C72C
    bmi CODE_82C756             ; $82C72F
    inx                         ; $82C731
    inx                         ; $82C732
    cmp.w #$FFC0                ; $82C733
    bmi CODE_82C756             ; $82C736
    inx                         ; $82C738
    inx                         ; $82C739
    cmp.w #$FFE8                ; $82C73A
    bmi CODE_82C756             ; $82C73D
    inx                         ; $82C73F
    inx                         ; $82C740
    cmp.w #$0018                ; $82C741
    bmi CODE_82C756             ; $82C744
    inx                         ; $82C746
    inx                         ; $82C747
    cmp.w #$0040                ; $82C748
    bmi CODE_82C756             ; $82C74B
    inx                         ; $82C74D
    inx                         ; $82C74E
    cmp.w #$0060                ; $82C74F
    bmi CODE_82C756             ; $82C752
    inx                         ; $82C754
    inx                         ; $82C755
CODE_82C756:
    lda.l $86AFD8,x             ; $82C756
    plx                         ; $82C75A
    clc                         ; $82C75B
    adc.l $849180,x             ; $82C75C
    tax                         ; $82C760
    lda.l $860000,x             ; $82C761
    tax                         ; $82C765
    jsr.w sprite_parts_stream   ; $82C766
    bcs CODE_82C79B             ; $82C769
    inc.b $28                   ; $82C76B
    inc.b $28                   ; $82C76D
    ldx.b $28                   ; $82C76F
    lda.l $811E6A,x             ; $82C771
    tax                         ; $82C775
    lda.l $7EFAB6,x             ; $82C776
    cmp.b $2C                   ; $82C77A
    bne CODE_82C781             ; $82C77C
    jmp.w $C605                 ; $82C77E
CODE_82C781:
    inc.b $2C                   ; $82C781
    lda.b $2C                   ; $82C783
    cmp.w $019A                 ; $82C785
    bne CODE_82C78E             ; $82C788
    stz.b $2C                   ; $82C78A
    stz.b $28                   ; $82C78C
CODE_82C78E:
    inc.b $14                   ; $82C78E
    inc.b $14                   ; $82C790
    lda.b $14                   ; $82C792
    cmp.b $94                   ; $82C794
    beq CODE_82C79B             ; $82C796
    jmp.w $C586                 ; $82C798
CODE_82C79B:
    stz.w $8000                 ; $82C79B
    lda.w #$8000                ; $82C79E
    sta.w $8000                 ; $82C7A1
    lda.w $0194                 ; $82C7A4
    sec                         ; $82C7A7
    sbc.w #$0080                ; $82C7A8
    sta.w $0194                 ; $82C7AB
    lda.w $0184                 ; $82C7AE
    sec                         ; $82C7B1
    sbc.w #$0440                ; $82C7B2
    sta.w $0188                 ; $82C7B5
    plb                         ; $82C7B8
    rts                         ; $82C7B9
vs_sprites_view3:
    pea.w $8330                 ; $82C7BA
    plb                         ; $82C7BD
    lda.w $0188                 ; $82C7BE
    clc                         ; $82C7C1
    adc.w #$0440                ; $82C7C2
    sta.w $0184                 ; $82C7C5
    lda.w #$000E                ; $82C7C8
    sta.w $8000                 ; $82C7CB
    lda.b $8E                   ; $82C7CE
    clc                         ; $82C7D0
    adc.w #$0200                ; $82C7D1
    clc                         ; $82C7D4
    adc.w #$0440                ; $82C7D5
    sta.b $1A                   ; $82C7D8
    lda.w #$1000                ; $82C7DA
    sta.b $3C                   ; $82C7DD
CODE_82C7DF:
    lda.w $C000                 ; $82C7DF
    bpl CODE_82C7DF             ; $82C7E2
    ldy.w #$0008                ; $82C7E4
    lda.l $811E70               ; $82C7E7
    tax                         ; $82C7EB
    lda.w $033A                 ; $82C7EC
    cmp.l $7EFAB5,x             ; $82C7EF
    lda.w $033C                 ; $82C7F3
    sbc.l $7EFAB7,x             ; $82C7F6
    bcs CODE_82C819             ; $82C7FA
    ldy.w #$0006                ; $82C7FC
CODE_82C7FF:
    tyx                         ; $82C7FF
    lda.l $811E68,x             ; $82C800
    tax                         ; $82C804
    lda.w $033A                 ; $82C805
    cmp.l $7EFAB5,x             ; $82C808
    lda.w $033C                 ; $82C80C
    sbc.l $7EFAB7,x             ; $82C80F
    bcs CODE_82C819             ; $82C813
    dey                         ; $82C815
    dey                         ; $82C816
    bne CODE_82C7FF             ; $82C817
CODE_82C819:
    sty.b $28                   ; $82C819
    lda.w $033B                 ; $82C81B
    sta.b $2C                   ; $82C81E
CODE_82C820:
    lda.w $C000                 ; $82C820
    bpl CODE_82C820             ; $82C823
    lda.w #$0009                ; $82C825
    sta.w $8000                 ; $82C828
    lda.w #$00C4                ; $82C82B
    sta.w $8000                 ; $82C82E
    lda.w #$00A9                ; $82C831
    sta.w $8000                 ; $82C834
    stz.w $8000                 ; $82C837
    lda.w #$0088                ; $82C83A
    sta.w $8000                 ; $82C83D
    lda.w #$0100                ; $82C840
    sta.w $8000                 ; $82C843
    lda.w #$0074                ; $82C846
    sta.w $8000                 ; $82C849
    lda.w #$00DC                ; $82C84C
    sta.w $8000                 ; $82C84F
    lda.w #$00C0                ; $82C852
    sta.b $14                   ; $82C855
    lda.w $0196                 ; $82C857
    clc                         ; $82C85A
    adc.w #$00C0                ; $82C85B
    sta.w $0196                 ; $82C85E
    lda.w $033B                 ; $82C861
    asl a                       ; $82C864
    asl a                       ; $82C865
    sta.b $16                   ; $82C866
    lda.w $019A                 ; $82C868
    asl a                       ; $82C86B
    asl a                       ; $82C86C
    sta.b $18                   ; $82C86D
    stz.w $1DFC                 ; $82C86F
    stz.w $1DFE                 ; $82C872
    lda.l $7E35AE               ; $82C875
    sec                         ; $82C879
    sbc.w $0321                 ; $82C87A
    sta.l $7E35AC               ; $82C87D
    lda.w #$006C                ; $82C881
    sta.l $7E38AC               ; $82C884
    lda.w $031E                 ; $82C888
    and.w #$00FF                ; $82C88B
    bit.w #$0080                ; $82C88E
    beq CODE_82C896             ; $82C891
    incbin !baserom:$14893..$14896
CODE_82C896:
    eor.w #$FFFF                ; $82C896
    sec                         ; $82C899
    adc.l $7E37AE               ; $82C89A
    sta.l $7E37AC               ; $82C89E
    lda.l $7E3AAE               ; $82C8A2
    sec                         ; $82C8A6
    sbc.w #$0080                ; $82C8A7
    sta.l $7E3AAC               ; $82C8AA
    jmp.w $C91E                 ; $82C8AE
    ldx.b $16                   ; $82C8B1
    txa                         ; $82C8B3
    clc                         ; $82C8B4
    adc.w #$0004                ; $82C8B5
    cmp.b $18                   ; $82C8B8
    bcc CODE_82C8BE             ; $82C8BA
    sbc.b $18                   ; $82C8BC
CODE_82C8BE:
    sta.b $16                   ; $82C8BE
    lda.l $7E8CB3,x             ; $82C8C0
    beq CODE_82C911             ; $82C8C4
    stx.b $1C                   ; $82C8C6
    xba                         ; $82C8C8
    and.w #$007F                ; $82C8C9
    asl a                       ; $82C8CC
    tax                         ; $82C8CD
    stx.b $1E                   ; $82C8CE
    lda.b $3C                   ; $82C8D0
    sta.b $3A                   ; $82C8D2
    ldx.b $14                   ; $82C8D4
    lda.l $7E39EC,x             ; $82C8D6
    cmp.w #$03C2                ; $82C8DA
    bcs CODE_82C8F8             ; $82C8DD
    ldx.b $1E                   ; $82C8DF
    lda.l $86AD66,x             ; $82C8E1
    tax                         ; $82C8E5
    lda.l $7E4880,x             ; $82C8E6
    sta.b $26                   ; $82C8EA
    ldx.b $1E                   ; $82C8EC
    jsr.w ($7FFE,x)             ; $82C8EE
    bcs CODE_82C8F5             ; $82C8F1
    bra CODE_82C91E             ; $82C8F3
CODE_82C8F5:
    jmp.w $CAC6                 ; $82C8F5
CODE_82C8F8:
    ldx.b $1E                   ; $82C8F8
    lda.l $86AD66,x             ; $82C8FA
    tax                         ; $82C8FE
    lda.l $7E4BF0,x             ; $82C8FF
    sta.b $26                   ; $82C903
    ldx.b $1E                   ; $82C905
    jsr.w ($7FFE,x)             ; $82C907
    bcs CODE_82C8F5             ; $82C90A
    bra CODE_82C91E             ; $82C90C
    incbin !baserom:$1490E..$14911
CODE_82C911:
    ldx.b $14                   ; $82C911
    lda.l $7E37EC,x             ; $82C913
    sta.w $8000                 ; $82C917
    nop                         ; $82C91A
    stz.w $8000                 ; $82C91B
CODE_82C91E:
    ldx.b $28                   ; $82C91E
    lda.l $811E6A,x             ; $82C920
    tax                         ; $82C924
    lda.l $7EFAB6,x             ; $82C925
    cmp.b $2C                   ; $82C929
    beq CODE_82C930             ; $82C92B
CODE_82C92D:
    jmp.w $CAAC                 ; $82C92D
CODE_82C930:
    txy                         ; $82C930
    ldx.b $14                   ; $82C931
    inx                         ; $82C933
    inx                         ; $82C934
    cpx.b $96                   ; $82C935
    beq CODE_82C92D             ; $82C937
    tyx                         ; $82C939
    lda.l $7EFAB5,x             ; $82C93A
    lsr a                       ; $82C93E
    and.w #$007E                ; $82C93F
    ldx.b $14                   ; $82C942
    clc                         ; $82C944
    adc.l $7E39EC,x             ; $82C945
    tax                         ; $82C949
    lda.l $83ACEC,x             ; $82C94A
    sta.b $34                   ; $82C94E
    lda.w $0359                 ; $82C950
    bit.w #$0001                ; $82C953
    bne CODE_82C95B             ; $82C956
    cpy.w #$009F                ; $82C958
CODE_82C95B:
    bne CODE_82C9D5             ; $82C95B
    lda.w $0346                 ; $82C95D
    beq CODE_82C973             ; $82C960
    incbin !baserom:$14962..$14973
CODE_82C973:
    ldx.b $14                   ; $82C973
    lda.l $7E37EC,x             ; $82C975
    sta.w $8000                 ; $82C979
    lda.w #$9000                ; $82C97C
    sta.w $8000                 ; $82C97F
    tyx                         ; $82C982
    lda.l $7EFAB4,x             ; $82C983
    sta.w $8000                 ; $82C987
    ldx.b $14                   ; $82C98A
    lda.w #$0033                ; $82C98C
    sta.w $8000                 ; $82C98F
    nop                         ; $82C992
    sta.w $8000                 ; $82C993
    lda.w $0317                 ; $82C996
    sta.w $8000                 ; $82C999
    nop                         ; $82C99C
    sta.w $8000                 ; $82C99D
    lda.b $34                   ; $82C9A0
    sta.w $8000                 ; $82C9A2
    tyx                         ; $82C9A5
    lda.w $0317                 ; $82C9A6
    sta.w $8000                 ; $82C9A9
    nop                         ; $82C9AC
    lda.w $8000                 ; $82C9AD
    sta.b $2A                   ; $82C9B0
    lda.l $7EFABF,x             ; $82C9B2
    sta.w $8000                 ; $82C9B6
    lda.b $3C                   ; $82C9B9
    ora.l $7EFAD1,x             ; $82C9BB
    sta.w $8000                 ; $82C9BF
    lda.w $0350                 ; $82C9C2
    asl a                       ; $82C9C5
    and.w #$00FE                ; $82C9C6
    tax                         ; $82C9C9
    lda.l $86C89E,x             ; $82C9CA
    tax                         ; $82C9CE
    jsr.w sprite_parts_stream   ; $82C9CF
    jmp.w $CA94                 ; $82C9D2
CODE_82C9D5:
    ldx.b $14                   ; $82C9D5
    lda.l $7E37EC,x             ; $82C9D7
    sta.w $8000                 ; $82C9DB
    lda.w #$9000                ; $82C9DE
    sta.w $8000                 ; $82C9E1
    tyx                         ; $82C9E4
    lda.l $7EFAB4,x             ; $82C9E5
    sta.w $8000                 ; $82C9E9
    ldx.b $14                   ; $82C9EC
    lda.l $7E36EE,x             ; $82C9EE
    sta.w $8000                 ; $82C9F2
    lda.l $7E36EC,x             ; $82C9F5
    sta.w $8000                 ; $82C9F9
    lda.l $7E34EE,x             ; $82C9FC
    sta.w $8000                 ; $82CA00
    lda.l $7E34EC,x             ; $82CA03
    sta.w $8000                 ; $82CA07
    tyx                         ; $82CA0A
    lda.l $7EFAB5,x             ; $82CA0B
    lsr a                       ; $82CA0F
    and.w #$007E                ; $82CA10
    ldx.b $14                   ; $82CA13
    clc                         ; $82CA15
    adc.l $7E39EC,x             ; $82CA16
    tax                         ; $82CA1A
    lda.l $83ACEC,x             ; $82CA1B
    sta.w $8000                 ; $82CA1F
    phx                         ; $82CA22
    tyx                         ; $82CA23
    lda.l $7EFABB,x             ; $82CA24
    sta.w $8000                 ; $82CA28
    nop                         ; $82CA2B
    nop                         ; $82CA2C
    lda.w $8000                 ; $82CA2D
    sta.b $2A                   ; $82CA30
    lda.l $7EFABF,x             ; $82CA32
    sta.w $8000                 ; $82CA36
    lda.b $3C                   ; $82CA39
    ora.l $7EFAD1,x             ; $82CA3B
    sta.w $8000                 ; $82CA3F
    ldx.b $14                   ; $82CA42
    lda.l $7E34EC,x             ; $82CA44
    sec                         ; $82CA48
    sbc.l $7E34EE,x             ; $82CA49
    asl a                       ; $82CA4D
    eor.w #$FFFF                ; $82CA4E
    sec                         ; $82CA51
    adc.b $2A                   ; $82CA52
    ldx.w #$0000                ; $82CA54
    cmp.w #$FFA0                ; $82CA57
    bmi CODE_82CA81             ; $82CA5A
    inx                         ; $82CA5C
    inx                         ; $82CA5D
    cmp.w #$FFC0                ; $82CA5E
    bmi CODE_82CA81             ; $82CA61
    inx                         ; $82CA63
    inx                         ; $82CA64
    cmp.w #$FFE8                ; $82CA65
    bmi CODE_82CA81             ; $82CA68
    inx                         ; $82CA6A
    inx                         ; $82CA6B
    cmp.w #$0018                ; $82CA6C
    bmi CODE_82CA81             ; $82CA6F
    inx                         ; $82CA71
    inx                         ; $82CA72
    cmp.w #$0040                ; $82CA73
    bmi CODE_82CA81             ; $82CA76
    inx                         ; $82CA78
    inx                         ; $82CA79
    cmp.w #$0060                ; $82CA7A
    bmi CODE_82CA81             ; $82CA7D
    inx                         ; $82CA7F
    inx                         ; $82CA80
CODE_82CA81:
    lda.l $86AFD8,x             ; $82CA81
    plx                         ; $82CA85
    clc                         ; $82CA86
    adc.l $849180,x             ; $82CA87
    tax                         ; $82CA8B
    lda.l $860000,x             ; $82CA8C
    tax                         ; $82CA90
    jsr.w sprite_parts_stream   ; $82CA91
    bcs CODE_82CAC6             ; $82CA94
    inc.b $28                   ; $82CA96
    inc.b $28                   ; $82CA98
    ldx.b $28                   ; $82CA9A
    lda.l $811E6A,x             ; $82CA9C
    tax                         ; $82CAA0
    lda.l $7EFAB6,x             ; $82CAA1
    cmp.b $2C                   ; $82CAA5
    bne CODE_82CAAC             ; $82CAA7
    jmp.w $C930                 ; $82CAA9
CODE_82CAAC:
    inc.b $2C                   ; $82CAAC
    lda.b $2C                   ; $82CAAE
    cmp.w $019A                 ; $82CAB0
    bne CODE_82CAB9             ; $82CAB3
    stz.b $2C                   ; $82CAB5
    stz.b $28                   ; $82CAB7
CODE_82CAB9:
    inc.b $14                   ; $82CAB9
    inc.b $14                   ; $82CABB
    lda.b $14                   ; $82CABD
    cmp.b $96                   ; $82CABF
    beq CODE_82CAC6             ; $82CAC1
    jmp.w $C8B1                 ; $82CAC3
CODE_82CAC6:
    stz.w $8000                 ; $82CAC6
    lda.w #$8000                ; $82CAC9
    sta.w $8000                 ; $82CACC
    lda.w $0196                 ; $82CACF
    sec                         ; $82CAD2
    sbc.w #$00C0                ; $82CAD3
    sta.w $0196                 ; $82CAD6
    lda.w $0184                 ; $82CAD9
    sec                         ; $82CADC
    sbc.w #$0440                ; $82CADD
    sta.w $0188                 ; $82CAE0
    plb                         ; $82CAE3
    rts                         ; $82CAE4
sprite_put_rows_c:
    ldx.b $14                   ; $82CAE5
    lda.l $7E37EC,x             ; $82CAE7
    sta.w $8000                 ; $82CAEB
    lda.l $7E38EC,x             ; $82CAEE
    sta.w $8000                 ; $82CAF2
    lda.l $7E35EC,x             ; $82CAF5
    sta.w $8000                 ; $82CAF9
    lda.l $7E37EC,x             ; $82CAFC
    sta.w $8000                 ; $82CB00
    lda.b $20                   ; $82CB03
    sta.w $8000                 ; $82CB05
    lda.b $22                   ; $82CB08
    sta.w $8000                 ; $82CB0A
    lda.b $26                   ; $82CB0D
    ora.b $3A                   ; $82CB0F
    sta.w $8000                 ; $82CB11
    ldx.b $24                   ; $82CB14
    lda.l $860000,x             ; $82CB16
    tax                         ; $82CB1A
    lda.l $7E6591,x             ; $82CB1B
    pha                         ; $82CB1F
    ldx.b $14                   ; $82CB20
    lda.l $7E39EC,x             ; $82CB22
    tax                         ; $82CB26
    lda.l $848080,x             ; $82CB27
    clc                         ; $82CB2B
    adc.b $24                   ; $82CB2C
    tax                         ; $82CB2E
    pla                         ; $82CB2F
    clc                         ; $82CB30
    adc.l $860002,x             ; $82CB31
    tax                         ; $82CB35
    jmp.w sprite_parts_stream   ; $82CB36
    incbin !baserom:$14B39..$14BD4
object_sprites_stream:
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
    inx                         ; $82CC59
    inx                         ; $82CC5A
CODE_82CC5B:
    lda.w $0235                 ; $82CC5B
    sta.b $14                   ; $82CC5E
    lda.w #$001E                ; $82CC60
    jsr.w minimap_car           ; $82CC63
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
    inx                         ; $82CC7F static
    inx                         ; $82CC80 static
CODE_82CC81:
    lda.w $02E8                 ; $82CC81
    sta.b $14                   ; $82CC84
    lda.w #$001E                ; $82CC86
    jsr.w minimap_car           ; $82CC89
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
    clc                         ; $82CCB8 static
    adc.w #$00DC                ; $82CCB9 static
    bra CODE_82CCC1             ; $82CCBC static
CODE_82CCBE:
    lda.w #$00DC                ; $82CCBE
CODE_82CCC1:
    sta.w $8000                 ; $82CCC1
    jmp.w $CDB7                 ; $82CCC4
mode1_view1_sprites:
    pea.w $8330                 ; $82CCC7
    plb                         ; $82CCCA
    lda.w $0186                 ; $82CCCB
    sta.w $0184                 ; $82CCCE
    lda.w #$0003                ; $82CCD1
    sta.w $8000                 ; $82CCD4
    lda.b $8E                   ; $82CCD7
    clc                         ; $82CCD9
    adc.w #$0200                ; $82CCDA
    sta.b $1A                   ; $82CCDD
    stz.b $3C                   ; $82CCDF
    lda.b $C8                   ; $82CCE1
    bne CODE_82CCEA             ; $82CCE3
    lda.w #$1000                ; $82CCE5
    sta.b $3C                   ; $82CCE8
CODE_82CCEA:
    lda.w $C000                 ; $82CCEA
    bpl CODE_82CCEA             ; $82CCED
    lda.w #$0005                ; $82CCEF
    sta.w $8000                 ; $82CCF2
    ldy.w #$0028                ; $82CCF5
    lda.l $811E90               ; $82CCF8
    tax                         ; $82CCFC
    lda.w $0225                 ; $82CCFD
    cmp.l $7EFAB5,x             ; $82CD00
    lda.w $0227                 ; $82CD04
    sbc.l $7EFAB7,x             ; $82CD07
    bcs CODE_82CD2A             ; $82CD0B
    ldy.w #$0026                ; $82CD0D
CODE_82CD10:
    tyx                         ; $82CD10
    lda.l $811E68,x             ; $82CD11
    tax                         ; $82CD15
    lda.w $0225                 ; $82CD16
    cmp.l $7EFAB5,x             ; $82CD19
    lda.w $0227                 ; $82CD1D
    sbc.l $7EFAB7,x             ; $82CD20
    bcs CODE_82CD2A             ; $82CD24
    dey                         ; $82CD26
    dey                         ; $82CD27
    bne CODE_82CD10             ; $82CD28
CODE_82CD2A:
    sty.b $28                   ; $82CD2A
    lda.w $0226                 ; $82CD2C
    sta.b $2C                   ; $82CD2F
CODE_82CD31:
    lda.w $C000                 ; $82CD31
    bpl CODE_82CD31             ; $82CD34
    lda.l $7EFAD1               ; $82CD36
    ora.w #$30FD                ; $82CD3A
    tay                         ; $82CD3D
    lda.w $0226                 ; $82CD3E
    lsr a                       ; $82CD41
    lsr a                       ; $82CD42
    and.w #$FFFC                ; $82CD43
    tax                         ; $82CD46
    lda.w $1E66                 ; $82CD47
    bpl CODE_82CD4E             ; $82CD4A
    inx                         ; $82CD4C
    inx                         ; $82CD4D
CODE_82CD4E:
    lda.w $0235                 ; $82CD4E
    sta.b $14                   ; $82CD51
    lda.w #$001E                ; $82CD53
    jsr.w minimap_car           ; $82CD56
    ldx.w #$0035                ; $82CD59
    lda.l $7EFAD1,x             ; $82CD5C
    ora.w #$30FD                ; $82CD60
    tay                         ; $82CD63
    lda.w $02D9                 ; $82CD64
    lsr a                       ; $82CD67
    lsr a                       ; $82CD68
    and.w #$FFFC                ; $82CD69
    tax                         ; $82CD6C
    lda.w $1E68                 ; $82CD6D
    bpl CODE_82CD74             ; $82CD70
    inx                         ; $82CD72
    inx                         ; $82CD73
CODE_82CD74:
    lda.w $02E8                 ; $82CD74
    sta.b $14                   ; $82CD77
    lda.w #$001E                ; $82CD79
    jsr.w minimap_car           ; $82CD7C
    lda.w #$0009                ; $82CD7F
    sta.w $8000                 ; $82CD82
    lda.w #$0080                ; $82CD85
    sta.w $8000                 ; $82CD88
    lda.w #$0039                ; $82CD8B
    sta.w $8000                 ; $82CD8E
    stz.w $8000                 ; $82CD91
    lda.w #$0000                ; $82CD94
    sta.w $8000                 ; $82CD97
    lda.w #$0100                ; $82CD9A
    sta.w $8000                 ; $82CD9D
    lda.w #$0004                ; $82CDA0
    sta.w $8000                 ; $82CDA3
    lda.w $1EF2                 ; $82CDA6
    bpl CODE_82CDB1             ; $82CDA9
    incbin !baserom:$14DAB..$14DB1
CODE_82CDB1:
    lda.w #$006C                ; $82CDB1
    sta.w $8000                 ; $82CDB4
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
    lda.w #$0000                ; $82CDED
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
    ora.w #$FF00                ; $82CE0F static
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
    sta.b $16                   ; $82CE5F
    lda.w $1EE0                 ; $82CE61
    clc                         ; $82CE64
    adc.b $2C                   ; $82CE65
    sta.b $2C                   ; $82CE67
    lda.w $1EE0                 ; $82CE69
    asl a                       ; $82CE6C
    asl a                       ; $82CE6D
    adc.b $16                   ; $82CE6E
CODE_82CE70:
    cmp.b $2E                   ; $82CE70
    bne CODE_82CE82             ; $82CE72
    clc                         ; $82CE74
    adc.b $36                   ; $82CE75
    pha                         ; $82CE77
    lda.b $36                   ; $82CE78
    lsr a                       ; $82CE7A
    lsr a                       ; $82CE7B
    clc                         ; $82CE7C
    adc.b $2C                   ; $82CE7D
    sta.b $2C                   ; $82CE7F
    pla                         ; $82CE81
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
    bcs CODE_82CEBE             ; $82CEBA
    bra CODE_82CEFD             ; $82CEBC
CODE_82CEBE:
    jmp.w $D1E7                 ; $82CEBE
CODE_82CEC1:
    ldx.b $1E                   ; $82CEC1
    lda.l $86AD66,x             ; $82CEC3
    tax                         ; $82CEC7
    lda.l $7E495C,x             ; $82CEC8
    sta.b $26                   ; $82CECC
    ldx.b $1E                   ; $82CECE
    jsr.w ($80CE,x)             ; $82CED0
    bcs CODE_82CEBE             ; $82CED3
    bra CODE_82CEFD             ; $82CED5
    jmp.w $D1E7                 ; $82CED7 static
CODE_82CEDA:
    tax                         ; $82CEDA
    lda.l $7E45EC               ; $82CEDB
    sta.b $26                   ; $82CEDF
    lda.b $1E                   ; $82CEE1
    sta.w $1DF6                 ; $82CEE3
    tax                         ; $82CEE6
    jsr.w ($80CE,x)             ; $82CEE7
    bcs CODE_82CEBE             ; $82CEEA
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
    phy                         ; $82CF3D
    phx                         ; $82CF3E
    lda.w $1F1C                 ; $82CF3F
    sta.b $32                   ; $82CF42
    lda.w $1F20                 ; $82CF44
    sta.b $34                   ; $82CF47
    jsr.w car_sprite_head       ; $82CF49
    lda.l $7EFABF,x             ; $82CF4C
    sta.w $8000                 ; $82CF50
    lda.b $3C                   ; $82CF53
    clc                         ; $82CF55
    adc.w $1F34                 ; $82CF56
    clc                         ; $82CF59
    adc.w #$0138                ; $82CF5A
    sta.w $8000                 ; $82CF5D
    lda.w $1F18                 ; $82CF60
    and.w #$FFF8                ; $82CF63
    sta.w $1F24                 ; $82CF66
    tax                         ; $82CF69
    lda.l $86C8B2,x             ; $82CF6A
    tax                         ; $82CF6E
    jsr.w sprite_parts_stream_rom; $82CF6F
    plx                         ; $82CF72
    ply                         ; $82CF73
CODE_82CF74:
    lda.w $021D                 ; $82CF74
    bpl CODE_82CFA5             ; $82CF77
    phy                         ; $82CF79
    phx                         ; $82CF7A
    stz.b $32                   ; $82CF7B
    stz.b $34                   ; $82CF7D
    jsr.w car_sprite_head       ; $82CF7F
    lda.l $7EFABF,x             ; $82CF82
    sta.w $8000                 ; $82CF86
    lda.b $3C                   ; $82CF89
    clc                         ; $82CF8B
    adc.w #$0F00                ; $82CF8C
    sta.w $8000                 ; $82CF8F
    lda.w $023A                 ; $82CF92
    xba                         ; $82CF95
    and.w #$00FF                ; $82CF96
    asl a                       ; $82CF99
    tax                         ; $82CF9A
    lda.l $86CBEC,x             ; $82CF9B
    tax                         ; $82CF9F
    jsr.w sprite_parts_stream_rom; $82CFA0
    plx                         ; $82CFA3
    ply                         ; $82CFA4
CODE_82CFA5:
    phy                         ; $82CFA5
    phx                         ; $82CFA6
    stz.b $32                   ; $82CFA7
    stz.b $34                   ; $82CFA9
    jsr.w car_sprite_head       ; $82CFAB
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
    jsr.w sprite_parts_stream_rom; $82CFCE
    plx                         ; $82CFD1
    ply                         ; $82CFD2
    jsr.w car_sprite_head       ; $82CFD3
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
    jsl.l sprite_bar_put        ; $82CFFF
    lda.w $014C                 ; $82D003
    cmp.w #$0002                ; $82D006
    bne CODE_82D058             ; $82D009
    ldx.b $14                   ; $82D00B
    lda.l $7E386A,x             ; $82D00D
    sta.w $8000                 ; $82D011
    lda.w #$9000                ; $82D014
    sta.w $8000                 ; $82D017
    lda.w #$0000                ; $82D01A
    sta.w $8000                 ; $82D01D
    lda.w #$0033                ; $82D020
    sta.w $8000                 ; $82D023
    sta.w $8000                 ; $82D026
    lda.w #$0000                ; $82D029
    sta.w $8000                 ; $82D02C
    lda.w #$0000                ; $82D02F
    sta.w $8000                 ; $82D032
    lda.w #$7FFF                ; $82D035
    sta.w $8000                 ; $82D038
    lda.w $028B                 ; $82D03B
    sta.w $8000                 ; $82D03E
    nop                         ; $82D041
    nop                         ; $82D042
    lda.w $8000                 ; $82D043
    nop                         ; $82D046
    lda.w $028D                 ; $82D047
    sta.w $8000                 ; $82D04A
    lda.b $3C                   ; $82D04D
    sta.w $8000                 ; $82D04F
    ldx.w #$AD4A                ; $82D052
    jsr.w sprite_parts_stream_rom; $82D055
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
    ldx.b $14                   ; $82D0A9
    lda.l $7E376E,x             ; $82D0AB
    sta.w $8000                 ; $82D0AF
    lda.l $7E376C,x             ; $82D0B2
    sta.w $8000                 ; $82D0B6
    lda.l $7E356E,x             ; $82D0B9
    sta.w $8000                 ; $82D0BD
    lda.l $7E356C,x             ; $82D0C0
    sta.w $8000                 ; $82D0C4
    tyx                         ; $82D0C7
    lda.l $7EFAB5,x             ; $82D0C8
    lsr a                       ; $82D0CC
    and.w #$007E                ; $82D0CD
    ldx.b $14                   ; $82D0D0
    clc                         ; $82D0D2
    adc.l $7E39EC,x             ; $82D0D3
    tax                         ; $82D0D7
    lda.l $83ACEC,x             ; $82D0D8
    sta.w $8000                 ; $82D0DC
    phx                         ; $82D0DF
    tyx                         ; $82D0E0
    lda.l $7EFABB,x             ; $82D0E1
    sta.w $8000                 ; $82D0E5
    nop                         ; $82D0E8
    nop                         ; $82D0E9
    lda.w $8000                 ; $82D0EA
    sta.b $2A                   ; $82D0ED
    lda.l $7EFABF,x             ; $82D0EF
    sta.w $8000                 ; $82D0F3
    lda.w #$1000                ; $82D0F6
    ora.l $7EFAD1,x             ; $82D0F9
    sta.w $8000                 ; $82D0FD
    ldx.b $14                   ; $82D100
    lda.l $7E356C,x             ; $82D102
    sec                         ; $82D106
    sbc.l $7E356E,x             ; $82D107
    bra CODE_82D16E             ; $82D10B
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
CODE_82D16E:
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
    inx                         ; $82D199
    inx                         ; $82D19A
    cmp.w #$0060                ; $82D19B
    bmi CODE_82D1A2             ; $82D19E
    inx                         ; $82D1A0
    inx                         ; $82D1A1
CODE_82D1A2:
    lda.l $7F0000,x             ; $82D1A2
    plx                         ; $82D1A6
    clc                         ; $82D1A7
    adc.l $849180,x             ; $82D1A8
    tax                         ; $82D1AC
    lda.l $7F0000,x             ; $82D1AD
    tax                         ; $82D1B1
    jsr.w sprite_parts_stream_rom; $82D1B2
    bcs CODE_82D1E7             ; $82D1B5
    inc.b $28                   ; $82D1B7
    inc.b $28                   ; $82D1B9
    ldx.b $28                   ; $82D1BB
    lda.l $811E6A,x             ; $82D1BD
    tax                         ; $82D1C1
    lda.l $7EFAB6,x             ; $82D1C2
    cmp.b $2C                   ; $82D1C6
    bne CODE_82D1CD             ; $82D1C8
    jmp.w $CF1A                 ; $82D1CA
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
car_sprite_head:
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
    ora.w #$FF00                ; $82D289 static
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
mode1_view2_sprites:
    pea.w $8330                 ; $82D2CC
    plb                         ; $82D2CF
    lda.w $0188                 ; $82D2D0
    clc                         ; $82D2D3
    adc.w #$0440                ; $82D2D4
    sta.w $0184                 ; $82D2D7
    lda.w #$0003                ; $82D2DA
    sta.w $8000                 ; $82D2DD
    lda.b $8E                   ; $82D2E0
    clc                         ; $82D2E2
    adc.w #$0200                ; $82D2E3
    clc                         ; $82D2E6
    adc.w #$0440                ; $82D2E7
    sta.b $1A                   ; $82D2EA
    stz.b $3C                   ; $82D2EC
    lda.b $CA                   ; $82D2EE
    bne CODE_82D2F7             ; $82D2F0
    lda.w #$1000                ; $82D2F2
    sta.b $3C                   ; $82D2F5
CODE_82D2F7:
    lda.w $C000                 ; $82D2F7
    bpl CODE_82D2F7             ; $82D2FA
    lda.w #$0005                ; $82D2FC
    sta.w $8000                 ; $82D2FF
    ldy.w #$0028                ; $82D302
    lda.l $811E90               ; $82D305
    tax                         ; $82D309
    lda.w $02D8                 ; $82D30A
    cmp.l $7EFAB5,x             ; $82D30D
    lda.w $02DA                 ; $82D311
    sbc.l $7EFAB7,x             ; $82D314
    bcs CODE_82D337             ; $82D318
    ldy.w #$0026                ; $82D31A
CODE_82D31D:
    tyx                         ; $82D31D
    lda.l $811E68,x             ; $82D31E
    tax                         ; $82D322
    lda.w $02D8                 ; $82D323
    cmp.l $7EFAB5,x             ; $82D326
    lda.w $02DA                 ; $82D32A
    sbc.l $7EFAB7,x             ; $82D32D
    bcs CODE_82D337             ; $82D331
    dey                         ; $82D333
    dey                         ; $82D334
    bne CODE_82D31D             ; $82D335
CODE_82D337:
    sty.b $28                   ; $82D337
    lda.w $02D9                 ; $82D339
    sta.b $2C                   ; $82D33C
CODE_82D33E:
    lda.w $C000                 ; $82D33E
    bpl CODE_82D33E             ; $82D341
    ldx.w #$0035                ; $82D343
    lda.l $7EFAD1,x             ; $82D346
    ora.w #$30FD                ; $82D34A
    tay                         ; $82D34D
    lda.w $02D9                 ; $82D34E
    lsr a                       ; $82D351
    lsr a                       ; $82D352
    and.w #$FFFC                ; $82D353
    tax                         ; $82D356
    lda.w $1E68                 ; $82D357
    bpl CODE_82D35E             ; $82D35A
    inx                         ; $82D35C
    inx                         ; $82D35D
CODE_82D35E:
    lda.w $0235                 ; $82D35E
    sta.b $14                   ; $82D361
    lda.w #$008E                ; $82D363
    jsr.w minimap_car           ; $82D366
    lda.l $7EFAD1               ; $82D369
    ora.w #$30FD                ; $82D36D
    tay                         ; $82D370
    lda.w $0226                 ; $82D371
    lsr a                       ; $82D374
    lsr a                       ; $82D375
    and.w #$FFFC                ; $82D376
    tax                         ; $82D379
    lda.w $1E66                 ; $82D37A
    bpl CODE_82D381             ; $82D37D
    inx                         ; $82D37F
    inx                         ; $82D380
CODE_82D381:
    lda.w $02E8                 ; $82D381
    sta.b $14                   ; $82D384
    lda.w #$008E                ; $82D386
    jsr.w minimap_car           ; $82D389
    lda.w #$0009                ; $82D38C
    sta.w $8000                 ; $82D38F
    lda.w #$0080                ; $82D392
    sta.w $8000                 ; $82D395
    lda.w #$00A9                ; $82D398
    sta.w $8000                 ; $82D39B
    stz.w $8000                 ; $82D39E
    lda.w #$0000                ; $82D3A1
    sta.w $8000                 ; $82D3A4
    lda.w #$0100                ; $82D3A7
    sta.w $8000                 ; $82D3AA
    lda.w #$0074                ; $82D3AD
    sta.w $8000                 ; $82D3B0
    lda.w $1EFC                 ; $82D3B3
    bpl CODE_82D3BE             ; $82D3B6
    incbin !baserom:$153B8..$153BE
CODE_82D3BE:
    lda.w #$00DC                ; $82D3BE
    sta.w $8000                 ; $82D3C1
    lda.w #$0040                ; $82D3C4
    sta.b $14                   ; $82D3C7
    lda.b $92                   ; $82D3C9
    clc                         ; $82D3CB
    adc.w #$0040                ; $82D3CC
    sta.b $92                   ; $82D3CF
    lda.w $02D9                 ; $82D3D1
    asl a                       ; $82D3D4
    asl a                       ; $82D3D5
    sta.b $16                   ; $82D3D6
    lda.w $019A                 ; $82D3D8
    asl a                       ; $82D3DB
    asl a                       ; $82D3DC
    sta.b $18                   ; $82D3DD
    lda.w $1EE2                 ; $82D3DF
    asl a                       ; $82D3E2
    asl a                       ; $82D3E3
    sta.b $30                   ; $82D3E4
    lda.w $019C                 ; $82D3E6
    clc                         ; $82D3E9
    adc.w $1E62                 ; $82D3EA
    tax                         ; $82D3ED
    lda.l $840000,x             ; $82D3EE
    clc                         ; $82D3F2
    adc.w #$003F                ; $82D3F3
    asl a                       ; $82D3F6
    asl a                       ; $82D3F7
    sta.b $2E                   ; $82D3F8
    lda.l $840010,x             ; $82D3FA
    asl a                       ; $82D3FE
    asl a                       ; $82D3FF
    bit.w $1E68                 ; $82D400
    bpl CODE_82D408             ; $82D403
    lda.w #$0000                ; $82D405
CODE_82D408:
    sta.b $36                   ; $82D408
    stz.w $1DF8                 ; $82D40A
    stz.w $1DFA                 ; $82D40D
    lda.l $7E352E               ; $82D410
    sec                         ; $82D414
    sbc.w $02BF                 ; $82D415
    sta.l $7E352C               ; $82D418
    lda.l $7E372E               ; $82D41C
    sec                         ; $82D420
    sbc.w $02BB                 ; $82D421
    sta.l $7E372C               ; $82D424
    lda.w #$0033                ; $82D428
    sta.l $7E38AC               ; $82D42B
    lda.l $7E3A2E               ; $82D42F
    sec                         ; $82D433
    sbc.w #$0080                ; $82D434
    sta.l $7E3A2C               ; $82D437
    jmp.w $D4EB                 ; $82D43B
    ldx.b $16                   ; $82D43E
    txa                         ; $82D440
    clc                         ; $82D441
    adc.w #$0004                ; $82D442
    cmp.b $18                   ; $82D445
    bcc CODE_82D44B             ; $82D447
    sbc.b $18                   ; $82D449
CODE_82D44B:
    cmp.b $30                   ; $82D44B
    bne CODE_82D460             ; $82D44D
    sta.b $16                   ; $82D44F
    lda.w $1EE4                 ; $82D451
    clc                         ; $82D454
    adc.b $2C                   ; $82D455
    sta.b $2C                   ; $82D457
    lda.w $1EE4                 ; $82D459
    asl a                       ; $82D45C
    asl a                       ; $82D45D
    adc.b $16                   ; $82D45E
CODE_82D460:
    cmp.b $2E                   ; $82D460
    bne CODE_82D472             ; $82D462
    clc                         ; $82D464
    adc.b $36                   ; $82D465
    pha                         ; $82D467
    lda.b $36                   ; $82D468
    lsr a                       ; $82D46A
    lsr a                       ; $82D46B
    clc                         ; $82D46C
    adc.b $2C                   ; $82D46D
    sta.b $2C                   ; $82D46F
    pla                         ; $82D471
CODE_82D472:
    sta.b $16                   ; $82D472
    lda.l $7E8CB3,x             ; $82D474
    beq CODE_82D4DE             ; $82D478
    stx.b $1C                   ; $82D47A
    xba                         ; $82D47C
    and.w #$007F                ; $82D47D
    asl a                       ; $82D480
    tax                         ; $82D481
    stx.b $1E                   ; $82D482
    lda.b $3C                   ; $82D484
    sta.b $3A                   ; $82D486
    ldx.b $14                   ; $82D488
    lda.l $7E39EC,x             ; $82D48A
    cmp.w #$0082                ; $82D48E
    bcc CODE_82D4CA             ; $82D491
    cmp.w #$0442                ; $82D493
    bcs CODE_82D4B1             ; $82D496
    ldx.b $1E                   ; $82D498
    lda.l $86AD66,x             ; $82D49A
    tax                         ; $82D49E
    lda.l $7E46C8,x             ; $82D49F
    sta.b $26                   ; $82D4A3
    ldx.b $1E                   ; $82D4A5
    jsr.w ($80CE,x)             ; $82D4A7
    bcs CODE_82D4AE             ; $82D4AA
    bra CODE_82D4EB             ; $82D4AC
CODE_82D4AE:
    jmp.w $D7D3                 ; $82D4AE
CODE_82D4B1:
    ldx.b $1E                   ; $82D4B1
    lda.l $86AD66,x             ; $82D4B3
    tax                         ; $82D4B7
    lda.l $7E4A38,x             ; $82D4B8
    sta.b $26                   ; $82D4BC
    ldx.b $1E                   ; $82D4BE
    jsr.w ($80CE,x)             ; $82D4C0
    bcs CODE_82D4AE             ; $82D4C3
    bra CODE_82D4EB             ; $82D4C5
    incbin !baserom:$154C7..$154CA
CODE_82D4CA:
    tax                         ; $82D4CA
    lda.l $7E46C8               ; $82D4CB
    sta.b $26                   ; $82D4CF
    lda.b $1E                   ; $82D4D1
    sta.w $1DF8                 ; $82D4D3
    tax                         ; $82D4D6
    jsr.w ($80CE,x)             ; $82D4D7
    bcs CODE_82D4AE             ; $82D4DA
    bra CODE_82D4EB             ; $82D4DC
CODE_82D4DE:
    ldx.b $14                   ; $82D4DE
    lda.l $7E386C,x             ; $82D4E0
    sta.w $8000                 ; $82D4E4
    nop                         ; $82D4E7
    stz.w $8000                 ; $82D4E8
CODE_82D4EB:
    ldx.b $28                   ; $82D4EB
    lda.l $811E6A,x             ; $82D4ED
    tax                         ; $82D4F1
    lda.l $7EFAB6,x             ; $82D4F2
    cmp.b $2C                   ; $82D4F6
    beq CODE_82D508             ; $82D4F8
CODE_82D4FA:
    bmi CODE_82D4FF             ; $82D4FA
    jmp.w $D7B9                 ; $82D4FC
CODE_82D4FF:
    inc.b $28                   ; $82D4FF
    inc.b $28                   ; $82D501
    bra CODE_82D4EB             ; $82D503
CODE_82D505:
    jmp.w $D64E                 ; $82D505
CODE_82D508:
    txy                         ; $82D508
    ldx.b $14                   ; $82D509
    inx                         ; $82D50B
    inx                         ; $82D50C
    cpx.b $92                   ; $82D50D
    beq CODE_82D4FA             ; $82D50F
    cpy.w #$0035                ; $82D511
    bne CODE_82D505             ; $82D514
    lda.w $02F7                 ; $82D516
    bit.w #$0001                ; $82D519
    bne CODE_82D505             ; $82D51C
    lda.w $02CD                 ; $82D51E
    bne CODE_82D562             ; $82D521
    lda.w $1F1A                 ; $82D523
    sta.w $1F26                 ; $82D526
    bmi CODE_82D562             ; $82D529
    incbin !baserom:$1552B..$15562
CODE_82D562:
    lda.w $02D0                 ; $82D562
    bpl CODE_82D593             ; $82D565
    phy                         ; $82D567
    phx                         ; $82D568
    stz.b $32                   ; $82D569
    stz.b $34                   ; $82D56B
    jsr.w car_sprite_head2      ; $82D56D
    lda.l $7EFABF,x             ; $82D570
    sta.w $8000                 ; $82D574
    lda.b $3C                   ; $82D577
    clc                         ; $82D579
    adc.w #$0F00                ; $82D57A
    sta.w $8000                 ; $82D57D
    lda.w $02ED                 ; $82D580
    xba                         ; $82D583
    and.w #$00FF                ; $82D584
    asl a                       ; $82D587
    tax                         ; $82D588
    lda.l $86CBEC,x             ; $82D589
    tax                         ; $82D58D
    jsr.w sprite_parts_stream_rom; $82D58E
    plx                         ; $82D591
    ply                         ; $82D592
CODE_82D593:
    phy                         ; $82D593
    phx                         ; $82D594
    stz.b $32                   ; $82D595
    stz.b $34                   ; $82D597
    jsr.w car_sprite_head2      ; $82D599
    lda.l $7EFABF,x             ; $82D59C
    sta.w $8000                 ; $82D5A0
    lda.b $3C                   ; $82D5A3
    clc                         ; $82D5A5
    adc.w #$0F00                ; $82D5A6
    sta.w $8000                 ; $82D5A9
    lda.w $02ED                 ; $82D5AC
    xba                         ; $82D5AF
    and.w #$00FF                ; $82D5B0
    asl a                       ; $82D5B3
    asl a                       ; $82D5B4
    asl a                       ; $82D5B5
    tax                         ; $82D5B6
    lda.l $86C97A,x             ; $82D5B7
    tax                         ; $82D5BB
    jsr.w sprite_parts_stream_rom; $82D5BC
    plx                         ; $82D5BF
    ply                         ; $82D5C0
    jsr.w car_sprite_head2      ; $82D5C1
    lda.w #$0000                ; $82D5C4
    sta.w $8000                 ; $82D5C7
    lda.b $3C                   ; $82D5CA
    sta.w $8000                 ; $82D5CC
    lda.w $02ED                 ; $82D5CF
    xba                         ; $82D5D2
    and.w #$00FF                ; $82D5D3
    asl a                       ; $82D5D6
    asl a                       ; $82D5D7
    asl a                       ; $82D5D8
    tax                         ; $82D5D9
    lda.l $86CC66,x             ; $82D5DA
    pha                         ; $82D5DE
    lda.l $86CC68,x             ; $82D5DF
    tay                         ; $82D5E3
    lda.l $86CC64,x             ; $82D5E4
    plx                         ; $82D5E8
    jsl.l sprite_bar_put        ; $82D5E9
    lda.w $014E                 ; $82D5ED
    cmp.w #$0002                ; $82D5F0
    bne CODE_82D644             ; $82D5F3
    incbin !baserom:$155F5..$15644
CODE_82D644:
    lda.w #$FFFF                ; $82D644
    sta.w $0334                 ; $82D647
    clc                         ; $82D64A
    jmp.w $D7A1                 ; $82D64B
    lda.w $0334                 ; $82D64E
    bpl CODE_82D674             ; $82D651
    sty.w $0334                 ; $82D653
    stx.w $0336                 ; $82D656
    tyx                         ; $82D659
    lda.l $7EFAB5,x             ; $82D65A
    lsr a                       ; $82D65E
    and.w #$007E                ; $82D65F
    ldx.b $14                   ; $82D662
    clc                         ; $82D664
    adc.l $7E39EC,x             ; $82D665
    tax                         ; $82D669
    lda.l $83ACEC,x             ; $82D66A
    sta.w $0346                 ; $82D66E
    ldx.w $0336                 ; $82D671
CODE_82D674:
    lda.l $7E386A,x             ; $82D674
    sta.w $8000                 ; $82D678
    nop                         ; $82D67B
    lda.w #$9000                ; $82D67C
    sta.w $8000                 ; $82D67F
    tyx                         ; $82D682
    lda.l $7EFAB4,x             ; $82D683
    sta.w $8000                 ; $82D687
    lda.l $7EFADE,x             ; $82D68A
    and.w #$0022                ; $82D68E
    cmp.w #$0020                ; $82D691
    bne CODE_82D6F9             ; $82D694
    ldx.b $14                   ; $82D696
    lda.l $7E36EE,x             ; $82D698
    sta.w $8000                 ; $82D69C
    lda.l $7E36EC,x             ; $82D69F
    sta.w $8000                 ; $82D6A3
    lda.l $7E34EE,x             ; $82D6A6
    sta.w $8000                 ; $82D6AA
    lda.l $7E34EC,x             ; $82D6AD
    sta.w $8000                 ; $82D6B1
    tyx                         ; $82D6B4
    lda.l $7EFAB5,x             ; $82D6B5
    lsr a                       ; $82D6B9
    and.w #$007E                ; $82D6BA
    ldx.b $14                   ; $82D6BD
    clc                         ; $82D6BF
    adc.l $7E39EC,x             ; $82D6C0
    tax                         ; $82D6C4
    lda.l $83ACEC,x             ; $82D6C5
    sta.w $8000                 ; $82D6C9
    phx                         ; $82D6CC
    tyx                         ; $82D6CD
    lda.l $7EFABB,x             ; $82D6CE
    sta.w $8000                 ; $82D6D2
    nop                         ; $82D6D5
    nop                         ; $82D6D6
    lda.w $8000                 ; $82D6D7
    sta.b $2A                   ; $82D6DA
    lda.l $7EFABF,x             ; $82D6DC
    sta.w $8000                 ; $82D6E0
    lda.b $3C                   ; $82D6E3
    ora.l $7EFAD1,x             ; $82D6E5
    sta.w $8000                 ; $82D6E9
    ldx.b $14                   ; $82D6EC
    lda.l $7E34EC,x             ; $82D6EE
    sec                         ; $82D6F2
    sbc.l $7E34EE,x             ; $82D6F3
    bra CODE_82D75A             ; $82D6F7
CODE_82D6F9:
    ldx.b $14                   ; $82D6F9
    lda.l $7E36EE,x             ; $82D6FB
    sta.w $8000                 ; $82D6FF
    lda.l $7E36EC,x             ; $82D702
    sta.w $8000                 ; $82D706
    lda.l $7E34EE,x             ; $82D709
    sta.w $8000                 ; $82D70D
    lda.l $7E34EC,x             ; $82D710
    sta.w $8000                 ; $82D714
    tyx                         ; $82D717
    lda.l $7EFAB5,x             ; $82D718
    lsr a                       ; $82D71C
    and.w #$007E                ; $82D71D
    ldx.b $14                   ; $82D720
    clc                         ; $82D722
    adc.l $7E39EC,x             ; $82D723
    tax                         ; $82D727
    lda.l $83ACEC,x             ; $82D728
    sta.w $8000                 ; $82D72C
    phx                         ; $82D72F
    tyx                         ; $82D730
    lda.l $7EFABB,x             ; $82D731
    sta.w $8000                 ; $82D735
    nop                         ; $82D738
    nop                         ; $82D739
    lda.w $8000                 ; $82D73A
    sta.b $2A                   ; $82D73D
    lda.l $7EFABF,x             ; $82D73F
    sta.w $8000                 ; $82D743
    lda.b $3C                   ; $82D746
    ora.l $7EFAD1,x             ; $82D748
    sta.w $8000                 ; $82D74C
    ldx.b $14                   ; $82D74F
    lda.l $7E34EC,x             ; $82D751
    sec                         ; $82D755
    sbc.l $7E34EE,x             ; $82D756
CODE_82D75A:
    asl a                       ; $82D75A
    eor.w #$FFFF                ; $82D75B
    sec                         ; $82D75E
    adc.b $2A                   ; $82D75F
    ldx.w #$0000                ; $82D761
    cmp.w #$FFA0                ; $82D764
    bmi CODE_82D78E             ; $82D767
    inx                         ; $82D769
    inx                         ; $82D76A
    cmp.w #$FFC0                ; $82D76B
    bmi CODE_82D78E             ; $82D76E
    inx                         ; $82D770
    inx                         ; $82D771
    cmp.w #$FFE8                ; $82D772
    bmi CODE_82D78E             ; $82D775
    inx                         ; $82D777
    inx                         ; $82D778
    cmp.w #$0018                ; $82D779
    bmi CODE_82D78E             ; $82D77C
    inx                         ; $82D77E
    inx                         ; $82D77F
    cmp.w #$0040                ; $82D780
    bmi CODE_82D78E             ; $82D783
    inx                         ; $82D785
    inx                         ; $82D786
    cmp.w #$0060                ; $82D787
    bmi CODE_82D78E             ; $82D78A
    inx                         ; $82D78C
    inx                         ; $82D78D
CODE_82D78E:
    lda.l $7F0000,x             ; $82D78E
    plx                         ; $82D792
    clc                         ; $82D793
    adc.l $849180,x             ; $82D794
    tax                         ; $82D798
    lda.l $7F0000,x             ; $82D799
    tax                         ; $82D79D
    jsr.w sprite_parts_stream_rom; $82D79E
    bcs CODE_82D7D3             ; $82D7A1
    inc.b $28                   ; $82D7A3
    inc.b $28                   ; $82D7A5
    ldx.b $28                   ; $82D7A7
    lda.l $811E6A,x             ; $82D7A9
    tax                         ; $82D7AD
    lda.l $7EFAB6,x             ; $82D7AE
    cmp.b $2C                   ; $82D7B2
    bne CODE_82D7B9             ; $82D7B4
    jmp.w $D508                 ; $82D7B6
CODE_82D7B9:
    inc.b $2C                   ; $82D7B9
    lda.b $2C                   ; $82D7BB
    cmp.w $019A                 ; $82D7BD
    bne CODE_82D7C6             ; $82D7C0
    stz.b $2C                   ; $82D7C2
    stz.b $28                   ; $82D7C4
CODE_82D7C6:
    inc.b $14                   ; $82D7C6
    inc.b $14                   ; $82D7C8
    lda.b $14                   ; $82D7CA
    cmp.b $92                   ; $82D7CC
    beq CODE_82D7D3             ; $82D7CE
    jmp.w $D43E                 ; $82D7D0
CODE_82D7D3:
    stz.w $8000                 ; $82D7D3
    lda.w #$8000                ; $82D7D6
    sta.w $8000                 ; $82D7D9
    stz.w $1E0A                 ; $82D7DC
    ldx.w $1DF8                 ; $82D7DF
    beq CODE_82D82F             ; $82D7E2
    lda.l $86AE36,x             ; $82D7E4
    beq CODE_82D82F             ; $82D7E8
    sta.b $14                   ; $82D7EA
    tax                         ; $82D7EC
    lda.w #$007F                ; $82D7ED
    sta.w $1E0C                 ; $82D7F0
    lda.l $7E3A2E               ; $82D7F3
    tax                         ; $82D7F7
    lda.l $848000,x             ; $82D7F8
    asl a                       ; $82D7FC
    adc.b $14                   ; $82D7FD
    tax                         ; $82D7FF
    lda.l $860006,x             ; $82D800
    sta.w $1E06                 ; $82D804
    lda.l $860008,x             ; $82D807
    sta.w $1E0A                 ; $82D80B
    ldx.b $14                   ; $82D80E
    lda.l $860000,x             ; $82D810
    tax                         ; $82D814
    lda.l $7E6591,x             ; $82D815
    clc                         ; $82D819
    adc.w $1E06                 ; $82D81A
    sta.w $1E06                 ; $82D81D
    lda.l $7E46C8               ; $82D820
    asl a                       ; $82D824
    asl a                       ; $82D825
    asl a                       ; $82D826
    asl a                       ; $82D827
    clc                         ; $82D828
    adc.w #$1000                ; $82D829
    sta.w $1E08                 ; $82D82C
CODE_82D82F:
    lda.w $0184                 ; $82D82F
    sec                         ; $82D832
    sbc.w #$0440                ; $82D833
    sta.w $0188                 ; $82D836
    lda.b $92                   ; $82D839
    sec                         ; $82D83B
    sbc.w #$0040                ; $82D83C
    sta.b $92                   ; $82D83F
    lda.w $02ED                 ; $82D841
    xba                         ; $82D844
    and.w #$00FF                ; $82D845
    asl a                       ; $82D848
    asl a                       ; $82D849
    asl a                       ; $82D84A
    tax                         ; $82D84B
    lda.l $86C974,x             ; $82D84C
    sta.w $1E58                 ; $82D850
    lda.l $86C976,x             ; $82D853
    sta.w $1E5A                 ; $82D857
    lda.l $86C978,x             ; $82D85A
    sta.w $1E5C                 ; $82D85E
    plb                         ; $82D861
    rts                         ; $82D862
car_sprite_head2:
    lda.l $7E386A,x             ; $82D863
    sta.w $8000                 ; $82D867
    lda.w #$9000                ; $82D86A
    sta.w $8000                 ; $82D86D
    tyx                         ; $82D870
    lda.l $7EFAB4,x             ; $82D871
    sta.w $8000                 ; $82D875
    ldx.b $14                   ; $82D878
    lda.w $02F6                 ; $82D87A
    and.w #$00FF                ; $82D87D
    bit.w #$0080                ; $82D880
    beq CODE_82D888             ; $82D883
    incbin !baserom:$15885..$15888
CODE_82D888:
    clc                         ; $82D888
    adc.w #$0033                ; $82D889
    clc                         ; $82D88C
    adc.b $34                   ; $82D88D
    sta.w $8000                 ; $82D88F
    sta.w $8000                 ; $82D892
    lda.w #$0000                ; $82D895
    sta.w $8000                 ; $82D898
    lda.b $32                   ; $82D89B
    sta.w $8000                 ; $82D89D
    tyx                         ; $82D8A0
    lda.l $7EFAB5,x             ; $82D8A1
    lsr a                       ; $82D8A5
    and.w #$007E                ; $82D8A6
    ldx.b $14                   ; $82D8A9
    clc                         ; $82D8AB
    adc.l $7E39EC,x             ; $82D8AC
    tax                         ; $82D8B0
    lda.l $83ACEC,x             ; $82D8B1
    sta.w $8000                 ; $82D8B5
    tyx                         ; $82D8B8
    lda.w #$0000                ; $82D8B9
    sta.w $8000                 ; $82D8BC
    nop                         ; $82D8BF
    lda.w $8000                 ; $82D8C0
    sta.b $2A                   ; $82D8C3
    rts                         ; $82D8C5
minimap_car:
    pha                         ; $82D8C6
    phx                         ; $82D8C7
    ldx.w $047B                 ; $82D8C8
    lda.l $82DC68,x             ; $82D8CB
    bpl CODE_82D8EC             ; $82D8CF
    lda.b $14                   ; $82D8D1
    beq CODE_82D8EC             ; $82D8D3
    bpl CODE_82D8DE             ; $82D8D5
    ldx.w #$0000                ; $82D8D7
    pla                         ; $82D8DA
    pla                         ; $82D8DB
    bra CODE_82D8EE             ; $82D8DC
CODE_82D8DE:
    lda.w $019A                 ; $82D8DE
    dec a                       ; $82D8E1
    lsr a                       ; $82D8E2
    lsr a                       ; $82D8E3
    and.w #$FFFC                ; $82D8E4
    tax                         ; $82D8E7
    pla                         ; $82D8E8
    pla                         ; $82D8E9
    bra CODE_82D8EE             ; $82D8EA
CODE_82D8EC:
    plx                         ; $82D8EC
    pla                         ; $82D8ED
CODE_82D8EE:
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
sprite_put_rows_a:
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
    jmp.w sprite_parts_stream   ; $82D985
sprite_put_rows_b:
    ldx.b $14                   ; $82D988
    lda.l $7E386C,x             ; $82D98A
    sta.w $8000                 ; $82D98E
    lda.l $7E38EC,x             ; $82D991
    sta.w $8000                 ; $82D995
    lda.l $7E366C,x             ; $82D998
    sta.w $8000                 ; $82D99C
    lda.l $7E386C,x             ; $82D99F
    sta.w $8000                 ; $82D9A3
    lda.b $20                   ; $82D9A6
    sta.w $8000                 ; $82D9A8
    lda.b $22                   ; $82D9AB
    sta.w $8000                 ; $82D9AD
    lda.b $26                   ; $82D9B0
    ora.w #$1000                ; $82D9B2
    sta.w $8000                 ; $82D9B5
    ldx.b $24                   ; $82D9B8
    lda.l $860000,x             ; $82D9BA
    tax                         ; $82D9BE
    lda.l $7E6591,x             ; $82D9BF
    pha                         ; $82D9C3
    ldx.b $14                   ; $82D9C4
    lda.l $7E39EC,x             ; $82D9C6
    tax                         ; $82D9CA
    lda.l $848000,x             ; $82D9CB
    clc                         ; $82D9CF
    adc.b $24                   ; $82D9D0
    tax                         ; $82D9D2
    pla                         ; $82D9D3
    clc                         ; $82D9D4
    adc.l $860002,x             ; $82D9D5
    tax                         ; $82D9D9
    jmp.w sprite_parts_stream   ; $82D9DA
    incbin !baserom:$159DD..$15A46
clc_82DA46:
    clc                         ; $82DA46
    rts                         ; $82DA47
object_sprite_shadowed:
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
    jsr.w sprite_put_rows_a     ; $82DA90
    bcs CODE_82DA9F             ; $82DA93
    stz.b $22                   ; $82DA95
    lda.w #$BC30                ; $82DA97
    sta.b $24                   ; $82DA9A
    jmp.w sprite_put_rows_b     ; $82DA9C
CODE_82DA9F:
    rts                         ; $82DA9F
CODE_82DAA0:
    jsr.w sprite_put_rows_a     ; $82DAA0
    bcs CODE_82DA9F             ; $82DAA3
    stz.b $22                   ; $82DAA5
    lda.w #$BC30                ; $82DAA7
    sta.b $24                   ; $82DAAA
    jmp.w sprite_put_rows_a     ; $82DAAC
object_sprite_hover:
    lda.l $86AF06,x             ; $82DAAF
    sta.b $24                   ; $82DAB3
    ldx.b $1C                   ; $82DAB5
    lda.l $7E8CB3,x             ; $82DAB7
    and.w #$00FF                ; $82DABB
    asl a                       ; $82DABE
    asl a                       ; $82DABF
    asl a                       ; $82DAC0
    asl a                       ; $82DAC1
    bit.w #$0800                ; $82DAC2
    beq CODE_82DACA             ; $82DAC5
    ora.w #$F000                ; $82DAC7
CODE_82DACA:
    sta.b $20                   ; $82DACA
    lda.l $7E8CB4,x             ; $82DACC
    and.w #$0700                ; $82DAD0
    asl a                       ; $82DAD3
    ora.b $26                   ; $82DAD4
    sta.b $26                   ; $82DAD6
    lda.w $0024                 ; $82DAD8
    sec                         ; $82DADB
    sbc.l $7E8CB6,x             ; $82DADC
    and.w #$003E                ; $82DAE0
    tax                         ; $82DAE3
    lda.l $84A200,x             ; $82DAE4
    clc                         ; $82DAE8
    adc.w #$FFC0                ; $82DAE9
    sta.b $22                   ; $82DAEC
    lda.l $7E8CB5,x             ; $82DAEE
    and.w #$0010                ; $82DAF2
    beq CODE_82DAFA             ; $82DAF5
    jmp.w sprite_put_rows_a     ; $82DAF7
CODE_82DAFA:
    jmp.w sprite_put_rows_a     ; $82DAFA
object_sprite_raised:
    lda.l $86AF06,x             ; $82DAFD
    sta.b $24                   ; $82DB01
    ldx.b $1C                   ; $82DB03
    lda.l $7E8CB3,x             ; $82DB05
    and.w #$00FF                ; $82DB09
    asl a                       ; $82DB0C
    asl a                       ; $82DB0D
    asl a                       ; $82DB0E
    asl a                       ; $82DB0F
    bit.w #$0800                ; $82DB10
    beq CODE_82DB18             ; $82DB13
    ora.w #$F000                ; $82DB15
CODE_82DB18:
    sta.b $20                   ; $82DB18
    lda.l $7E8CB4,x             ; $82DB1A
    and.w #$0700                ; $82DB1E
    asl a                       ; $82DB21
    ora.b $26                   ; $82DB22
    sta.b $26                   ; $82DB24
    lda.l $7E8CB5,x             ; $82DB26
    and.w #$00F0                ; $82DB2A
    lsr a                       ; $82DB2D
    pha                         ; $82DB2E
    lda.w $0024                 ; $82DB2F
    sec                         ; $82DB32
    sbc.l $7E8CB6,x             ; $82DB33
    and.w #$003E                ; $82DB37
    tax                         ; $82DB3A
    pla                         ; $82DB3B
    eor.w #$FFFF                ; $82DB3C
    sec                         ; $82DB3F
    adc.l $84A200,x             ; $82DB40
    clc                         ; $82DB44
    adc.w #$FFE0                ; $82DB45
    sta.b $22                   ; $82DB48
    jmp.w sprite_put_rows_a     ; $82DB4A
object_sprite_flat:
    lda.l $86AF06,x             ; $82DB4D
    sta.b $24                   ; $82DB51
    ldx.b $1C                   ; $82DB53
    lda.l $7E8CB4,x             ; $82DB55
    and.w #$0700                ; $82DB59
    asl a                       ; $82DB5C
    ora.b $26                   ; $82DB5D
    sta.b $26                   ; $82DB5F
    lda.l $7E8CB3,x             ; $82DB61
    and.w #$00FF                ; $82DB65
    asl a                       ; $82DB68
    asl a                       ; $82DB69
    asl a                       ; $82DB6A
    asl a                       ; $82DB6B
    bit.w #$0800                ; $82DB6C
    beq CODE_82DB74             ; $82DB6F
    ora.w #$F000                ; $82DB71
CODE_82DB74:
    sta.b $20                   ; $82DB74
    stz.b $22                   ; $82DB76
    lda.l $7E8CB5,x             ; $82DB78
    and.w #$0010                ; $82DB7C
    beq CODE_82DB84             ; $82DB7F
    jmp.w sprite_put_rows_b     ; $82DB81
CODE_82DB84:
    jmp.w sprite_put_rows_a     ; $82DB84
object_sprite_group:
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
    jsr.w sprite_put_rows_a     ; $82DBCA
    bcs CODE_82DBF4             ; $82DBCD
    lda.b $20                   ; $82DBCF
    eor.w #$FFFF                ; $82DBD1
    inc a                       ; $82DBD4
    sta.b $20                   ; $82DBD5
    jsr.w sprite_put_rows_a     ; $82DBD7
    bcs CODE_82DBF4             ; $82DBDA
    lda.b $20                   ; $82DBDC
    sec                         ; $82DBDE
    sbc.w #$0080                ; $82DBDF
    sta.b $20                   ; $82DBE2
    jsr.w sprite_put_rows_a     ; $82DBE4
    bcs CODE_82DBF4             ; $82DBE7
    lda.b $20                   ; $82DBE9
    eor.w #$FFFF                ; $82DBEB
    inc a                       ; $82DBEE
    sta.b $20                   ; $82DBEF
    jmp.w sprite_put_rows_a     ; $82DBF1
CODE_82DBF4:
    rts                         ; $82DBF4
    tyx                         ; $82DBF5
    lda.l $7E8CB3,x             ; $82DBF6
    and.w #$00FF                ; $82DBFA
    asl a                       ; $82DBFD
    asl a                       ; $82DBFE
    asl a                       ; $82DBFF
    asl a                       ; $82DC00
    bit.w #$0800                ; $82DC01
    beq CODE_82DC09             ; $82DC04
    ora.w #$F000                ; $82DC06
CODE_82DC09:
    sta.b $20                   ; $82DC09
    stz.b $22                   ; $82DC0B
    jsr.w sprite_put_rows_a     ; $82DC0D
    bcs CODE_82DBF4             ; $82DC10
    jsr.w sprite_put_rows_b     ; $82DC12
    bcs CODE_82DBF4             ; $82DC15
    lda.b $20                   ; $82DC17
    eor.w #$FFFF                ; $82DC19
    inc a                       ; $82DC1C
    sta.b $20                   ; $82DC1D
    jsr.w sprite_put_rows_a     ; $82DC1F
    bcs CODE_82DBF4             ; $82DC22
    jmp.w sprite_put_rows_b     ; $82DC24
    tyx                         ; $82DC27
    lda.l $7E8CB3,x             ; $82DC28
    and.w #$00FF                ; $82DC2C
    asl a                       ; $82DC2F
    asl a                       ; $82DC30
    asl a                       ; $82DC31
    asl a                       ; $82DC32
    bit.w #$0800                ; $82DC33
    beq CODE_82DC3B             ; $82DC36
    ora.w #$F000                ; $82DC38 static
CODE_82DC3B:
    sta.b $20                   ; $82DC3B
    stz.b $22                   ; $82DC3D
    jsr.w sprite_put_rows_b     ; $82DC3F
    bcs CODE_82DBF4             ; $82DC42
    lda.b $20                   ; $82DC44
    eor.w #$FFFF                ; $82DC46
    inc a                       ; $82DC49
    sta.b $20                   ; $82DC4A
    jsr.w sprite_put_rows_b     ; $82DC4C
    bcs CODE_82DBF4             ; $82DC4F
    lda.b $20                   ; $82DC51
    sec                         ; $82DC53
    sbc.w #$0080                ; $82DC54
    sta.b $20                   ; $82DC57
    jsr.w sprite_put_rows_b     ; $82DC59
    bcs CODE_82DBF4             ; $82DC5C
    lda.b $20                   ; $82DC5E
    eor.w #$FFFF                ; $82DC60
    inc a                       ; $82DC63
    sta.b $20                   ; $82DC64
    jmp.w sprite_put_rows_b     ; $82DC66
    incbin !baserom:$15C69..$15CEC
object_sprite_kind0:
    lda.l $86AF06,x             ; $82DCEC
    sta.b $24                   ; $82DCF0
    ldx.b $1C                   ; $82DCF2
    lda.l $7E8CB3,x             ; $82DCF4
    and.w #$00FF                ; $82DCF8
    asl a                       ; $82DCFB
    asl a                       ; $82DCFC
    asl a                       ; $82DCFD
    asl a                       ; $82DCFE
    bit.w #$0800                ; $82DCFF
    beq CODE_82DD07             ; $82DD02
    ora.w #$F000                ; $82DD04
CODE_82DD07:
    sta.b $20                   ; $82DD07
    lda.l $7E8CB4,x             ; $82DD09
    and.w #$0700                ; $82DD0D
    asl a                       ; $82DD10
    ora.b $26                   ; $82DD11
    sta.b $26                   ; $82DD13
    lda.w $0024                 ; $82DD15
    sec                         ; $82DD18
    sbc.l $7E8CB6,x             ; $82DD19
    and.w #$003E                ; $82DD1D
    tax                         ; $82DD20
    lda.l $84A200,x             ; $82DD21
    clc                         ; $82DD25
    adc.w #$FFC0                ; $82DD26
    sta.b $22                   ; $82DD29
    jsr.w sprite_put_rows_c     ; $82DD2B
    bcs CODE_82DD3A             ; $82DD2E
    stz.b $22                   ; $82DD30
    lda.w #$BC30                ; $82DD32
    sta.b $24                   ; $82DD35
    jmp.w sprite_put_rows_c     ; $82DD37
CODE_82DD3A:
    rts                         ; $82DD3A
vs_object_flat:
    lda.l $86AF06,x             ; $82DD3B
    sta.b $24                   ; $82DD3F
    ldx.b $1C                   ; $82DD41
    lda.l $7E8CB4,x             ; $82DD43
    and.w #$0700                ; $82DD47
    asl a                       ; $82DD4A
    ora.b $26                   ; $82DD4B
    sta.b $26                   ; $82DD4D
    lda.l $7E8CB3,x             ; $82DD4F
    and.w #$00FF                ; $82DD53
    asl a                       ; $82DD56
    asl a                       ; $82DD57
    asl a                       ; $82DD58
    asl a                       ; $82DD59
    bit.w #$0800                ; $82DD5A
    beq CODE_82DD62             ; $82DD5D
    ora.w #$F000                ; $82DD5F
CODE_82DD62:
    sta.b $20                   ; $82DD62
    stz.b $22                   ; $82DD64
    jmp.w sprite_put_rows_c     ; $82DD66
vs_object_group:
    lda.l $86AF06,x             ; $82DD69
    sta.b $24                   ; $82DD6D
    ldx.b $1C                   ; $82DD6F
    lda.l $7E8CB4,x             ; $82DD71
    and.w #$0700                ; $82DD75
    asl a                       ; $82DD78
    ora.b $26                   ; $82DD79
    sta.b $26                   ; $82DD7B
    lda.l $7E8CB3,x             ; $82DD7D
    and.w #$00FF                ; $82DD81
    asl a                       ; $82DD84
    asl a                       ; $82DD85
    asl a                       ; $82DD86
    asl a                       ; $82DD87
    bit.w #$0800                ; $82DD88
    beq CODE_82DD90             ; $82DD8B
    ora.w #$F000                ; $82DD8D
CODE_82DD90:
    sta.b $20                   ; $82DD90
    stz.b $22                   ; $82DD92
    jsr.w sprite_put_rows_c     ; $82DD94
    bcs CODE_82DDBE             ; $82DD97
    lda.b $20                   ; $82DD99
    eor.w #$FFFF                ; $82DD9B
    inc a                       ; $82DD9E
    sta.b $20                   ; $82DD9F
    jsr.w sprite_put_rows_c     ; $82DDA1
    bcs CODE_82DDBE             ; $82DDA4
    lda.b $20                   ; $82DDA6
    sec                         ; $82DDA8
    sbc.w #$0080                ; $82DDA9
    sta.b $20                   ; $82DDAC
    jsr.w sprite_put_rows_c     ; $82DDAE
    bcs CODE_82DDBE             ; $82DDB1
    lda.b $20                   ; $82DDB3
    eor.w #$FFFF                ; $82DDB5
    inc a                       ; $82DDB8
    sta.b $20                   ; $82DDB9
    jmp.w sprite_put_rows_c     ; $82DDBB
CODE_82DDBE:
    rts                         ; $82DDBE
sprite_parts_stream:
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
    beq CODE_82DE63             ; $82DDF4
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
    beq CODE_82DE87             ; $82DE44
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
CODE_82DE63:
    lda.w $C000                 ; $82DE63 static
    bpl CODE_82DE63             ; $82DE66 static
    lda.w $8000                 ; $82DE68 static
    beq CODE_82DE7D             ; $82DE6B static
    lda.w $8000                 ; $82DE6D static
    xba                         ; $82DE70 static
    xba                         ; $82DE71 static
    xba                         ; $82DE72 static
    xba                         ; $82DE73 static
    lda.w $8000                 ; $82DE74 static
    xba                         ; $82DE77 static
    xba                         ; $82DE78 static
    xba                         ; $82DE79 static
    xba                         ; $82DE7A static
    bra CODE_82DE63             ; $82DE7B static
CODE_82DE7D:
    stz.w $8000                 ; $82DE7D static
    stz.w $8000                 ; $82DE80 static
    sty.b $84                   ; $82DE83 static
    sec                         ; $82DE85 static
    rts                         ; $82DE86 static
CODE_82DE87:
    lda.w $C000                 ; $82DE87
    bpl CODE_82DE87             ; $82DE8A
    lda.w $8000                 ; $82DE8C
    beq CODE_82DEA1             ; $82DE8F
    lda.w $8000                 ; $82DE91
    xba                         ; $82DE94
    xba                         ; $82DE95
    xba                         ; $82DE96
    xba                         ; $82DE97
    lda.w $8000                 ; $82DE98
    xba                         ; $82DE9B
    xba                         ; $82DE9C
    xba                         ; $82DE9D
    xba                         ; $82DE9E
    bra CODE_82DE87             ; $82DE9F
CODE_82DEA1:
    stz.w $8000                 ; $82DEA1
    sty.b $84                   ; $82DEA4
    sec                         ; $82DEA6
    rts                         ; $82DEA7
sprite_parts_stream_rom:
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
    beq CODE_82DF4C             ; $82DEDD
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
    beq CODE_82DF70             ; $82DF2D
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
CODE_82DF4C:
    lda.w $C000                 ; $82DF4C static
    bpl CODE_82DF4C             ; $82DF4F static
    lda.w $8000                 ; $82DF51 static
    beq CODE_82DF66             ; $82DF54 static
    lda.w $8000                 ; $82DF56 static
    xba                         ; $82DF59 static
    xba                         ; $82DF5A static
    xba                         ; $82DF5B static
    xba                         ; $82DF5C static
    lda.w $8000                 ; $82DF5D static
    xba                         ; $82DF60 static
    xba                         ; $82DF61 static
    xba                         ; $82DF62 static
    xba                         ; $82DF63 static
    bra CODE_82DF4C             ; $82DF64 static
CODE_82DF66:
    stz.w $8000                 ; $82DF66 static
    stz.w $8000                 ; $82DF69 static
    sty.b $84                   ; $82DF6C static
    sec                         ; $82DF6E static
    rts                         ; $82DF6F static
CODE_82DF70:
    lda.w $C000                 ; $82DF70
    bpl CODE_82DF70             ; $82DF73
    lda.w $8000                 ; $82DF75
    beq CODE_82DF8A             ; $82DF78
    lda.w $8000                 ; $82DF7A static
    xba                         ; $82DF7D static
    xba                         ; $82DF7E static
    xba                         ; $82DF7F static
    xba                         ; $82DF80 static
    lda.w $8000                 ; $82DF81 static
    xba                         ; $82DF84 static
    xba                         ; $82DF85 static
    xba                         ; $82DF86 static
    xba                         ; $82DF87 static
    bra CODE_82DF70             ; $82DF88 static
CODE_82DF8A:
    stz.w $8000                 ; $82DF8A
    sty.b $84                   ; $82DF8D
    sec                         ; $82DF8F
    rts                         ; $82DF90
vs_road_rows:
    jsl.l vs_road_rows_view0    ; $82DF91
    jsl.l vs_road_rows_view1    ; $82DF95
    jsl.l vs_road_rows_view2    ; $82DF99
    jsl.l vs_road_rows_view3    ; $82DF9D
    rts                         ; $82DFA1
    jsr.w road_rows_build       ; $82DFA2
    jsr.w mode1_road_rows_view2 ; $82DFA5
    rts                         ; $82DFA8
road_rows_build_call:
    jsr.w road_rows_build       ; $82DFA9
    rts                         ; $82DFAC
road_rows_build:
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
    clc                         ; $82E0C7
    adc.w $1EE0                 ; $82E0C8
    sta.b $16                   ; $82E0CB
    lda.w #$7FFF                ; $82E0CD
    sta.b $22                   ; $82E0D0
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
CODE_82E178:
    lda.l $850000,x             ; $82E178
    tay                         ; $82E17C
    lda.l $850002,x             ; $82E17D
    cmp.b $1A                   ; $82E181
    bcs CODE_82E19A             ; $82E183
    lda.l $850004,x             ; $82E185 static
    tay                         ; $82E189 static
    lda.l $850006,x             ; $82E18A static
    cmp.b $1A                   ; $82E18E static
    bcs CODE_82E19A             ; $82E190 static
    txa                         ; $82E192 static
    clc                         ; $82E193 static
    adc.w #$0008                ; $82E194 static
    tax                         ; $82E197 static
    bra CODE_82E178             ; $82E198 static
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
    bcs CODE_82E1D3             ; $82E1B9
    lda.b $16                   ; $82E1BB
    inc a                       ; $82E1BD
    cmp.b $22                   ; $82E1BE
    bne CODE_82E1D0             ; $82E1C0
    bit.w $1E66                 ; $82E1C2
    bpl CODE_82E1D0             ; $82E1C5
    lda.w #$8002                ; $82E1C7
    sta.w $3D6C,y               ; $82E1CA
    sta.w $3DAC,y               ; $82E1CD
CODE_82E1D0:
    jmp.w $E2C3                 ; $82E1D0
CODE_82E1D3:
    cmp.w #$0040                ; $82E1D3
    bcc CODE_82E218             ; $82E1D6
    lda.w #$0008                ; $82E1D8
    ora.w $3D6C,y               ; $82E1DB
    sta.w $3D6C,y               ; $82E1DE
    sta.w $3DAC,y               ; $82E1E1
    lda.w $3CEA,y               ; $82E1E4
    sta.w $3FEC,y               ; $82E1E7
    sta.w $402C,y               ; $82E1EA
    lda.b $A2                   ; $82E1ED
    clc                         ; $82E1EF
    adc.w #$0014                ; $82E1F0
    sta.b $A2                   ; $82E1F3
    bit.w $1E66                 ; $82E1F5
    bmi CODE_82E206             ; $82E1F8
    lda.l $840010,x             ; $82E1FA
    sta.w $1EE6                 ; $82E1FE
    clc                         ; $82E201
    adc.b $16                   ; $82E202
    sta.b $16                   ; $82E204
CODE_82E206:
    lda.b $9C                   ; $82E206
    clc                         ; $82E208
    adc.b $A2                   ; $82E209
    tax                         ; $82E20B
    lda.b $16                   ; $82E20C
    sec                         ; $82E20E
    sbc.l $840000,x             ; $82E20F
    bcs CODE_82E1D3             ; $82E213
    jmp.w $E2D5                 ; $82E215
CODE_82E218:
    cmp.w #$003E                ; $82E218
    bne CODE_82E22E             ; $82E21B
    bit.w $1E66                 ; $82E21D
    bpl CODE_82E22E             ; $82E220
    lda.w #$8001                ; $82E222
    sta.w $3D6C,y               ; $82E225
    sta.w $3DAC,y               ; $82E228
    lda.w #$003E                ; $82E22B
CODE_82E22E:
    asl a                       ; $82E22E
    sta.b $1E                   ; $82E22F
    stx.b $20                   ; $82E231
    clc                         ; $82E233
    adc.l $840004,x             ; $82E234
    tax                         ; $82E238
    lda.l $850000,x             ; $82E239
    sta.w $3DEC,y               ; $82E23D
    sta.w $3E2C,y               ; $82E240
    ldx.b $20                   ; $82E243
    lda.b $1E                   ; $82E245
    clc                         ; $82E247
    adc.l $840006,x             ; $82E248
    tax                         ; $82E24C
    lda.l $850000,x             ; $82E24D
    sta.w $3E6C,y               ; $82E251
    sta.w $3EAC,y               ; $82E254
    ldx.b $20                   ; $82E257
    lda.l $84000A,x             ; $82E259
    tax                         ; $82E25D
    jmp.w ($E261,x)             ; $82E25E
    incbin !baserom:$16261..$16265
    lda.w #$FF00                ; $82E265 static
    sta.w $3C6C,y               ; $82E268 static
    sta.w $3CAC,y               ; $82E26B static
    lda.w #$0100                ; $82E26E static
    sta.w $3F6C,y               ; $82E271 static
    sta.w $3FAC,y               ; $82E274 static
    bra CODE_82E2A3             ; $82E277 static
    lda.w $3DEC,y               ; $82E279
    bpl CODE_82E291             ; $82E27C
    sec                         ; $82E27E
    ror a                       ; $82E27F
    sta.w $3C6C,y               ; $82E280
    sta.w $3CAC,y               ; $82E283
    lda.w #$3FFF                ; $82E286
    sta.w $3F6C,y               ; $82E289
    sta.w $3FAC,y               ; $82E28C
    bra CODE_82E2A3             ; $82E28F
CODE_82E291:
    lsr a                       ; $82E291
    sta.w $3F6C,y               ; $82E292
    sta.w $3FAC,y               ; $82E295
    lda.w #$C001                ; $82E298
    sta.w $3C6C,y               ; $82E29B
    sta.w $3CAC,y               ; $82E29E
    bra CODE_82E2A3             ; $82E2A1
CODE_82E2A3:
    ldx.b $20                   ; $82E2A3
    lda.b $1E                   ; $82E2A5
    cmp.w #$0040                ; $82E2A7
    bcc CODE_82E2B1             ; $82E2AA
    sbc.w #$0040                ; $82E2AC
    inx                         ; $82E2AF
    inx                         ; $82E2B0
CODE_82E2B1:
    clc                         ; $82E2B1
    adc.l $84000C,x             ; $82E2B2
    tax                         ; $82E2B6
    lda.l $7E5391,x             ; $82E2B7
    sta.w $3FEC,y               ; $82E2BB
    sta.w $402C,y               ; $82E2BE
    bra CODE_82E2F6             ; $82E2C1
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
CODE_82E2F6:
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
    bcs CODE_82E330             ; $82E32B
    jmp.w $E429                 ; $82E32D
CODE_82E330:
    ldy.w $0100                 ; $82E330
    lda.l $7E5B94,x             ; $82E333
    beq CODE_82E3A1             ; $82E337
    bit.w #$FF00                ; $82E339
    bne CODE_82E3A4             ; $82E33C
    asl a                       ; $82E33E
    tax                         ; $82E33F
    lda.l $86AE36,x             ; $82E340
    tax                         ; $82E344
    lda.l $86001E,x             ; $82E345
    sta.w $F533,y               ; $82E349
    lda.l $860020,x             ; $82E34C
    sta.w $F433,y               ; $82E350
    lda.w #$007F                ; $82E353
    sta.w $F733,y               ; $82E356
    lda.l $860000,x             ; $82E359
    tax                         ; $82E35D
    lda.l $7E6591,x             ; $82E35E
    clc                         ; $82E362
    adc.w $F533,y               ; $82E363
    sta.w $F533,y               ; $82E366
    ldx.b $A0                   ; $82E369
    lda.l $7E5B93,x             ; $82E36B
    and.w #$00FF                ; $82E36F
    xba                         ; $82E372
    lsr a                       ; $82E373
    lsr a                       ; $82E374
    lsr a                       ; $82E375
    sta.w $F633,y               ; $82E376
    lda.l $7E5B93,x             ; $82E379
    and.w #$00FF                ; $82E37D
    asl a                       ; $82E380
    sta.w $F933,y               ; $82E381
    lda.l $7E5B93,x             ; $82E384
    xba                         ; $82E388
    and.w #$00FF                ; $82E389
    asl a                       ; $82E38C
    clc                         ; $82E38D
    adc.w #$45EC                ; $82E38E
    sta.w $F833,y               ; $82E391
    jsl.l func_85CDAF           ; $82E394
    iny                         ; $82E398
    iny                         ; $82E399
    tya                         ; $82E39A
    and.w #$00FF                ; $82E39B
    tay                         ; $82E39E
    bra CODE_82E413             ; $82E39F
CODE_82E3A1:
    jmp.w $E407                 ; $82E3A1
CODE_82E3A4:
    asl a                       ; $82E3A4
    tax                         ; $82E3A5
    lda.l $86AC36,x             ; $82E3A6
    tax                         ; $82E3AA
    lda.l $860022,x             ; $82E3AB
    sta.w $F533,y               ; $82E3AF
    lda.l $860024,x             ; $82E3B2
    sta.w $F433,y               ; $82E3B6
    lda.w #$007F                ; $82E3B9
    sta.w $F733,y               ; $82E3BC
    lda.l $860000,x             ; $82E3BF
    tax                         ; $82E3C3
    lda.l $7E6591,x             ; $82E3C4
    clc                         ; $82E3C8
    adc.w $F533,y               ; $82E3C9
    sta.w $F533,y               ; $82E3CC
    ldx.b $A0                   ; $82E3CF
    lda.l $7E5B93,x             ; $82E3D1
    and.w #$00FF                ; $82E3D5
    xba                         ; $82E3D8
    lsr a                       ; $82E3D9
    lsr a                       ; $82E3DA
    lsr a                       ; $82E3DB
    sta.w $F633,y               ; $82E3DC
    lda.l $7E5B93,x             ; $82E3DF
    and.w #$00FF                ; $82E3E3
    asl a                       ; $82E3E6
    sta.w $F933,y               ; $82E3E7
    lda.l $7E5B93,x             ; $82E3EA
    xba                         ; $82E3EE
    and.w #$00FF                ; $82E3EF
    asl a                       ; $82E3F2
    clc                         ; $82E3F3
    adc.w #$495C                ; $82E3F4
    sta.w $F833,y               ; $82E3F7
    jsl.l func_85CDAF           ; $82E3FA
    iny                         ; $82E3FE
    iny                         ; $82E3FF
    tya                         ; $82E400
    and.w #$00FF                ; $82E401
    tay                         ; $82E404
    bra CODE_82E413             ; $82E405
    lda.l $7E5B93,x             ; $82E407
    and.w #$00FF                ; $82E40B
    asl a                       ; $82E40E
    sta.l $7E45EC               ; $82E40F
CODE_82E413:
    inx                         ; $82E413
    inx                         ; $82E414
    inx                         ; $82E415
    inx                         ; $82E416
    inx                         ; $82E417
    stx.b $A0                   ; $82E418
    lda.w $0226                 ; $82E41A
    cmp.l $7E5B91,x             ; $82E41D
    bcc CODE_82E426             ; $82E421
    jmp.w $E333                 ; $82E423
CODE_82E426:
    sty.w $0100                 ; $82E426
    plb                         ; $82E429
    lda.w $0226                 ; $82E42A
    cmp.w $019A                 ; $82E42D
    bcc CODE_82E44D             ; $82E430
    sbc.w $019A                 ; $82E432
    sta.w $0226                 ; $82E435
    sta.w $023E                 ; $82E438
    stz.b $A0                   ; $82E43B
    stz.w $1E5E                 ; $82E43D
    stz.w $1E60                 ; $82E440
    lda.w #$0140                ; $82E443
    sta.l $7E45EC               ; $82E446
    jmp.w $E31E                 ; $82E44A
CODE_82E44D:
    sta.w $0226                 ; $82E44D
    sta.w $023E                 ; $82E450
    stz.w $0260                 ; $82E453
    ldx.b $9C                   ; $82E456
CODE_82E458:
    lda.w $0226                 ; $82E458
    inc a                       ; $82E45B
    inc a                       ; $82E45C
    sec                         ; $82E45D
    sbc.l $840000,x             ; $82E45E
    bcc CODE_82E48E             ; $82E462
    cmp.w #$0041                ; $82E464
    bcc CODE_82E471             ; $82E467
    txa                         ; $82E469
    clc                         ; $82E46A
    adc.w #$0014                ; $82E46B
    tax                         ; $82E46E
    bra CODE_82E458             ; $82E46F
CODE_82E471:
    lda.l $840002,x             ; $82E471
    and.w #$00FF                ; $82E475
    cmp.w #$000A                ; $82E478
    bne CODE_82E487             ; $82E47B
    lda.w $1EDE                 ; $82E47D
    inc a                       ; $82E480
    cmp.l $840000,x             ; $82E481
    beq CODE_82E48E             ; $82E485
CODE_82E487:
    lda.l $840002,x             ; $82E487
    sta.w $0260                 ; $82E48B
CODE_82E48E:
    lda.w $0260                 ; $82E48E
    and.w #$00FF                ; $82E491
    sta.w $0142                 ; $82E494
    lda.w $0226                 ; $82E497
    clc                         ; $82E49A
    adc.w #$0001                ; $82E49B
    cmp.w $019A                 ; $82E49E
    bcc CODE_82E4A6             ; $82E4A1
    sbc.w $019A                 ; $82E4A3
CODE_82E4A6:
    jsr.w func_82EA49           ; $82E4A6
    sta.b $14                   ; $82E4A9
    lda.w $0226                 ; $82E4AB
    sec                         ; $82E4AE
    sbc.w #$0003                ; $82E4AF
    bcs CODE_82E4B7             ; $82E4B2
    adc.w $019A                 ; $82E4B4
CODE_82E4B7:
    jsr.w func_82EA49           ; $82E4B7
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
    ora.w #$FF00                ; $82E4EA
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
mode1_road_rows_view2:
    pea.w $837E                 ; $82E503
    plb                         ; $82E506
    lda.w $02F1                 ; $82E507
    clc                         ; $82E50A
    adc.w $1EE8                 ; $82E50B
    asl a                       ; $82E50E
    and.w #$003E                ; $82E50F
    tax                         ; $82E512
    lda.w $02D9                 ; $82E513
    sec                         ; $82E516
    sbc.w $02F1                 ; $82E517
    beq CODE_82E595             ; $82E51A
    tay                         ; $82E51C
    lda.w #$0000                ; $82E51D
    sta.l $308000               ; $82E520
    sec                         ; $82E524
    sbc.w $02EF                 ; $82E525
    xba                         ; $82E528
    and.w #$00FF                ; $82E529
    sta.l $308000               ; $82E52C
    lda.w $406E,x               ; $82E530
    sta.l $308000               ; $82E533
    nop                         ; $82E537
    lda.l $308000               ; $82E538
    xba                         ; $82E53C
    pha                         ; $82E53D
    and.w #$FF00                ; $82E53E
    sta.b $14                   ; $82E541
    pla                         ; $82E543
    and.w #$00FF                ; $82E544
    sta.b $16                   ; $82E547
    lda.l $308000               ; $82E549
    xba                         ; $82E54D
    and.w #$FF00                ; $82E54E
    ora.b $16                   ; $82E551
    sta.w $01D2                 ; $82E553
    inx                         ; $82E556
    inx                         ; $82E557
    dey                         ; $82E558
    beq CODE_82E566             ; $82E559
CODE_82E55B:
    adc.w $406E,x               ; $82E55B
    inx                         ; $82E55E
    inx                         ; $82E55F
    dey                         ; $82E560
    bne CODE_82E55B             ; $82E561
    sta.w $01D2                 ; $82E563
CODE_82E566:
    lda.w #$0000                ; $82E566
    sta.l $308000               ; $82E569
    lda.w $02D7                 ; $82E56D
    and.w #$FF00                ; $82E570
    lsr a                       ; $82E573
    sta.l $308000               ; $82E574
    lda.w $406E,x               ; $82E578
    asl a                       ; $82E57B
    sta.l $308000               ; $82E57C
    nop                         ; $82E580
    lda.l $308000               ; $82E581
    clc                         ; $82E585
    adc.b $14                   ; $82E586
    lda.l $308000               ; $82E588
    clc                         ; $82E58C
    adc.w $01D2                 ; $82E58D
    sta.w $01D2                 ; $82E590
    bra CODE_82E5BF             ; $82E593
CODE_82E595:
    lda.w #$0000                ; $82E595
    sta.l $308000               ; $82E598
    lda.w $02D7                 ; $82E59C
    and.w #$FF00                ; $82E59F
    sec                         ; $82E5A2
    sbc.w $02EF                 ; $82E5A3
    lsr a                       ; $82E5A6
    sta.l $308000               ; $82E5A7
    lda.w $406E,x               ; $82E5AB
    asl a                       ; $82E5AE
    sta.l $308000               ; $82E5AF
    nop                         ; $82E5B3
    lda.l $308000               ; $82E5B4
    lda.l $308000               ; $82E5B8
    sta.w $01D2                 ; $82E5BC
CODE_82E5BF:
    lda.w $02F1                 ; $82E5BF
    clc                         ; $82E5C2
    adc.w #$0020                ; $82E5C3
    clc                         ; $82E5C6
    adc.w $1EE8                 ; $82E5C7
    cmp.w $019A                 ; $82E5CA
    bcc CODE_82E5D2             ; $82E5CD
    sbc.w $019A                 ; $82E5CF
CODE_82E5D2:
    sta.b $16                   ; $82E5D2
    lda.w $02D7                 ; $82E5D4
    and.w #$FF00                ; $82E5D7
    sta.w $02EF                 ; $82E5DA
    lda.w $02D9                 ; $82E5DD
    sec                         ; $82E5E0
    sbc.w $02F1                 ; $82E5E1
    bne CODE_82E5E9             ; $82E5E4
    jmp.w $E9DD                 ; $82E5E6
CODE_82E5E9:
    sta.b $14                   ; $82E5E9
    clc                         ; $82E5EB
    adc.w $02F3                 ; $82E5EC
    sta.w $02F3                 ; $82E5EF
    lda.w $1EE2                 ; $82E5F2
    sta.b $22                   ; $82E5F5
    lda.w $02F1                 ; $82E5F7
    asl a                       ; $82E5FA
    and.w #$003E                ; $82E5FB
    tax                         ; $82E5FE
    lda.l $7E43EE,x             ; $82E5FF
    sec                         ; $82E603
    sbc.l $7E43EC,x             ; $82E604
    sta.w $1F02                 ; $82E608
    lda.l $7E436E,x             ; $82E60B
    sec                         ; $82E60F
    sbc.l $7E436C,x             ; $82E610
    sta.w $1F04                 ; $82E614
    lda.b $16                   ; $82E617
    cmp.b $22                   ; $82E619
    bcc CODE_82E628             ; $82E61B
    clc                         ; $82E61D
    adc.w $1EE4                 ; $82E61E
    sta.b $16                   ; $82E621
    lda.w #$7FFF                ; $82E623
    sta.b $22                   ; $82E626
CODE_82E628:
    lda.w #$0000                ; $82E628
    sta.l $308000               ; $82E62B
    lda.b $16                   ; $82E62F
    asl a                       ; $82E631
    asl a                       ; $82E632
    xba                         ; $82E633
    and.w #$00FF                ; $82E634
    sta.l $308000               ; $82E637
    lda.w #$000A                ; $82E63B
    sta.l $308000               ; $82E63E
    nop                         ; $82E642
    lda.l $308000               ; $82E643
    clc                         ; $82E647
    adc.b $98                   ; $82E648
    sta.b $18                   ; $82E64A
    tax                         ; $82E64C
    lda.l $308000               ; $82E64D
    lda.b $16                   ; $82E651
    asl a                       ; $82E653
    and.w #$003E                ; $82E654
    tay                         ; $82E657
    sta.b $1A                   ; $82E658
    lda.b $16                   ; $82E65A
    and.w #$003E                ; $82E65C
    sta.b $1C                   ; $82E65F
    adc.l $840000,x             ; $82E661
    tax                         ; $82E665
    lda.l $850000,x             ; $82E666
    sta.w $406C,y               ; $82E66A
    sta.w $40AC,y               ; $82E66D
    ldx.b $18                   ; $82E670
    lda.b $1C                   ; $82E672
    clc                         ; $82E674
    adc.l $840002,x             ; $82E675
    tax                         ; $82E679
    lda.l $850000,x             ; $82E67A
    sec                         ; $82E67E
    sbc.w $1E20                 ; $82E67F
    sta.w $40EC,y               ; $82E682
    sta.w $412C,y               ; $82E685
    tya                         ; $82E688
    and.w #$0006                ; $82E689
    cmp.w #$0006                ; $82E68C
    bne CODE_82E698             ; $82E68F
    lda.l $850000,x             ; $82E691
    sta.w $1E20                 ; $82E695
CODE_82E698:
    ldx.b $18                   ; $82E698
    tya                         ; $82E69A
    bit.w #$0002                ; $82E69B
    beq CODE_82E6A7             ; $82E69E
    lda.b $1C                   ; $82E6A0
    ora.w #$0001                ; $82E6A2
    bra CODE_82E6A9             ; $82E6A5
CODE_82E6A7:
    lda.b $1C                   ; $82E6A7
CODE_82E6A9:
    asl a                       ; $82E6A9
    bit.w #$0040                ; $82E6AA
    bne CODE_82E6B5             ; $82E6AD
    adc.l $840006,x             ; $82E6AF
    bra CODE_82E6BC             ; $82E6B3
CODE_82E6B5:
    and.w #$003F                ; $82E6B5
    adc.l $840008,x             ; $82E6B8
CODE_82E6BC:
    tax                         ; $82E6BC
    lda.l $7E5391,x             ; $82E6BD
    sta.w $426C,y               ; $82E6C1
    sta.w $42AC,y               ; $82E6C4
    ldx.b $18                   ; $82E6C7
    lda.l $840004,x             ; $82E6C9
    tax                         ; $82E6CD
    lda.l $850000,x             ; $82E6CE
    tay                         ; $82E6D2
    lda.l $850002,x             ; $82E6D3
    cmp.b $1A                   ; $82E6D7
    bcs CODE_82E6F0             ; $82E6D9
    incbin !baserom:$166DB..$166F0
CODE_82E6F0:
    tya                         ; $82E6F0
    ldy.b $1A                   ; $82E6F1
    sta.w $416C,y               ; $82E6F3
    sta.w $41AC,y               ; $82E6F6
    lda.w #$0000                ; $82E6F9
    sta.w $42EC,y               ; $82E6FC
    sta.w $432C,y               ; $82E6FF
    lda.b $9C                   ; $82E702
    clc                         ; $82E704
    adc.b $A6                   ; $82E705
    tax                         ; $82E707
    lda.b $16                   ; $82E708
    sec                         ; $82E70A
    sbc.l $840000,x             ; $82E70B
    bcs CODE_82E729             ; $82E70F
    lda.b $16                   ; $82E711
    inc a                       ; $82E713
    cmp.b $22                   ; $82E714
    bne CODE_82E726             ; $82E716
    bit.w $1E68                 ; $82E718
    bpl CODE_82E726             ; $82E71B
    lda.w #$8002                ; $82E71D
    sta.w $42EC,y               ; $82E720
    sta.w $432C,y               ; $82E723
CODE_82E726:
    jmp.w $E819                 ; $82E726
CODE_82E729:
    cmp.w #$0040                ; $82E729
    bcc CODE_82E76E             ; $82E72C
    lda.w #$0008                ; $82E72E
    ora.w $42EC,y               ; $82E731
    sta.w $42EC,y               ; $82E734
    sta.w $432C,y               ; $82E737
    lda.w $426A,y               ; $82E73A
    sta.w $456C,y               ; $82E73D
    sta.w $45AC,y               ; $82E740
    lda.b $A6                   ; $82E743
    clc                         ; $82E745
    adc.w #$0014                ; $82E746
    sta.b $A6                   ; $82E749
    bit.w $1E68                 ; $82E74B
    bmi CODE_82E75C             ; $82E74E
    lda.l $840010,x             ; $82E750
    sta.w $1EE8                 ; $82E754
    clc                         ; $82E757
    adc.b $16                   ; $82E758
    sta.b $16                   ; $82E75A
CODE_82E75C:
    lda.b $9C                   ; $82E75C
    clc                         ; $82E75E
    adc.b $A6                   ; $82E75F
    tax                         ; $82E761
    lda.b $16                   ; $82E762
    sec                         ; $82E764
    sbc.l $840000,x             ; $82E765
    bcs CODE_82E729             ; $82E769
    jmp.w $E82B                 ; $82E76B
CODE_82E76E:
    cmp.w #$003E                ; $82E76E
    bne CODE_82E784             ; $82E771
    bit.w $1E68                 ; $82E773
    bpl CODE_82E784             ; $82E776
    lda.w #$8001                ; $82E778
    sta.w $42EC,y               ; $82E77B
    sta.w $432C,y               ; $82E77E
    lda.w #$003E                ; $82E781
CODE_82E784:
    asl a                       ; $82E784
    sta.b $1E                   ; $82E785
    stx.b $20                   ; $82E787
    clc                         ; $82E789
    adc.l $840004,x             ; $82E78A
    tax                         ; $82E78E
    lda.l $850000,x             ; $82E78F
    sta.w $436C,y               ; $82E793
    sta.w $43AC,y               ; $82E796
    ldx.b $20                   ; $82E799
    lda.b $1E                   ; $82E79B
    clc                         ; $82E79D
    adc.l $840006,x             ; $82E79E
    tax                         ; $82E7A2
    lda.l $850000,x             ; $82E7A3
    sta.w $43EC,y               ; $82E7A7
    sta.w $442C,y               ; $82E7AA
    ldx.b $20                   ; $82E7AD
    lda.l $84000A,x             ; $82E7AF
    tax                         ; $82E7B3
    jmp.w ($E7B7,x)             ; $82E7B4
    incbin !baserom:$167B7..$167CF
    lda.w $436C,y               ; $82E7CF
    bpl CODE_82E7E7             ; $82E7D2
    incbin !baserom:$167D4..$167E7
CODE_82E7E7:
    lsr a                       ; $82E7E7
    sta.w $44EC,y               ; $82E7E8
    sta.w $452C,y               ; $82E7EB
    lda.w #$C001                ; $82E7EE
    sta.w $41EC,y               ; $82E7F1
    sta.w $422C,y               ; $82E7F4
    bra CODE_82E7F9             ; $82E7F7
CODE_82E7F9:
    ldx.b $20                   ; $82E7F9
    lda.b $1E                   ; $82E7FB
    cmp.w #$0040                ; $82E7FD
    bcc CODE_82E807             ; $82E800
    sbc.w #$0040                ; $82E802
    inx                         ; $82E805
    inx                         ; $82E806
CODE_82E807:
    clc                         ; $82E807
    adc.l $84000C,x             ; $82E808
    tax                         ; $82E80C
    lda.l $7E5391,x             ; $82E80D
    sta.w $456C,y               ; $82E811
    sta.w $45AC,y               ; $82E814
    bra CODE_82E84C             ; $82E817
    lda.b $16                   ; $82E819
    and.w #$0007                ; $82E81B
    asl a                       ; $82E81E
    asl a                       ; $82E81F
    asl a                       ; $82E820
    clc                         ; $82E821
    adc.w $1F0C                 ; $82E822
    sta.w $456C,y               ; $82E825
    sta.w $45AC,y               ; $82E828
    lda.w #$0000                ; $82E82B
    sta.w $436C,y               ; $82E82E
    sta.w $43AC,y               ; $82E831
    sta.w $43EC,y               ; $82E834
    sta.w $442C,y               ; $82E837
    lda.w $1F12                 ; $82E83A
    sta.w $41EC,y               ; $82E83D
    sta.w $422C,y               ; $82E840
    lda.w $1F14                 ; $82E843
    sta.w $44EC,y               ; $82E846
    sta.w $452C,y               ; $82E849
CODE_82E84C:
    lda.b $16                   ; $82E84C
    asl a                       ; $82E84E
    asl a                       ; $82E84F
    tax                         ; $82E850
    tay                         ; $82E851
    lda.l $7E8CB4,x             ; $82E852
    and.w #$00FF                ; $82E856
    beq CODE_82E860             ; $82E859
    asl a                       ; $82E85B
    tax                         ; $82E85C
    jsr.w ($826E,x)             ; $82E85D
CODE_82E860:
    inc.b $16                   ; $82E860
    lda.b $16                   ; $82E862
    cmp.w $019A                 ; $82E864
    bne CODE_82E86D             ; $82E867
    stz.b $16                   ; $82E869
    stz.b $A6                   ; $82E86B
CODE_82E86D:
    dec.b $14                   ; $82E86D
    beq CODE_82E874             ; $82E86F
    jmp.w $E617                 ; $82E871
CODE_82E874:
    pea.w $837E                 ; $82E874
    plb                         ; $82E877
    ldx.b $A4                   ; $82E878
    lda.w $02D9                 ; $82E87A
    cmp.l $7E5B91,x             ; $82E87D
    bcs CODE_82E886             ; $82E881
    jmp.w $E985                 ; $82E883
CODE_82E886:
    ldy.w $0100                 ; $82E886
    lda.l $7E5B94,x             ; $82E889
    db $F0,$6B ; beq            ; $82E88D
    bit.w #$FF00                ; $82E88F
    bne CODE_82E8FD             ; $82E892
    asl a                       ; $82E894
    tax                         ; $82E895
    lda.l $86AE36,x             ; $82E896
    tax                         ; $82E89A
    lda.l $86001E,x             ; $82E89B
    sta.w $F533,y               ; $82E89F
    lda.l $860020,x             ; $82E8A2
    sta.w $F433,y               ; $82E8A6
    lda.w #$017F                ; $82E8A9
    sta.w $F733,y               ; $82E8AC
    lda.l $860000,x             ; $82E8AF
    tax                         ; $82E8B3
    lda.l $7E6591,x             ; $82E8B4
    clc                         ; $82E8B8
    adc.w $F533,y               ; $82E8B9
    sta.w $F533,y               ; $82E8BC
    ldx.b $A4                   ; $82E8BF
    lda.l $7E5B93,x             ; $82E8C1
    and.w #$00FF                ; $82E8C5
    xba                         ; $82E8C8
    lsr a                       ; $82E8C9
    lsr a                       ; $82E8CA
    lsr a                       ; $82E8CB
    adc.w #$1000                ; $82E8CC
    sta.w $F633,y               ; $82E8CF
    lda.l $7E5B93,x             ; $82E8D2
    and.w #$00FF                ; $82E8D6
    asl a                       ; $82E8D9
    sta.w $F933,y               ; $82E8DA
    lda.l $7E5B93,x             ; $82E8DD
    xba                         ; $82E8E1
    and.w #$00FF                ; $82E8E2
    asl a                       ; $82E8E5
    clc                         ; $82E8E6
    adc.w #$46C8                ; $82E8E7
    sta.w $F833,y               ; $82E8EA
    jsl.l func_85CDAF           ; $82E8ED
    iny                         ; $82E8F1
    iny                         ; $82E8F2
    tya                         ; $82E8F3
    and.w #$00FF                ; $82E8F4
    tay                         ; $82E8F7
    bra CODE_82E96F             ; $82E8F8
    incbin !baserom:$168FA..$168FD
CODE_82E8FD:
    asl a                       ; $82E8FD
    tax                         ; $82E8FE
    lda.l $86AC36,x             ; $82E8FF
    tax                         ; $82E903
    lda.l $860022,x             ; $82E904
    sta.w $F533,y               ; $82E908
    lda.l $860024,x             ; $82E90B
    sta.w $F433,y               ; $82E90F
    lda.w #$017F                ; $82E912
    sta.w $F733,y               ; $82E915
    lda.l $860000,x             ; $82E918
    tax                         ; $82E91C
    lda.l $7E6591,x             ; $82E91D
    clc                         ; $82E921
    adc.w $F533,y               ; $82E922
    sta.w $F533,y               ; $82E925
    ldx.b $A4                   ; $82E928
    lda.l $7E5B93,x             ; $82E92A
    and.w #$00FF                ; $82E92E
    xba                         ; $82E931
    lsr a                       ; $82E932
    lsr a                       ; $82E933
    lsr a                       ; $82E934
    adc.w #$1000                ; $82E935
    sta.w $F633,y               ; $82E938
    lda.l $7E5B93,x             ; $82E93B
    and.w #$00FF                ; $82E93F
    asl a                       ; $82E942
    sta.w $F933,y               ; $82E943
    lda.l $7E5B93,x             ; $82E946
    xba                         ; $82E94A
    and.w #$00FF                ; $82E94B
    asl a                       ; $82E94E
    clc                         ; $82E94F
    adc.w #$4A38                ; $82E950
    sta.w $F833,y               ; $82E953
    jsl.l func_85CDAF           ; $82E956
    iny                         ; $82E95A
    iny                         ; $82E95B
    tya                         ; $82E95C
    and.w #$00FF                ; $82E95D
    tay                         ; $82E960
    bra CODE_82E96F             ; $82E961
    incbin !baserom:$16963..$1696F
CODE_82E96F:
    inx                         ; $82E96F
    inx                         ; $82E970
    inx                         ; $82E971
    inx                         ; $82E972
    inx                         ; $82E973
    stx.b $A4                   ; $82E974
    lda.w $02D9                 ; $82E976
    cmp.l $7E5B91,x             ; $82E979
    bcc CODE_82E982             ; $82E97D
    jmp.w $E889                 ; $82E97F
CODE_82E982:
    sty.w $0100                 ; $82E982
    plb                         ; $82E985
    lda.w $02D9                 ; $82E986
    cmp.w $019A                 ; $82E989
    bcc CODE_82E9A9             ; $82E98C
    sbc.w $019A                 ; $82E98E
    sta.w $02D9                 ; $82E991
    sta.w $02F1                 ; $82E994
    stz.b $A4                   ; $82E997
    stz.w $1E62                 ; $82E999
    stz.w $1E64                 ; $82E99C
    lda.w #$0140                ; $82E99F
    sta.l $7E46C8               ; $82E9A2
    jmp.w $E874                 ; $82E9A6
CODE_82E9A9:
    sta.w $02D9                 ; $82E9A9
    sta.w $02F1                 ; $82E9AC
    stz.w $0313                 ; $82E9AF
    ldx.b $9C                   ; $82E9B2
CODE_82E9B4:
    lda.w $02D9                 ; $82E9B4
    inc a                       ; $82E9B7
    inc a                       ; $82E9B8
    sec                         ; $82E9B9
    sbc.l $840000,x             ; $82E9BA
    bcc CODE_82E9D4             ; $82E9BE
    cmp.w #$0041                ; $82E9C0
    bcc CODE_82E9CD             ; $82E9C3
    txa                         ; $82E9C5
    clc                         ; $82E9C6
    adc.w #$0014                ; $82E9C7
    tax                         ; $82E9CA
    bra CODE_82E9B4             ; $82E9CB
CODE_82E9CD:
    lda.l $840002,x             ; $82E9CD
    sta.w $0313                 ; $82E9D1
CODE_82E9D4:
    lda.w $0313                 ; $82E9D4
    and.w #$00FF                ; $82E9D7
    sta.w $0144                 ; $82E9DA
    lda.w $02D9                 ; $82E9DD
    clc                         ; $82E9E0
    adc.w #$0001                ; $82E9E1
    cmp.w $019A                 ; $82E9E4
    bcc CODE_82E9EC             ; $82E9E7
    sbc.w $019A                 ; $82E9E9
CODE_82E9EC:
    jsr.w func_82EA49           ; $82E9EC
    sta.b $14                   ; $82E9EF
    lda.w $02D9                 ; $82E9F1
    sec                         ; $82E9F4
    sbc.w #$0003                ; $82E9F5
    bcs CODE_82E9FD             ; $82E9F8
    adc.w $019A                 ; $82E9FA
CODE_82E9FD:
    jsr.w func_82EA49           ; $82E9FD
    sta.b $16                   ; $82EA00
    lda.w #$0000                ; $82EA02
    sta.l $308000               ; $82EA05
    lda.b $14                   ; $82EA09
    sec                         ; $82EA0B
    sbc.b $16                   ; $82EA0C
    sta.l $308000               ; $82EA0E
    lda.w $02D8                 ; $82EA12
    clc                         ; $82EA15
    adc.w #$0100                ; $82EA16
    and.w #$03FF                ; $82EA19
    sta.l $308000               ; $82EA1C
    lda.b $16                   ; $82EA20
    cmp.w #$8000                ; $82EA22
    rol a                       ; $82EA25
    rol a                       ; $82EA26
    xba                         ; $82EA27
    and.w #$03FF                ; $82EA28
    bit.w #$0080                ; $82EA2B
    beq CODE_82EA33             ; $82EA2E
    ora.w #$FF00                ; $82EA30
CODE_82EA33:
    sta.l $308000               ; $82EA33
    clc                         ; $82EA37
    adc.l $308000               ; $82EA38
    clc                         ; $82EA3C
    adc.w #$0009                ; $82EA3D
    eor.w #$FFFF                ; $82EA40
    inc a                       ; $82EA43
    sta.w $02EA                 ; $82EA44
    plb                         ; $82EA47
    rts                         ; $82EA48
func_82EA49:
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
track_event_none:
    rts                         ; $82EA80
track_event_2007:
    lda.w #$2007                ; $82EA81
    jmp.w track_event_tail      ; $82EA84
track_event_2047:
    lda.w #$2047                ; $82EA87
    jmp.w track_event_tail      ; $82EA8A
track_event_2087:
    lda.w #$2087                ; $82EA8D
    jmp.w track_event_tail      ; $82EA90
track_event_20c7:
    lda.w #$20C7                ; $82EA93
    sta.w $1F0A                 ; $82EA96
    tyx                         ; $82EA99
    lda.l $7E8CB3,x             ; $82EA9A
    and.w #$00FF                ; $82EA9E
    asl a                       ; $82EAA1
    asl a                       ; $82EAA2
    asl a                       ; $82EAA3
    asl a                       ; $82EAA4
    bit.w #$0800                ; $82EAA5
    beq CODE_82EAAD             ; $82EAA8
    ora.w #$F000                ; $82EAAA
CODE_82EAAD:
    ldx.b $1A                   ; $82EAAD
    sta.w $1F0E                 ; $82EAAF
    sta.w $3C6C,x               ; $82EAB2
    sta.w $3CAC,x               ; $82EAB5
    sta.w $1F10                 ; $82EAB8
    sta.w $3F6C,x               ; $82EABB
    sta.w $3FAC,x               ; $82EABE
    lda.w #$0001                ; $82EAC1
    ora.w $3D6C,x               ; $82EAC4
    sta.w $3D6C,x               ; $82EAC7
    sta.w $3DAC,x               ; $82EACA
    rts                         ; $82EACD
track_event_2107:
    lda.w #$2107                ; $82EACE
    jmp.w track_event_tail      ; $82EAD1
track_event_c2e7:
    lda.w #$C2E7                ; $82EAD4
    jmp.w track_event_tail      ; $82EAD7
track_event_clear:
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
track_event_place_only:
    tyx                         ; $82EAFE
    jsr.w func_82EC09           ; $82EAFF
    ldx.b $1A                   ; $82EB02
    sta.w $1F0E                 ; $82EB04
    sta.w $3C6C,x               ; $82EB07
    sta.w $3CAC,x               ; $82EB0A
    clc                         ; $82EB0D
    adc.b $1E                   ; $82EB0E
    sta.w $1F10                 ; $82EB10
    sta.w $3F6C,x               ; $82EB13
    sta.w $3FAC,x               ; $82EB16
    rts                         ; $82EB19
track_event_tail:
    sta.w $1F0A                 ; $82EB1A
    tyx                         ; $82EB1D
    jsr.w func_82EC09           ; $82EB1E
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
track_event2_set:
    lda.w #$2007                ; $82EB45
    jmp.w $EBDE                 ; $82EB48
    lda.w #$2047                ; $82EB4B
    jmp.w $EBDE                 ; $82EB4E
    lda.w #$2087                ; $82EB51
    jmp.w $EBDE                 ; $82EB54
    lda.w #$C2E7                ; $82EB57
    jmp.w $EBDE                 ; $82EB5A
    lda.w #$20C7                ; $82EB5D
    sta.w $1F0C                 ; $82EB60
    tyx                         ; $82EB63
    lda.l $7E8CB3,x             ; $82EB64
    and.w #$00FF                ; $82EB68
    asl a                       ; $82EB6B
    asl a                       ; $82EB6C
    asl a                       ; $82EB6D
    asl a                       ; $82EB6E
    bit.w #$0800                ; $82EB6F
    beq CODE_82EB77             ; $82EB72
    incbin !baserom:$16B74..$16B77
CODE_82EB77:
    ldx.b $1A                   ; $82EB77
    sta.w $1F12                 ; $82EB79
    sta.w $41EC,x               ; $82EB7C
    sta.w $422C,x               ; $82EB7F
    sta.w $1F14                 ; $82EB82
    sta.w $44EC,x               ; $82EB85
    sta.w $452C,x               ; $82EB88
    lda.w #$0001                ; $82EB8B
    ora.w $42EC,x               ; $82EB8E
    sta.w $42EC,x               ; $82EB91
    sta.w $432C,x               ; $82EB94
    rts                         ; $82EB97
    lda.w #$2107                ; $82EB98
    jmp.w $EBDE                 ; $82EB9B
    ldx.b $1A                   ; $82EB9E
    lda.w #$0000                ; $82EBA0
    sta.w $1F12                 ; $82EBA3
    sta.w $41EC,x               ; $82EBA6
    sta.w $422C,x               ; $82EBA9
    sta.w $1F14                 ; $82EBAC
    sta.w $44EC,x               ; $82EBAF
    sta.w $452C,x               ; $82EBB2
    lda.w #$0001                ; $82EBB5
    ora.w $42EC,x               ; $82EBB8
    sta.w $42EC,x               ; $82EBBB
    sta.w $432C,x               ; $82EBBE
    rts                         ; $82EBC1
    tyx                         ; $82EBC2
    jsr.w func_82EC09           ; $82EBC3
    ldx.b $1A                   ; $82EBC6
    sta.w $1F12                 ; $82EBC8
    sta.w $41EC,x               ; $82EBCB
    sta.w $422C,x               ; $82EBCE
    clc                         ; $82EBD1
    adc.b $1E                   ; $82EBD2
    sta.w $1F14                 ; $82EBD4
    sta.w $44EC,x               ; $82EBD7
    sta.w $452C,x               ; $82EBDA
    rts                         ; $82EBDD
    sta.w $1F0C                 ; $82EBDE
    tyx                         ; $82EBE1
    jsr.w func_82EC09           ; $82EBE2
    ldx.b $1A                   ; $82EBE5
    sta.w $1F12                 ; $82EBE7
    sta.w $41EC,x               ; $82EBEA
    sta.w $422C,x               ; $82EBED
    clc                         ; $82EBF0
    adc.b $1E                   ; $82EBF1
    sta.w $1F14                 ; $82EBF3
    sta.w $44EC,x               ; $82EBF6
    sta.w $452C,x               ; $82EBF9
    lda.w #$0001                ; $82EBFC
    ora.w $42EC,x               ; $82EBFF
    sta.w $42EC,x               ; $82EC02
    sta.w $432C,x               ; $82EC05
    rts                         ; $82EC08
func_82EC09:
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
    ora.w #$F000                ; $82EC22
CODE_82EC25:
    sec                         ; $82EC25
    sbc.b $1E                   ; $82EC26
    asl.b $1E                   ; $82EC28
    rts                         ; $82EC2A
    jsr.w road_shift_frame      ; $82EC2B
    jsr.w $F3D4                 ; $82EC2E
    rts                         ; $82EC31
road_shift_frame_call:
    jsr.w road_shift_frame      ; $82EC32
    rts                         ; $82EC35
road_shift_frame:
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
    bmi CODE_82EC75             ; $82EC72
    dec a                       ; $82EC74
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
race_clear_0242_0208:
    stz.w $0242                 ; $82ECEA
    stz.w $0208                 ; $82ECED
    rts                         ; $82ECF0
race_scroll_commit:
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
    incbin !baserom:$16D17..$16D27
road_edges_side:
    ldx.b $16                   ; $82ED27 static
    stz.w $8000                 ; $82ED29 static
    lda.l $7E3C6E,x             ; $82ED2C static
    sec                         ; $82ED30 static
    sbc.l $7E3C6C,x             ; $82ED31 static
    asl a                       ; $82ED35 static
    sta.w $8000                 ; $82ED36 static
    lda.b $14                   ; $82ED39 static
    sta.w $8000                 ; $82ED3B static
    nop                         ; $82ED3E static
    lda.w $8000                 ; $82ED3F static
    lda.w $8000                 ; $82ED42 static
    clc                         ; $82ED45 static
    adc.l $7E3C6C,x             ; $82ED46 static
    eor.w #$FFFF                ; $82ED4A static
    sec                         ; $82ED4D static
    adc.w $1EF6                 ; $82ED4E static
    sta.b $1C                   ; $82ED51 static
    stz.w $8000                 ; $82ED53 static
    lda.l $7E3F6E,x             ; $82ED56 static
    sec                         ; $82ED5A static
    sbc.l $7E3F6C,x             ; $82ED5B static
    asl a                       ; $82ED5F static
    sta.w $8000                 ; $82ED60 static
    lda.b $14                   ; $82ED63 static
    sta.w $8000                 ; $82ED65 static
    nop                         ; $82ED68 static
    lda.w $8000                 ; $82ED69 static
    lda.w $8000                 ; $82ED6C static
    clc                         ; $82ED6F static
    adc.l $7E3F6C,x             ; $82ED70 static
    eor.w #$FFFF                ; $82ED74 static
    sec                         ; $82ED77 static
    adc.w $1EF6                 ; $82ED78 static
    sta.b $1E                   ; $82ED7B static
    lda.w $0202                 ; $82ED7D static
    eor.w #$FFFF                ; $82ED80 static
    inc a                       ; $82ED83 static
    cmp.b $1C                   ; $82ED84 static
    bpl CODE_82ED8C             ; $82ED86 static
    cmp.b $1E                   ; $82ED88 static
    bpl CODE_82ED95             ; $82ED8A static
CODE_82ED8C:
    stz.w $0242                 ; $82ED8C static
    stz.w $0208                 ; $82ED8F static
    stz.b $3C                   ; $82ED92 static
    rts                         ; $82ED94 static
CODE_82ED95:
    lda.w #$0000                ; $82ED95 static
    sta.l $308000               ; $82ED98 static
    lda.l $7E3E6E,x             ; $82ED9C static
    sec                         ; $82EDA0 static
    sbc.l $7E3E6C,x             ; $82EDA1 static
    asl a                       ; $82EDA5 static
    sta.b $18                   ; $82EDA6 static
    sta.l $308000               ; $82EDA8 static
    lda.b $14                   ; $82EDAC static
    sta.l $308000               ; $82EDAE static
    nop                         ; $82EDB2 static
    lda.l $308000               ; $82EDB3 static
    lda.l $308000               ; $82EDB7 static
    clc                         ; $82EDBB static
    adc.l $7E3E6C,x             ; $82EDBC static
    eor.w #$FFFF                ; $82EDC0 static
    inc a                       ; $82EDC3 static
    sta.w $0242                 ; $82EDC4 static
    lda.w #$0000                ; $82EDC7 static
    sta.l $308000               ; $82EDCA static
    lda.w $1EF8                 ; $82EDCE static
    asl a                       ; $82EDD1 static
    asl a                       ; $82EDD2 static
    sta.l $308000               ; $82EDD3 static
    lda.w #$8000                ; $82EDD7 static
    sec                         ; $82EDDA static
    sbc.b $14                   ; $82EDDB static
    lsr a                       ; $82EDDD static
    sta.l $308000               ; $82EDDE static
    nop                         ; $82EDE2 static
    lda.l $308000               ; $82EDE3 static
    sta.b $1A                   ; $82EDE7 static
    lda.l $308000               ; $82EDE9 static
    sta.b $1C                   ; $82EDED static
    lda.w #$0000                ; $82EDEF static
    sta.l $308000               ; $82EDF2 static
    lda.l $7E3E6E,x             ; $82EDF6 static
    sec                         ; $82EDFA static
    sbc.l $7E3E6C,x             ; $82EDFB static
    asl a                       ; $82EDFF static
    sta.l $308000               ; $82EE00 static
    lda.b $14                   ; $82EE04 static
    sta.l $308000               ; $82EE06 static
    lda.b $1A                   ; $82EE0A static
    clc                         ; $82EE0C static
    adc.l $308000               ; $82EE0D static
    sta.b $1A                   ; $82EE11 static
    lda.b $1C                   ; $82EE13 static
    adc.l $308000               ; $82EE15 static
    sta.b $1C                   ; $82EE19 static
    lda.b $1B                   ; $82EE1B static
    eor.w #$FFFF                ; $82EE1D static
    inc a                       ; $82EE20 static
    sta.w $0208                 ; $82EE21 static
    lda.w #$0001                ; $82EE24 static
    sta.b $3C                   ; $82EE27 static
    rts                         ; $82EE29 static
road_edge_shift:
    ldx.b $16                   ; $82EE2A static
    lda.b $3C                   ; $82EE2C static
    bne CODE_82EE50             ; $82EE2E static
    stz.w $020A                 ; $82EE30 static
    stz.w $020C                 ; $82EE33 static
    lda.w $0204                 ; $82EE36 static
    sta.w $0200                 ; $82EE39 static
    clc                         ; $82EE3C static
    adc.w $1EF4                 ; $82EE3D static
    sta.w $1EF4                 ; $82EE40 static
    lda.w $0206                 ; $82EE43 static
    sta.w $0202                 ; $82EE46 static
    adc.w $1EF6                 ; $82EE49 static
    sta.w $1EF6                 ; $82EE4C static
    rts                         ; $82EE4F static
CODE_82EE50:
    lda.w #$0000                ; $82EE50 static
    sta.l $308000               ; $82EE53 static
    lda.w $1EFA                 ; $82EE57 static
    asl a                       ; $82EE5A static
    asl a                       ; $82EE5B static
    sta.l $308000               ; $82EE5C static
    lda.w #$8000                ; $82EE60 static
    sec                         ; $82EE63 static
    sbc.b $14                   ; $82EE64 static
    lsr a                       ; $82EE66 static
    sta.l $308000               ; $82EE67 static
    nop                         ; $82EE6B static
    lda.l $308000               ; $82EE6C static
    sta.b $1A                   ; $82EE70 static
    lda.l $308000               ; $82EE72 static
    sta.w $020C                 ; $82EE76 static
    stz.w $020A                 ; $82EE79 static
    lda.w #$0000                ; $82EE7C static
    sta.l $308000               ; $82EE7F static
    lda.l $7E3DEE,x             ; $82EE83 static
    sec                         ; $82EE87 static
    sbc.l $7E3DEC,x             ; $82EE88 static
    asl a                       ; $82EE8C static
    sta.l $308000               ; $82EE8D static
    lda.b $14                   ; $82EE91 static
    sta.l $308000               ; $82EE93 static
    lda.b $1A                   ; $82EE97 static
    clc                         ; $82EE99 static
    adc.l $308000               ; $82EE9A static
    lda.l $308000               ; $82EE9E static
    adc.w #$0000                ; $82EEA2 static
    clc                         ; $82EEA5 static
    adc.w $020C                 ; $82EEA6 static
    eor.w #$FFFF                ; $82EEA9 static
    inc a                       ; $82EEAC static
    sta.w $020C                 ; $82EEAD static
    stz.w $1EEE                 ; $82EEB0 static
    stz.w $1EF0                 ; $82EEB3 static
    rts                         ; $82EEB6 static
road_shift_by_place:
    lda.w $0226                 ; $82EEB7
    and.w #$0020                ; $82EEBA
    beq CODE_82EED7             ; $82EEBD
    lda.w $1E66                 ; $82EEBF
    pha                         ; $82EEC2
    and.w #$7FFF                ; $82EEC3
    lsr a                       ; $82EEC6
    bcc CODE_82EECC             ; $82EEC7
    ora.w #$8000                ; $82EEC9
CODE_82EECC:
    sta.w $1E66                 ; $82EECC
    jsr.w road_shift_apply      ; $82EECF
    pla                         ; $82EED2
    sta.w $1E66                 ; $82EED3
    rts                         ; $82EED6
CODE_82EED7:
    lda.w $1E66                 ; $82EED7
    and.w #$FFFE                ; $82EEDA
    pha                         ; $82EEDD
    jsr.w road_shift_settle     ; $82EEDE
    lda.w $1E66                 ; $82EEE1
    asl a                       ; $82EEE4
    lda.w #$0000                ; $82EEE5
    rol a                       ; $82EEE8
    ora.b $01,s                 ; $82EEE9
    sta.w $1E66                 ; $82EEEB
    pla                         ; $82EEEE
    rts                         ; $82EEEF
    incbin !baserom:$16EF0..$16FCC
    lda.w $0226                 ; $82EFCC
    and.w #$003E                ; $82EFCF
    cmp.w #$0018                ; $82EFD2
    bcc road_shift_settle       ; $82EFD5
    jmp.w road_shift_apply      ; $82EFD7
road_shift_settle:
    lda.w $020B                 ; $82EFDA
    sta.b $22                   ; $82EFDD
    ldx.b $16                   ; $82EFDF
    stz.w $8000                 ; $82EFE1
    lda.l $7E3DEE,x             ; $82EFE4
    sec                         ; $82EFE8
    sbc.l $7E3DEC,x             ; $82EFE9
    asl a                       ; $82EFED
    asl a                       ; $82EFEE
    sta.w $8000                 ; $82EFEF
    lda.w #$8000                ; $82EFF2
    sec                         ; $82EFF5
    sbc.b $14                   ; $82EFF6
    lsr a                       ; $82EFF8
    sta.w $8000                 ; $82EFF9
    nop                         ; $82EFFC
    lda.w $8000                 ; $82EFFD
    sta.b $1A                   ; $82F000
    lda.w $8000                 ; $82F002
    sta.b $1C                   ; $82F005
    stz.w $8000                 ; $82F007
    lda.l $7E3D6C,x             ; $82F00A
    bpl CODE_82F033             ; $82F00E
    bit.w #$0002                ; $82F010 static
    bne CODE_82F033             ; $82F013 static
    lda.l $7E3AEF,x             ; $82F015 static
    and.w #$00FF                ; $82F019 static
    bit.w #$0080                ; $82F01C static
    beq CODE_82F024             ; $82F01F static
    ora.w #$FF00                ; $82F021 static
CODE_82F024:
    eor.w #$FFFF                ; $82F024 static
    sec                         ; $82F027 static
    adc.l $7E3DEE,x             ; $82F028 static
    sec                         ; $82F02C static
    sbc.l $7E3DEC,x             ; $82F02D static
    bra CODE_82F03C             ; $82F031 static
CODE_82F033:
    lda.l $7E3DF0,x             ; $82F033
    sec                         ; $82F037
    sbc.l $7E3DEE,x             ; $82F038
CODE_82F03C:
    asl a                       ; $82F03C
    sta.w $8000                 ; $82F03D
    lda.b $14                   ; $82F040
    sta.w $8000                 ; $82F042
    lda.b $1A                   ; $82F045
    clc                         ; $82F047
    adc.w $8000                 ; $82F048
    sta.b $1A                   ; $82F04B
    lda.b $1C                   ; $82F04D
    adc.w $8000                 ; $82F04F
    sta.b $1C                   ; $82F052
    lda.b $1A                   ; $82F054
    eor.w #$FFFF                ; $82F056
    sec                         ; $82F059
    adc.w #$0000                ; $82F05A
    sta.w $020A                 ; $82F05D
    lda.b $1C                   ; $82F060
    eor.w #$FFFF                ; $82F062
    adc.w #$0000                ; $82F065
    sta.w $020C                 ; $82F068
    lda.w $1EF6                 ; $82F06B
    eor.w #$FFFF                ; $82F06E
    inc a                       ; $82F071
    sta.b $24                   ; $82F072
    lda.w $0206                 ; $82F074
    eor.b $24                   ; $82F077
    bpl CODE_82F07E             ; $82F079
    jmp.w $F158                 ; $82F07B
CODE_82F07E:
    lda.w $0206                 ; $82F07E
    sec                         ; $82F081
    sbc.b $24                   ; $82F082
    eor.b $24                   ; $82F084
    bmi CODE_82F08B             ; $82F086
    jmp.w $F168                 ; $82F088
CODE_82F08B:
    lda.w $0206                 ; $82F08B
    bpl CODE_82F09F             ; $82F08E
    lda.b $24                   ; $82F090
    eor.w #$FFFF                ; $82F092
    inc a                       ; $82F095
    sta.b $24                   ; $82F096
    lda.w $0206                 ; $82F098
    eor.w #$FFFF                ; $82F09B
    inc a                       ; $82F09E
CODE_82F09F:
    sta.b $26                   ; $82F09F
    lda.b $24                   ; $82F0A1
    cmp.w #$0100                ; $82F0A3
    bpl CODE_82F0E7             ; $82F0A6
    lda.b $26                   ; $82F0A8
    sta.w $4204                 ; $82F0AA
    sep.b #$20                  ; $82F0AD
    lda.b $24                   ; $82F0AF
    sta.w $4206                 ; $82F0B1
    rep.b #$20                  ; $82F0B4
    xba                         ; $82F0B6
    xba                         ; $82F0B7
    xba                         ; $82F0B8
    xba                         ; $82F0B9
    nop                         ; $82F0BA
    lda.w $4216                 ; $82F0BB
    xba                         ; $82F0BE
    sta.w $4204                 ; $82F0BF
    sep.b #$20                  ; $82F0C2
    lda.b $24                   ; $82F0C4
    sta.w $4206                 ; $82F0C6
    rep.b #$20                  ; $82F0C9
    xba                         ; $82F0CB
    xba                         ; $82F0CC
    xba                         ; $82F0CD
    xba                         ; $82F0CE
    nop                         ; $82F0CF
    lda.w $4214                 ; $82F0D0
    jmp.w $F0FF                 ; $82F0D3
    incbin !baserom:$170D6..$170E7
CODE_82F0E7:
    lda.b $24                   ; $82F0E7
    lsr a                       ; $82F0E9
    sec                         ; $82F0EA
    sbc.b $26                   ; $82F0EB
    eor.w #$FFFF                ; $82F0ED
    inc a                       ; $82F0F0
    cmp.w #$0080                ; $82F0F1
    bpl CODE_82F168             ; $82F0F4
    cmp.w #$FF80                ; $82F0F6
    bmi CODE_82F158             ; $82F0F9
    clc                         ; $82F0FB
    adc.w #$0080                ; $82F0FC
    pha                         ; $82F0FF
    lda.w #$0000                ; $82F100
    sta.l $308000               ; $82F103
    pla                         ; $82F107
    sta.b $28                   ; $82F108
    sta.l $308000               ; $82F10A
    lda.w $020C                 ; $82F10E
    sta.l $308000               ; $82F111
    nop                         ; $82F115
    lda.l $308000               ; $82F116
    sta.b $24                   ; $82F11A
    lda.l $308000               ; $82F11C
    sta.b $26                   ; $82F120
    lda.b $25                   ; $82F122
    sta.w $020C                 ; $82F124
    lda.w #$0000                ; $82F127
    sta.l $308000               ; $82F12A
    lda.b $28                   ; $82F12E
    sta.l $308000               ; $82F130
    lda.w $1EF6                 ; $82F134
    sec                         ; $82F137
    sbc.w $1EC4                 ; $82F138
    sta.l $308000               ; $82F13B
    lda.l $308000               ; $82F13F
    sta.b $24                   ; $82F143
    lda.l $308000               ; $82F145
    sta.b $26                   ; $82F149
    lda.b $23                   ; $82F14B
    and.w #$FF00                ; $82F14D
    sta.b $1E                   ; $82F150
    lda.b $25                   ; $82F152
    sta.b $20                   ; $82F154
    bra CODE_82F189             ; $82F156
CODE_82F158:
    stz.w $020C                 ; $82F158
    stz.b $1E                   ; $82F15B
    stz.b $20                   ; $82F15D
    lda.w #$8000                ; $82F15F
    trb.w $1E66                 ; $82F162
    jmp.w $F189                 ; $82F165
CODE_82F168:
    lda.w $1EF4                 ; $82F168
    sta.b $1E                   ; $82F16B
    lda.w $1EF6                 ; $82F16D
    sta.b $20                   ; $82F170
    lda.w #$8000                ; $82F172
    tsb.w $1E66                 ; $82F175
    lda.w $1EF4                 ; $82F178
    sec                         ; $82F17B
    sbc.w $1EC2                 ; $82F17C
    sta.b $1E                   ; $82F17F
    lda.w $1EF6                 ; $82F181
    sbc.w $1EC4                 ; $82F184
    sta.b $20                   ; $82F187
CODE_82F189:
    lda.b $22                   ; $82F189
    sec                         ; $82F18B
    sbc.w $020B                 ; $82F18C
    clc                         ; $82F18F
    adc.w $01CE                 ; $82F190
    sta.w $01CE                 ; $82F193
    lda.w $0204                 ; $82F196
    sec                         ; $82F199
    sbc.b $1E                   ; $82F19A
    sta.w $0204                 ; $82F19C
    lda.w $0206                 ; $82F19F
    sbc.b $20                   ; $82F1A2
    sta.w $0206                 ; $82F1A4
    lda.w $0204                 ; $82F1A7
    sta.w $0200                 ; $82F1AA
    clc                         ; $82F1AD
    adc.w $1EF4                 ; $82F1AE
    sta.w $1EF4                 ; $82F1B1
    lda.w $0206                 ; $82F1B4
    sta.w $0202                 ; $82F1B7
    adc.w $1EF6                 ; $82F1BA
    sta.w $1EF6                 ; $82F1BD
    rts                         ; $82F1C0
road_shift_apply:
    lda.w $020B                 ; $82F1C1
    sta.b $22                   ; $82F1C4
    ldx.b $16                   ; $82F1C6
    stz.w $8000                 ; $82F1C8
    lda.l $7E3DEE,x             ; $82F1CB
    sec                         ; $82F1CF
    sbc.l $7E3DEC,x             ; $82F1D0
    asl a                       ; $82F1D4
    asl a                       ; $82F1D5
    sta.w $8000                 ; $82F1D6
    lda.w #$8000                ; $82F1D9
    sec                         ; $82F1DC
    sbc.b $14                   ; $82F1DD
    lsr a                       ; $82F1DF
    sta.w $8000                 ; $82F1E0
    nop                         ; $82F1E3
    lda.w $8000                 ; $82F1E4
    sta.b $1A                   ; $82F1E7
    lda.w $8000                 ; $82F1E9
    sta.b $1C                   ; $82F1EC
    stz.w $8000                 ; $82F1EE
    lda.l $7E3D6C,x             ; $82F1F1
    bpl CODE_82F21A             ; $82F1F5
    bit.w #$0002                ; $82F1F7
    bne CODE_82F21A             ; $82F1FA
    lda.l $7E3AEF,x             ; $82F1FC
    and.w #$00FF                ; $82F200
    bit.w #$0080                ; $82F203
    beq CODE_82F20B             ; $82F206
    ora.w #$FF00                ; $82F208
CODE_82F20B:
    eor.w #$FFFF                ; $82F20B
    sec                         ; $82F20E
    adc.l $7E3DEE,x             ; $82F20F
    sec                         ; $82F213
    sbc.l $7E3DEC,x             ; $82F214
    bra CODE_82F223             ; $82F218
CODE_82F21A:
    lda.l $7E3DF0,x             ; $82F21A
    sec                         ; $82F21E
    sbc.l $7E3DEE,x             ; $82F21F
CODE_82F223:
    asl a                       ; $82F223
    sta.w $8000                 ; $82F224
    lda.b $14                   ; $82F227
    sta.w $8000                 ; $82F229
    lda.b $1A                   ; $82F22C
    clc                         ; $82F22E
    adc.w $8000                 ; $82F22F
    sta.b $1A                   ; $82F232
    lda.b $1C                   ; $82F234
    adc.w $8000                 ; $82F236
    sta.b $1C                   ; $82F239
    lda.b $1A                   ; $82F23B
    eor.w #$FFFF                ; $82F23D
    sec                         ; $82F240
    adc.w #$0000                ; $82F241
    sta.w $020A                 ; $82F244
    lda.b $1C                   ; $82F247
    eor.w #$FFFF                ; $82F249
    adc.w #$0000                ; $82F24C
    sta.w $020C                 ; $82F24F
    lda.w $1EF6                 ; $82F252
    eor.w #$FFFF                ; $82F255
    inc a                       ; $82F258
    sta.b $24                   ; $82F259
    lda.w $1E66                 ; $82F25B
    bmi CODE_82F26A             ; $82F25E
    stz.w $020C                 ; $82F260
    stz.b $1E                   ; $82F263
    stz.b $20                   ; $82F265
    jmp.w $F27B                 ; $82F267
CODE_82F26A:
    lda.w $1EF4                 ; $82F26A
    sec                         ; $82F26D
    sbc.w $1EC2                 ; $82F26E
    sta.b $1E                   ; $82F271
    lda.w $1EF6                 ; $82F273
    sbc.w $1EC4                 ; $82F276
    sta.b $20                   ; $82F279
    lda.b $22                   ; $82F27B
    sec                         ; $82F27D
    sbc.w $020B                 ; $82F27E
    clc                         ; $82F281
    adc.w $01CE                 ; $82F282
    sta.w $01CE                 ; $82F285
    lda.w $0204                 ; $82F288
    sec                         ; $82F28B
    sbc.b $1E                   ; $82F28C
    sta.w $0204                 ; $82F28E
    lda.w $0206                 ; $82F291
    sbc.b $20                   ; $82F294
    sta.w $0206                 ; $82F296
    lda.w $0204                 ; $82F299
    sta.w $0200                 ; $82F29C
    clc                         ; $82F29F
    adc.w $1EF4                 ; $82F2A0
    sta.w $1EF4                 ; $82F2A3
    lda.w $0206                 ; $82F2A6
    sta.w $0202                 ; $82F2A9
    adc.w $1EF6                 ; $82F2AC
    sta.w $1EF6                 ; $82F2AF
    rts                         ; $82F2B2
player_segment_step:
    stz.b $2A                   ; $82F2B3
    lda.w $0226                 ; $82F2B5
    cmp.w $1EDE                 ; $82F2B8
    bcc CODE_82F31D             ; $82F2BB
    lda.w $1EE0                 ; $82F2BD
    clc                         ; $82F2C0
    adc.w $0226                 ; $82F2C1
    sta.w $0226                 ; $82F2C4
    lda.w $1EE0                 ; $82F2C7
    clc                         ; $82F2CA
    adc.w $023E                 ; $82F2CB
    sta.w $023E                 ; $82F2CE
    lda.w $1EDE                 ; $82F2D1
    asl a                       ; $82F2D4
    and.w #$003E                ; $82F2D5
    tax                         ; $82F2D8
    lda.l $7E3DEE,x             ; $82F2D9
    sec                         ; $82F2DD
    sbc.l $7E3DF0,x             ; $82F2DE
    pha                         ; $82F2E2
    clc                         ; $82F2E3
    adc.l $7E3DEE,x             ; $82F2E4
    sta.l $7E3DEC,x             ; $82F2E8
    sta.l $7E3E2C,x             ; $82F2EC
    eor.w #$FFFF                ; $82F2F0
    inc a                       ; $82F2F3
    clc                         ; $82F2F4
    adc.w $0206                 ; $82F2F5
    sta.w $0206                 ; $82F2F8
    sta.w $0202                 ; $82F2FB
    pla                         ; $82F2FE
    sta.w $020C                 ; $82F2FF
    lda.w $1EDE                 ; $82F302
    asl a                       ; $82F305
    and.w #$003E                ; $82F306
    tax                         ; $82F309
    lda.l $7E3DEC,x             ; $82F30A
    stz.w $1EC2                 ; $82F30E
    sta.w $1EC4                 ; $82F311
    stz.w $1EE0                 ; $82F314
    lda.w #$7FFF                ; $82F317
    sta.w $1EDE                 ; $82F31A
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
    txy                         ; $82F335
    lda.l $840002,x             ; $82F336
    and.w #$00FF                ; $82F33A
    tax                         ; $82F33D
    jsr.w ($F374,x)             ; $82F33E
    lda.w $1E5E                 ; $82F341
    clc                         ; $82F344
    adc.w #$0014                ; $82F345
    sta.w $1E5E                 ; $82F348
    bra CODE_82F31D             ; $82F34B
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
    lda.w $1E60                 ; $82F362
    clc                         ; $82F365
    adc.w #$0014                ; $82F366
    sta.w $1E60                 ; $82F369
    lda.w #$0002                ; $82F36C
    tsb.b $2A                   ; $82F36F
    bra CODE_82F31D             ; $82F371
CODE_82F373:
    rts                         ; $82F373
    incbin !baserom:$17374..$17380
func_82F380:
    rts                         ; $82F380
    bit.w $1E66                 ; $82F381
    bpl CODE_82F3B7             ; $82F384
    lda.w $0204                 ; $82F386
    clc                         ; $82F389
    adc.w $1EC2                 ; $82F38A
    sta.w $0204                 ; $82F38D
    lda.w $0206                 ; $82F390
    adc.w $1EC4                 ; $82F393
    sta.w $0206                 ; $82F396
    tyx                         ; $82F399
    lda.l $840000,x             ; $82F39A
    clc                         ; $82F39E
    adc.w #$003F                ; $82F39F
    adc.l $840010,x             ; $82F3A2
    sta.w $1EDE                 ; $82F3A6
    lda.l $840012,x             ; $82F3A9
    sta.w $1EE0                 ; $82F3AD
    stz.w $020A                 ; $82F3B0
    stz.w $020C                 ; $82F3B3
    rts                         ; $82F3B6
CODE_82F3B7:
    tyx                         ; $82F3B7
    lda.w $1EE6                 ; $82F3B8
    clc                         ; $82F3BB
    adc.w $0226                 ; $82F3BC
    sta.w $0226                 ; $82F3BF
    lda.w $1EE6                 ; $82F3C2
    clc                         ; $82F3C5
    adc.w $023E                 ; $82F3C6
    sta.w $023E                 ; $82F3C9
    stz.w $1EE6                 ; $82F3CC
    rts                         ; $82F3CF
    stz.w $1E66                 ; $82F3D0 static
    rts                         ; $82F3D3 static
    pea.w $8330                 ; $82F3D4
    plb                         ; $82F3D7
    stz.b $3C                   ; $82F3D8
    lda.w $02D9                 ; $82F3DA
    asl a                       ; $82F3DD
    and.w #$003E                ; $82F3DE
    sta.b $16                   ; $82F3E1
    tax                         ; $82F3E3
    stz.w $8000                 ; $82F3E4
    lda.l $7E436E,x             ; $82F3E7
    sec                         ; $82F3EB
    sbc.l $7E436C,x             ; $82F3EC
    asl a                       ; $82F3F0
    sta.w $8000                 ; $82F3F1
    lda.w $02D7                 ; $82F3F4
    lsr a                       ; $82F3F7
    and.w #$FF80                ; $82F3F8
    sta.w $8000                 ; $82F3FB
    sta.b $14                   ; $82F3FE
    lda.w $8000                 ; $82F400
    sta.w $1EFE                 ; $82F403
    lda.w $8000                 ; $82F406
    clc                         ; $82F409
    adc.l $7E436C,x             ; $82F40A
    beq CODE_82F413             ; $82F40E
    bmi CODE_82F413             ; $82F410
    dec a                       ; $82F412
CODE_82F413:
    sta.w $1F00                 ; $82F413
    lda.w $0314                 ; $82F416
    and.w #$00FF                ; $82F419
    tax                         ; $82F41C
    jsr.w ($F4C1,x)             ; $82F41D
    lda.w $1EFE                 ; $82F420
    pha                         ; $82F423
    lda.w $1F00                 ; $82F424
    pha                         ; $82F427
    lda.w $0313                 ; $82F428
    and.w #$00FF                ; $82F42B
    tax                         ; $82F42E
    jsr.w ($F4B5,x)             ; $82F42F
    pla                         ; $82F432
    sta.w $1EC8                 ; $82F433
    pla                         ; $82F436
    sta.w $1EC6                 ; $82F437
    lda.w $01D2                 ; $82F43A
    sta.w $02DC                 ; $82F43D
    lda.w $01D2                 ; $82F440
    bpl CODE_82F45F             ; $82F443
    eor.w #$FFFF                ; $82F445
    inc a                       ; $82F448
    sta.w $4204                 ; $82F449
    sep.b #$20                  ; $82F44C
    lda.w $1E9E                 ; $82F44E
    sta.w $4206                 ; $82F451
    rep.b #$20                  ; $82F454
    nop                         ; $82F456
    stz.w $8000                 ; $82F457
    lda.w #$0001                ; $82F45A
    bra CODE_82F473             ; $82F45D
CODE_82F45F:
    sta.w $4204                 ; $82F45F
    sep.b #$20                  ; $82F462
    lda.w $1E9E                 ; $82F464
    sta.w $4206                 ; $82F467
    rep.b #$20                  ; $82F46A
    nop                         ; $82F46C
    stz.w $8000                 ; $82F46D
    lda.w #$FFFF                ; $82F470
CODE_82F473:
    sta.w $8000                 ; $82F473
    lda.w $4214                 ; $82F476
    sta.w $8000                 ; $82F479
    nop                         ; $82F47C
    lda.w $8000                 ; $82F47D
    sta.w $01D2                 ; $82F480
    lda.w $8000                 ; $82F483
    plb                         ; $82F486
    rts                         ; $82F487
    stz.w $02F5                 ; $82F488
    stz.w $02BB                 ; $82F48B
    rts                         ; $82F48E
    stz.w $1EEA                 ; $82F48F
    stz.w $1EEC                 ; $82F492
    lda.w $02B7                 ; $82F495
    sta.w $02B3                 ; $82F498
    clc                         ; $82F49B
    adc.w $1EFE                 ; $82F49C
    sta.w $1EFE                 ; $82F49F
    lda.w $02B9                 ; $82F4A2
    sta.w $02B5                 ; $82F4A5
    adc.w $1F00                 ; $82F4A8
    sta.w $1F00                 ; $82F4AB
    stz.w $02BD                 ; $82F4AE
    stz.w $02BF                 ; $82F4B1
    rts                         ; $82F4B4
    incbin !baserom:$174B5..$17764
    lda.w $02D9                 ; $82F764
    and.w #$003E                ; $82F767
    cmp.w #$0018                ; $82F76A
    bcc CODE_82F772             ; $82F76D
    jmp.w $F95D                 ; $82F76F
CODE_82F772:
    lda.w $02BE                 ; $82F772
    sta.b $22                   ; $82F775
    ldx.b $16                   ; $82F777
    stz.w $8000                 ; $82F779
    lda.l $7E436E,x             ; $82F77C
    sec                         ; $82F780
    sbc.l $7E436C,x             ; $82F781
    asl a                       ; $82F785
    asl a                       ; $82F786
    sta.w $8000                 ; $82F787
    lda.w #$8000                ; $82F78A
    sec                         ; $82F78D
    sbc.b $14                   ; $82F78E
    lsr a                       ; $82F790
    sta.w $8000                 ; $82F791
    nop                         ; $82F794
    lda.w $8000                 ; $82F795
    sta.b $1A                   ; $82F798
    lda.w $8000                 ; $82F79A
    sta.b $1C                   ; $82F79D
    stz.w $8000                 ; $82F79F
    lda.l $7E42EC,x             ; $82F7A2
    bpl CODE_82F7CB             ; $82F7A6
    incbin !baserom:$177A8..$177CB
CODE_82F7CB:
    lda.l $7E4370,x             ; $82F7CB
    sec                         ; $82F7CF
    sbc.l $7E436E,x             ; $82F7D0
    asl a                       ; $82F7D4
    sta.w $8000                 ; $82F7D5
    lda.b $14                   ; $82F7D8
    sta.w $8000                 ; $82F7DA
    lda.b $1A                   ; $82F7DD
    clc                         ; $82F7DF
    adc.w $8000                 ; $82F7E0
    sta.b $1A                   ; $82F7E3
    lda.b $1C                   ; $82F7E5
    adc.w $8000                 ; $82F7E7
    sta.b $1C                   ; $82F7EA
    lda.b $1A                   ; $82F7EC
    eor.w #$FFFF                ; $82F7EE
    sec                         ; $82F7F1
    adc.w #$0000                ; $82F7F2
    sta.w $02BD                 ; $82F7F5
    lda.b $1C                   ; $82F7F8
    eor.w #$FFFF                ; $82F7FA
    adc.w #$0000                ; $82F7FD
    sta.w $02BF                 ; $82F800
    lda.w $1F00                 ; $82F803
    eor.w #$FFFF                ; $82F806
    inc a                       ; $82F809
    sta.b $24                   ; $82F80A
    lda.w $02B9                 ; $82F80C
    eor.b $24                   ; $82F80F
    bpl CODE_82F816             ; $82F811
    jmp.w $F8F4                 ; $82F813
CODE_82F816:
    lda.w $02B9                 ; $82F816
    sec                         ; $82F819
    sbc.b $24                   ; $82F81A
    eor.b $24                   ; $82F81C
    bmi CODE_82F823             ; $82F81E
    jmp.w $F904                 ; $82F820
CODE_82F823:
    lda.w $02B9                 ; $82F823
    bpl CODE_82F837             ; $82F826
    lda.b $24                   ; $82F828
    eor.w #$FFFF                ; $82F82A
    inc a                       ; $82F82D
    sta.b $24                   ; $82F82E
    lda.w $02B9                 ; $82F830
    eor.w #$FFFF                ; $82F833
    inc a                       ; $82F836
CODE_82F837:
    sta.b $26                   ; $82F837
    lda.b $24                   ; $82F839
    cmp.w #$0100                ; $82F83B
    db $10,$43 ; bpl            ; $82F83E
    lda.b $26                   ; $82F840
    sta.w $4204                 ; $82F842
    sep.b #$20                  ; $82F845
    lda.b $24                   ; $82F847
    sta.w $4206                 ; $82F849
    rep.b #$20                  ; $82F84C
    nop                         ; $82F84E
    nop                         ; $82F84F
    nop                         ; $82F850
    nop                         ; $82F851
    nop                         ; $82F852
    nop                         ; $82F853
    nop                         ; $82F854
    lda.w $4216                 ; $82F855
    xba                         ; $82F858
    sta.w $4204                 ; $82F859
    sep.b #$20                  ; $82F85C
    lda.b $24                   ; $82F85E
    sta.w $4206                 ; $82F860
    rep.b #$20                  ; $82F863
    nop                         ; $82F865
    nop                         ; $82F866
    nop                         ; $82F867
    nop                         ; $82F868
    nop                         ; $82F869
    nop                         ; $82F86A
    nop                         ; $82F86B
    lda.w $4214                 ; $82F86C
    jmp.w $F89B                 ; $82F86F
    incbin !baserom:$17872..$1789B
    pha                         ; $82F89B
    lda.w #$0000                ; $82F89C
    sta.l $308000               ; $82F89F
    pla                         ; $82F8A3
    sta.b $28                   ; $82F8A4
    sta.l $308000               ; $82F8A6
    lda.w $02BF                 ; $82F8AA
    sta.l $308000               ; $82F8AD
    nop                         ; $82F8B1
    lda.l $308000               ; $82F8B2
    sta.b $24                   ; $82F8B6
    lda.l $308000               ; $82F8B8
    sta.b $26                   ; $82F8BC
    lda.b $25                   ; $82F8BE
    sta.w $02BF                 ; $82F8C0
    lda.w #$0000                ; $82F8C3
    sta.l $308000               ; $82F8C6
    lda.b $28                   ; $82F8CA
    sta.l $308000               ; $82F8CC
    lda.w $1F00                 ; $82F8D0
    sec                         ; $82F8D3
    sbc.w $1EC8                 ; $82F8D4
    sta.l $308000               ; $82F8D7
    lda.l $308000               ; $82F8DB
    sta.b $24                   ; $82F8DF
    lda.l $308000               ; $82F8E1
    sta.b $26                   ; $82F8E5
    lda.b $23                   ; $82F8E7
    and.w #$FF00                ; $82F8E9
    sta.b $1E                   ; $82F8EC
    lda.b $25                   ; $82F8EE
    sta.b $20                   ; $82F8F0
    bra CODE_82F925             ; $82F8F2
    stz.w $02BF                 ; $82F8F4
    stz.b $1E                   ; $82F8F7
    stz.b $20                   ; $82F8F9
    lda.w #$8000                ; $82F8FB
    trb.w $1E68                 ; $82F8FE
    jmp.w $F925                 ; $82F901
    lda.w $1EFE                 ; $82F904
    sta.b $1E                   ; $82F907
    lda.w $1F00                 ; $82F909
    sta.b $20                   ; $82F90C
    lda.w #$8000                ; $82F90E
    tsb.w $1E68                 ; $82F911
    lda.w $1EFE                 ; $82F914
    sec                         ; $82F917
    sbc.w $1EC6                 ; $82F918
    sta.b $1E                   ; $82F91B
    lda.w $1F00                 ; $82F91D
    sbc.w $1EC8                 ; $82F920
    sta.b $20                   ; $82F923
CODE_82F925:
    lda.b $22                   ; $82F925
    sec                         ; $82F927
    sbc.w $02BE                 ; $82F928
    clc                         ; $82F92B
    adc.w $01D2                 ; $82F92C
    sta.w $01D2                 ; $82F92F
    lda.w $02B7                 ; $82F932
    sec                         ; $82F935
    sbc.b $1E                   ; $82F936
    sta.w $02B7                 ; $82F938
    lda.w $02B9                 ; $82F93B
    sbc.b $20                   ; $82F93E
    sta.w $02B9                 ; $82F940
    lda.w $02B7                 ; $82F943
    sta.w $02B3                 ; $82F946
    clc                         ; $82F949
    adc.w $1EFE                 ; $82F94A
    sta.w $1EFE                 ; $82F94D
    lda.w $02B9                 ; $82F950
    sta.w $02B5                 ; $82F953
    adc.w $1F00                 ; $82F956
    sta.w $1F00                 ; $82F959
    rts                         ; $82F95C
    lda.w $02BE                 ; $82F95D
    sta.b $22                   ; $82F960
    ldx.b $16                   ; $82F962
    stz.w $8000                 ; $82F964
    lda.l $7E436E,x             ; $82F967
    sec                         ; $82F96B
    sbc.l $7E436C,x             ; $82F96C
    asl a                       ; $82F970
    asl a                       ; $82F971
    sta.w $8000                 ; $82F972
    lda.w #$8000                ; $82F975
    sec                         ; $82F978
    sbc.b $14                   ; $82F979
    lsr a                       ; $82F97B
    sta.w $8000                 ; $82F97C
    nop                         ; $82F97F
    lda.w $8000                 ; $82F980
    sta.b $1A                   ; $82F983
    lda.w $8000                 ; $82F985
    sta.b $1C                   ; $82F988
    stz.w $8000                 ; $82F98A
    lda.l $7E42EC,x             ; $82F98D
    bpl CODE_82F9B6             ; $82F991
    bit.w #$0002                ; $82F993
    bne CODE_82F9B6             ; $82F996
    lda.l $7E406F,x             ; $82F998
    and.w #$00FF                ; $82F99C
    bit.w #$0080                ; $82F99F
    beq CODE_82F9A7             ; $82F9A2
    incbin !baserom:$179A4..$179A7
CODE_82F9A7:
    eor.w #$FFFF                ; $82F9A7
    sec                         ; $82F9AA
    adc.l $7E436E,x             ; $82F9AB
    sec                         ; $82F9AF
    sbc.l $7E436C,x             ; $82F9B0
    bra CODE_82F9BF             ; $82F9B4
CODE_82F9B6:
    lda.l $7E4370,x             ; $82F9B6
    sec                         ; $82F9BA
    sbc.l $7E436E,x             ; $82F9BB
CODE_82F9BF:
    asl a                       ; $82F9BF
    sta.w $8000                 ; $82F9C0
    lda.b $14                   ; $82F9C3
    sta.w $8000                 ; $82F9C5
    lda.b $1A                   ; $82F9C8
    clc                         ; $82F9CA
    adc.w $8000                 ; $82F9CB
    sta.b $1A                   ; $82F9CE
    lda.b $1C                   ; $82F9D0
    adc.w $8000                 ; $82F9D2
    sta.b $1C                   ; $82F9D5
    lda.b $1A                   ; $82F9D7
    eor.w #$FFFF                ; $82F9D9
    sec                         ; $82F9DC
    adc.w #$0000                ; $82F9DD
    sta.w $02BD                 ; $82F9E0
    lda.b $1C                   ; $82F9E3
    eor.w #$FFFF                ; $82F9E5
    adc.w #$0000                ; $82F9E8
    sta.w $02BF                 ; $82F9EB
    lda.w $1F00                 ; $82F9EE
    eor.w #$FFFF                ; $82F9F1
    inc a                       ; $82F9F4
    sta.b $24                   ; $82F9F5
    lda.w $1E68                 ; $82F9F7
    bmi CODE_82FA06             ; $82F9FA
    stz.w $02BF                 ; $82F9FC
    stz.b $1E                   ; $82F9FF
    stz.b $20                   ; $82FA01
    jmp.w $FA17                 ; $82FA03
CODE_82FA06:
    lda.w $1EFE                 ; $82FA06
    sec                         ; $82FA09
    sbc.w $1EC6                 ; $82FA0A
    sta.b $1E                   ; $82FA0D
    lda.w $1F00                 ; $82FA0F
    sbc.w $1EC8                 ; $82FA12
    sta.b $20                   ; $82FA15
    lda.b $22                   ; $82FA17
    sec                         ; $82FA19
    sbc.w $02BE                 ; $82FA1A
    clc                         ; $82FA1D
    adc.w $01D2                 ; $82FA1E
    sta.w $01D2                 ; $82FA21
    lda.w $02B7                 ; $82FA24
    sec                         ; $82FA27
    sbc.b $1E                   ; $82FA28
    sta.w $02B7                 ; $82FA2A
    lda.w $02B9                 ; $82FA2D
    sbc.b $20                   ; $82FA30
    sta.w $02B9                 ; $82FA32
    lda.w $02B7                 ; $82FA35
    sta.w $02B3                 ; $82FA38
    clc                         ; $82FA3B
    adc.w $1EFE                 ; $82FA3C
    sta.w $1EFE                 ; $82FA3F
    lda.w $02B9                 ; $82FA42
    sta.w $02B5                 ; $82FA45
    adc.w $1F00                 ; $82FA48
    sta.w $1F00                 ; $82FA4B
    rts                         ; $82FA4E
player2_segment_step:
    stz.b $2A                   ; $82FA4F
    lda.w $02D9                 ; $82FA51
    cmp.w $1EE2                 ; $82FA54
    bcc CODE_82FAB9             ; $82FA57
    lda.w $1EE4                 ; $82FA59
    clc                         ; $82FA5C
    adc.w $02D9                 ; $82FA5D
    sta.w $02D9                 ; $82FA60
    lda.w $1EE4                 ; $82FA63
    clc                         ; $82FA66
    adc.w $02F1                 ; $82FA67
    sta.w $02F1                 ; $82FA6A
    lda.w $1EE2                 ; $82FA6D
    asl a                       ; $82FA70
    and.w #$003E                ; $82FA71
    tax                         ; $82FA74
    lda.l $7E436E,x             ; $82FA75
    sec                         ; $82FA79
    sbc.l $7E4370,x             ; $82FA7A
    pha                         ; $82FA7E
    clc                         ; $82FA7F
    adc.l $7E436E,x             ; $82FA80
    sta.l $7E436C,x             ; $82FA84
    sta.l $7E43AC,x             ; $82FA88
    eor.w #$FFFF                ; $82FA8C
    inc a                       ; $82FA8F
    clc                         ; $82FA90
    adc.w $02B9                 ; $82FA91
    sta.w $02B9                 ; $82FA94
    sta.w $02B5                 ; $82FA97
    pla                         ; $82FA9A
    sta.w $02BF                 ; $82FA9B
    lda.w $1EE2                 ; $82FA9E
    asl a                       ; $82FAA1
    and.w #$003E                ; $82FAA2
    tax                         ; $82FAA5
    lda.l $7E436C,x             ; $82FAA6
    stz.w $1EC6                 ; $82FAAA
    sta.w $1EC8                 ; $82FAAD
    stz.w $1EE4                 ; $82FAB0
    lda.w #$7FFF                ; $82FAB3
    sta.w $1EE2                 ; $82FAB6
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
    txy                         ; $82FAD1
    lda.l $840002,x             ; $82FAD2
    and.w #$00FF                ; $82FAD6
    tax                         ; $82FAD9
    jsr.w ($FB10,x)             ; $82FADA
    lda.w $1E62                 ; $82FADD
    clc                         ; $82FAE0
    adc.w #$0014                ; $82FAE1
    sta.w $1E62                 ; $82FAE4
    bra CODE_82FAB9             ; $82FAE7
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
    lda.w $1E64                 ; $82FAFE
    clc                         ; $82FB01
    adc.w #$0014                ; $82FB02
    sta.w $1E64                 ; $82FB05
    lda.w #$0002                ; $82FB08
    tsb.b $2A                   ; $82FB0B
    bra CODE_82FAB9             ; $82FB0D
CODE_82FB0F:
    rts                         ; $82FB0F
    incbin !baserom:$17B10..$17B1C
func_82FB1C:
    rts                         ; $82FB1C
    bit.w $1E68                 ; $82FB1D
    bpl CODE_82FB53             ; $82FB20
    lda.w $02B7                 ; $82FB22
    clc                         ; $82FB25
    adc.w $1EC6                 ; $82FB26
    sta.w $02B7                 ; $82FB29
    lda.w $02B9                 ; $82FB2C
    adc.w $1EC8                 ; $82FB2F
    sta.w $02B9                 ; $82FB32
    tyx                         ; $82FB35
    lda.l $840000,x             ; $82FB36
    clc                         ; $82FB3A
    adc.w #$003F                ; $82FB3B
    adc.l $840010,x             ; $82FB3E
    sta.w $1EE2                 ; $82FB42
    lda.l $840012,x             ; $82FB45
    sta.w $1EE4                 ; $82FB49
    stz.w $02BD                 ; $82FB4C
    stz.w $02BF                 ; $82FB4F
    rts                         ; $82FB52
CODE_82FB53:
    tyx                         ; $82FB53
    lda.w $1EE8                 ; $82FB54
    clc                         ; $82FB57
    adc.w $02D9                 ; $82FB58
    sta.w $02D9                 ; $82FB5B
    lda.w $1EE8                 ; $82FB5E
    clc                         ; $82FB61
    adc.w $02F1                 ; $82FB62
    sta.w $02F1                 ; $82FB65
    stz.w $1EE8                 ; $82FB68
    rts                         ; $82FB6B
    stz.w $1E68                 ; $82FB6C static
    rts                         ; $82FB6F static
decompress:
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
