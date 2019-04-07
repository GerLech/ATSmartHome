/* Defines Styles, Pages and Forms for AT Smarthome
version 0.1
*/

#define ATSTYLEBLUEYELLOW 0
#define ATSTYLEDEVLIST 1
#define ATSTYLEBUTTON 2
#define ATSTYLEFRMLBL 3
#define ATSTYLEFRMINPUT 4
#define ATSTYLESMALLTITLE 5
#define ATSTYLEKBDKEY 6
#define ATSTYLEKBDINP 7
#define ATSTYLECHLLIST 8
#define ATSTYLECHLLISTACT 9
#define ATSTYLEOPTIONS 10
#define ATSTYLESELECTED 11
#define ATSTYLEFRMLBLCTR 12

const ATSTYLE AT_display_styles[] = {
  { //ATSTYLEBLUEYELLOW
    .fill = ATblue,
    .border = ATblue,
    .color = ATyellow,
    .alignment = ATALIGNCENTER,
    .font = &AT_Bold12pt7b
  },
  { //ATSTYLEDEVLIST
    .fill = ATyellow,
    .border = ATblack,
    .color = ATblack,
    .alignment = ATALIGNLEFT,
    .font = &AT_Standard9pt7b
  },
  { //ATSTYLEBUTTON
    .fill = ATgreen,
    .border = ATblack,
    .color = ATblack,
    .alignment = ATALIGNCENTER,
    .font = &AT_Standard9pt7b
  },
  { //ATSTYLEFRMLBL
    .fill = ATblack,
    .border = ATblack,
    .color = ATwhite,
    .alignment = ATALIGNRIGHT,
    .font = &AT_Standard9pt7b
  },
  { //ATSTYLEFRMINPUT
    .fill = ATwhite,
    .border = ATblue,
    .color = ATblack,
    .alignment = ATALIGNLEFT,
    .font = &AT_Standard9pt7b
  },
  { //ATSTYLESMALLTITLE
    .fill = ATblue,
    .border = ATblue,
    .color = ATyellow,
    .alignment = ATALIGNCENTER,
    .font = &AT_Standard9pt7b
  },
  { //ATSTYLEKBDKEY
    .fill = ATlightgray,
    .border = ATblue,
    .color = ATblack,
    .alignment = ATALIGNCENTER,
    .font = &AT_Bold12pt7b
  },
  { //ATSTYLEKBDINP
    .fill = ATwhite,
    .border = ATblack,
    .color = ATblack,
    .alignment = ATALIGNLEFT,
    .font = &AT_Standard9pt7b
  },
  { //ATSTYLECHLLIST
    .fill = ATwhite,
    .border = ATdarkgray,
    .color = ATblack,
    .alignment = ATALIGNLEFT,
    .font = &AT_Standard9pt7b
  },
  { //ATSTYLECHLLISTACT
    .fill = ATyellow,
    .border = ATdarkgray,
    .color = ATblack,
    .alignment = ATALIGNLEFT,
    .font = &AT_Standard9pt7b
  },
  { //ATSTYLEOPTIONS
    .fill = ATwhite,
    .border = ATdarkgray,
    .color = ATblack,
    .alignment = ATALIGNCENTER,
    .font = &AT_Standard9pt7b
  },
  { //ATSTYLESELECTED
    .fill = ATblue,
    .border = ATyellow,
    .color = ATwhite,
    .alignment = ATALIGNCENTER,
    .font = &AT_Standard9pt7b
  },
  { //ATSTYLEFRMLBLCTR
    .fill = ATblack,
    .border = ATblack,
    .color = ATwhite,
    .alignment = ATALIGNCENTER,
    .font = &AT_Standard9pt7b
  },
};

#define ATPAGRESULTS 0
#define ATPAGDEVICE 1
#define ATPAGCHANNEL 2
#define ATPAGDEVSTP 3
#define ATPAGWDGTSTP 4
#define ATPAGSYSSTP 5
#define ATPAGREGISTER 6
#define ATPAGTRIGGER 7
#define ATPAGTRIGSTP 8

const ATPAGETYPE AT_display_pages[8] = {
  {  //ATPAGRESULTS
    {.title = ATTXTSMARTHOME},
    .subpages = 32,
    .content = ATCONTRESULTS,
    .topbarType = ATBARTITLECLOCK,
    .topbarStyle = ATSTYLEBLUEYELLOW,
    .botbarType = ATBARSTATUS,
    .botbarStyle = ATSTYLEBLUEYELLOW,
    {.botbarText = ""},
    .previousPage = ATPAGRESULTS
  },
  {{ //ATPAGDEVICE
    .title = ATTXTDEVICES},
    .subpages = 3,
    .content = ATCONTLIST,
    .topbarType = ATBARTITLEPAGE,
    .topbarStyle = ATSTYLEBLUEYELLOW,
    .botbarType = ATBARBACK,
    .botbarStyle = ATSTYLEBUTTON,
    {.botbarText = ""},
    .previousPage = ATPAGRESULTS
  },
  {{ //ATPAGCHANNEL
    .title = ATTXTCHANNELS},
    .subpages = 1,
    .content = ATCONTLIST,
    .topbarType = ATBARTITLEPAGE,
    .topbarStyle = ATSTYLEBLUEYELLOW,
    .botbarType = ATBARBACK,
    .botbarStyle = ATSTYLEBUTTON,
    {.botbarText = ""},
    .previousPage = ATPAGDEVICE
  },
  {{ //ATPAGDEVSTP
    .title = ATTXTDEVSTP},
    .subpages = 1,
    .content = ATCONTFORM,
    .topbarType = ATBARDEVICE,
    .topbarStyle = ATSTYLESMALLTITLE,
    .botbarType = ATBARSCDX,
    .botbarStyle = ATSTYLEBUTTON,
    {.botbarText = ATTXTDETAILS},
    .previousPage = ATPAGDEVICE
  },
  {{ //ATPAGWDGTSTP
    .title = ATTXTWDGSTP},
    .subpages = 1,
    .content = ATCONTFORM,
    .topbarType = ATBARWIDGET,
    .topbarStyle = ATSTYLESMALLTITLE,
    .botbarType = ATBARSCDX,
    .botbarStyle = ATSTYLEBUTTON,
    {.botbarText = ATTXTEXTRA},
    .previousPage = ATPAGCHANNEL
  },
};

