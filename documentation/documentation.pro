TEMPLATE = subdirs # dummy entry to avoid compiling
contains(QMAKE_HOST.arch, arm.*):{
            }
  else {
       CONFIG(debug ,debug|release){
       dox.commands = cd $$PWD/manual ;doxygen  manual.doxy; cd $$PWD ;doxygen  $$PWD/api/api.doxy;
       dox.depends= FORCE
       PRE_TARGETDEPS       +=    dox
       }
    }





dox.path=/usr/share/doc/$$TARGET
dox.files= $$PWD/manual/*
QMAKE_EXTRA_TARGETS   +=   dox

DISTFILES += \
    api/api.doxy \
    manual/images/Gallery_image_options.png \
    manual/images/Gallery_rx.png \
    manual/images/Gallery_template.png \
    manual/images/Gallery_templates.png \
    manual/images/Gallery_tx.png \
    manual/images/Gallery_txdrm.png \
    manual/images/bsr_nfy.png \
    manual/images/bsr_select.png \
    manual/images/calibration.png \
    manual/images/cat_flrig.png \
    manual/images/config.png \
    manual/images/config1.png \
    manual/images/config10.png \
    manual/images/config11.png \
    manual/images/config12.png \
    manual/images/config2.png \
    manual/images/config3.png \
    manual/images/config4.png \
    manual/images/config5.png \
    manual/images/config6.png \
    manual/images/config7.png \
    manual/images/config8.png \
    manual/images/config9.png \
    manual/images/cqrlog1.png \
    manual/images/cqrlog2.png \
    manual/images/editor_1.png \
    manual/images/editor_1b.png \
    manual/images/editor_2.png \
    manual/images/editor_2b.png \
    manual/images/editor_3.png \
    manual/images/editor_image_size.png \
    manual/images/editor_image_sizeb.png \
    manual/images/editor_resize.png \
    manual/images/editor_rotate.png \
    manual/images/editor_rotate2.png \
    manual/images/editor_text_rotate.png \
    manual/images/entertext.png \
    manual/images/entertextb.png \
    manual/images/fix.png \
    manual/images/flrig1.png \
    manual/images/hybrid_checkbox.png \
    manual/images/hybrid_dis_checkbox.png \
    manual/images/i_binary.png \
    manual/images/i_eraser.png \
    manual/images/multiline.png \
    manual/images/multilineb.png \
    manual/images/pavu_playback.png \
    manual/images/pavu_rec.png \
    manual/images/receivedrm.png \
    manual/images/refreshbutton.png \
    manual/images/rxdrm_constellation.png \
    manual/images/rxdrm_segments.png \
    manual/images/rxdrm_status.png \
    manual/images/rxwidget1.png \
    manual/images/spectrum1.png \
    manual/images/spectrum_wf.png \
    manual/images/statusbar.png \
    manual/images/statusleds.png \
    manual/images/transmitdrm.png \
    manual/images/transmitdrmb.png \
    manual/images/tx-with-template.png \
    manual/images/txdrm_compression.png \
    manual/images/txdrm_options.png \
    manual/images/txdrm_status.png \
    manual/images/txwidget1.png \
    manual/images/vk4aes.jpg \
    manual/images/waterfall.png \
    manual/images/wf_bsr_id.png \
    manual/images/wftextpopup.png \
    manual/manual.doxy \
    manual/manual.txt \
    manual/qsstv.css