const ATFORM AT_devfrm = {
  .elementCnt = 2,
  .elements = {
    {
      .type = ATFRMLABEL,
      .size = 2,
      .row = 1,
      .col = 0,
      .style = ATSTYLEFRMLBLCTR,
    },
    {
      .type = ATFRMTEXT,
      .size = 2,
      .row = 2,
      .col = 0,
      .style = ATSTYLEFRMINPUT,
    },

  }
};

const ATFORM AT_wdgfrm = {
  .elementCnt = 16,
  .elements = {
    {
      .type = ATFRMLABEL,
      .size = 2,
      .row = 0,
      .col = 0,
      .style = ATSTYLEFRMLBLCTR,
    },
    {
      .type = ATFRMTEXT,
      .size = 2,
      .row = 1,
      .col = 0,
      .style = ATSTYLEFRMINPUT,
    },
    {
      .type = ATFRMLABEL,
      .size = 1,
      .row = 2,
      .col = 0,
      .style = ATSTYLEFRMLBL,
    },
    {
      .type = ATFRMSELECT,
      .size = 1,
      .row = 2,
      .col = 1,
      .style = ATSTYLEFRMINPUT,
      .optcnt = 4,
      .optlist = {ATTXTWDSMALL,ATTXTWDLEFT,ATTXTWDRIGHT,ATTXTWDBIG}
    },
    {
      .type = ATFRMLABEL,
      .size = 1,
      .row = 3,
      .col = 0,
      .style = ATSTYLEFRMLBL,
    },
    {
      .type = ATFRMCOLOR,
      .size = 1,
      .row = 3,
      .col = 1,
      .style = 0,
    },
    {
      .type = ATFRMLABEL,
      .size = 1,
      .row = 4,
      .col = 0,
      .style = ATSTYLEFRMLBL,
    },
    {
      .type = ATFRMCOLOR,
      .size = 1,
      .row = 4,
      .col = 1,
      .style = 0,
    },
    {
      .type = ATFRMLABEL,
      .size = 1,
      .row = 5,
      .col = 0,
      .style = ATSTYLEFRMLBL,
    },
    {
      .type = ATFRMCOLOR,
      .size = 1,
      .row = 5,
      .col = 1,
      .style = 0,
    },
    {
      .type = ATFRMLABEL,
      .size = 1,
      .row = 6,
      .col = 0,
      .style = ATSTYLEFRMLBL,
    },
    {
      .type = ATFRMINT,
      .size = 1,
      .row = 6,
      .col = 1,
      .style = ATSTYLEFRMINPUT,
    },
    {
      .type = ATFRMLABEL,
      .size = 1,
      .row = 7,
      .col = 0,
      .style = ATSTYLEFRMLBL,
    },
    {
      .type = ATFRMINT,
      .size = 1,
      .row = 7,
      .col = 1,
      .style = ATSTYLEFRMINPUT,
    },
    {
      .type = ATFRMLABEL,
      .size = 1,
      .row = 8,
      .col = 0,
      .style = ATSTYLEFRMLBL,
    },
    {
      .type = ATFRMINT,
      .size = 1,
      .row = 8,
      .col = 1,
      .style = ATSTYLEFRMINPUT,
    },

  }
};

const String AT_wdgt_size[4] {
  ATTXTWDSMALL,ATTXTWDLEFT,ATTXTWDRIGHT,ATTXTWDBIG
};

const uint16_t AT_palette[64] = {
  0x0000,0x4820,0xa800,0xf800,0x02c0,0x52a0,0xaaa0,0xfa80,
  0x0560,0x5541,0xb520,0xfd40,0x0fc0,0x5fe0,0xa7e0,0xffe1,
  0x000a,0x500a,0xa80a,0xf80a,0x02ea,0x52aa,0xaaac,0xfa8b,
  0x058a,0x554c,0xad4b,0xfd2c,0x07ec,0x57eb,0xb7e9,0xffcc,
  0x0016,0x5015,0xb014,0xf815,0x02d6,0x52b5,0xb295,0xfa94,
  0x0574,0x4d75,0xad54,0xfd73,0x07d5,0x5ff6,0xaff4,0xfff5,
  0x001f,0x501f,0xa81f,0xf81f,0x02be,0x5abf,0xaabf,0xfabf,
  0x053f,0x557f,0xad7f,0xfd9f,0x07df,0x5fdf,0xafff,0xffff
};
