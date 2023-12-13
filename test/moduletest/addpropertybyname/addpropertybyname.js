/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * @tc.name:addpropertybyname
 * @tc.desc:test addpropertybyname function
 * @tc.type: FUNC
 */
let a = {x: 1,x2:2,x3:3,x4:4,x5:5,x6:6,x7:7,x8:8,x9:9,x10:10,x11:11,x12:12,x13:13,x14:14,x15:15,x16:16,x17:17,x18:18,x19:19,x20:20,x21:21,x22:22,x23:23,x24:24,x25:25,x26:26,x27:27,x28:28,x29:29,x30:30,x31:31,x32:32,x33:33,x34:34,x35:35,x36:36,x37:37,x38:38,x39:39,x40:40,x41:41,x42:42,x43:43,x44:44,x45:45,x46:46,x47:47,x48:48,x49:49,x50:50,x51:51,x52:52,x53:53,x54:54,x55:55,x56:56,x57:57,x58:58,x59:59,x60:60,x61:61,x62:62,x63:63,x64:64,x65:65,x66:66,x67:67,x68:68,x69:69,x70:70,x71:71,x72:72,x73:73,x74:74,x75:75,x76:76,x77:77,x78:78,x79:79,x80:80,x81:81,x82:82,x83:83,x84:84,x85:85,x86:86,x87:87,x88:88,x89:89,x90:90,x91:91,x92:92,x93:93,x94:94,x95:95,x96:96,x97:97,x98:98,x99:99,x100:100,x101:101,x102:102,x103:103,x104:104,x105:105,x106:106,x107:107,x108:108,x109:109,x110:110,x111:111,x112:112,x113:113,x114:114,x115:115,x116:116,x117:117,x118:118,x119:119,x120:120,x121:121,x122:122,x123:123,x124:124,x125:125,x126:126,x127:127,x128:128,x129:129,x130:130,x131:131,x132:132,x133:133,x134:134,x135:135,x136:136,x137:137,x138:138,x139:139,x140:140,x141:141,x142:142,x143:143,x144:144,x145:145,x146:146,x147:147,x148:148,x149:149,x150:150,x151:151,x152:152,x153:153,x154:154,x155:155,x156:156,x157:157,x158:158,x159:159,x160:160,x161:161,x162:162,x163:163,x164:164,x165:165,x166:166,x167:167,x168:168,x169:169,x170:170,x171:171,x172:172,x173:173,x174:174,x175:175,x176:176,x177:177,x178:178,x179:179,x180:180,x181:181,x182:182,x183:183,x184:184,x185:185,x186:186,x187:187,x188:188,x189:189,x190:190,x191:191,x192:192,x193:193,x194:194,x195:195,x196:196,x197:197,x198:198,x199:199,x200:200,x201:201,x202:202,x203:203,x204:204,x205:205,x206:206,x207:207,x208:208,x209:209,x210:210,x211:211,x212:212,x213:213,x214:214,x215:215,x216:216,x217:217,x218:218,x219:219,x220:220,x221:221,x222:222,x223:223,x224:224,x225:225,x226:226,x227:227,x228:228,x229:229,x230:230,x231:231,x232:232,x233:233,x234:234,x235:235,x236:236,x237:237,x238:238,x239:239,x240:240,x241:241,x242:242,x243:243,x244:244,x245:245,x246:246,x247:247,x248:248,x249:249,x250:250,x251:251,x252:252,x253:253,x254:254,x255:255,x256:256,x257:257,x258:258,x259:259,x260:260,x261:261,x262:262,x263:263,x264:264,x265:265,x266:266,x267:267,x268:268,x269:269,x270:270,x271:271,x272:272,x273:273,x274:274,x275:275,x276:276,x277:277,x278:278,x279:279,x280:280,x281:281,x282:282,x283:283,x284:284,x285:285,x286:286,x287:287,x288:288,x289:289,x290:290,x291:291,x292:292,x293:293,x294:294,x295:295,x296:296,x297:297,x298:298,x299:299,x300:300,x301:301,x302:302,x303:303,x304:304,x305:305,x306:306,x307:307,x308:308,x309:309,x310:310,x311:311,x312:312,x313:313,x314:314,x315:315,x316:316,x317:317,x318:318,x319:319,x320:320,x321:321,x322:322,x323:323,x324:324,x325:325,x326:326,x327:327,x328:328,x329:329,x330:330,x331:331,x332:332,x333:333,x334:334,x335:335,x336:336,x337:337,x338:338,x339:339,x340:340,x341:341,x342:342,x343:343,x344:344,x345:345,x346:346,x347:347,x348:348,x349:349,x350:350,x351:351,x352:352,x353:353,x354:354,x355:355,x356:356,x357:357,x358:358,x359:359,x360:360,x361:361,x362:362,x363:363,x364:364,x365:365,x366:366,x367:367,x368:368,x369:369,x370:370,x371:371,x372:372,x373:373,x374:374,x375:375,x376:376,x377:377,x378:378,x379:379,x380:380,x381:381,x382:382,x383:383,x384:384,x385:385,x386:386,x387:387,x388:388,x389:389,x390:390,x391:391,x392:392,x393:393,x394:394,x395:395,x396:396,x397:397,x398:398,x399:399,x400:400,x401:401,x402:402,x403:403,x404:404,x405:405,x406:406,x407:407,x408:408,x409:409,x410:410,x411:411,x412:412,x413:413,x414:414,x415:415,x416:416,x417:417,x418:418,x419:419,x420:420,x421:421,x422:422,x423:423,x424:424,x425:425,x426:426,x427:427,x428:428,x429:429,x430:430,x431:431,x432:432,x433:433,x434:434,x435:435,x436:436,x437:437,x438:438,x439:439,x440:440,x441:441,x442:442,x443:443,x444:444,x445:445,x446:446,x447:447,x448:448,x449:449,x450:450,x451:451,x452:452,x453:453,x454:454,x455:455,x456:456,x457:457,x458:458,x459:459,x460:460,x461:461,x462:462,x463:463,x464:464,x465:465,x466:466,x467:467,x468:468,x469:469,x470:470,x471:471,x472:472,x473:473,x474:474,x475:475,x476:476,x477:477,x478:478,x479:479,x480:480,x481:481,x482:482,x483:483,x484:484,x485:485,x486:486,x487:487,x488:488,x489:489,x490:490,x491:491,x492:492,x493:493,x494:494,x495:495,x496:496,x497:497,x498:498,x499:499,x500:500,x501:501,x502:502,x503:503,x504:504,x505:505,x506:506,x507:507,x508:508,x509:509,x510:510,x511:511,x512:512,x513:513,x514:514,x515:515,x516:516,x517:517,x518:518,x519:519,x520:520,x521:521,x522:522,x523:523,x524:524,x525:525,x526:526,x527:527,x528:528,x529:529,x530:530,x531:531,x532:532,x533:533,x534:534,x535:535,x536:536,x537:537,x538:538,x539:539,x540:540,x541:541,x542:542,x543:543,x544:544,x545:545,x546:546,x547:547,x548:548,x549:549,x550:550,x551:551,x552:552,x553:553,x554:554,x555:555,x556:556,x557:557,x558:558,x559:559,x560:560,x561:561,x562:562,x563:563,x564:564,x565:565,x566:566,x567:567,x568:568,x569:569,x570:570,x571:571,x572:572,x573:573,x574:574,x575:575,x576:576,x577:577,x578:578,x579:579,x580:580,x581:581,x582:582,x583:583,x584:584,x585:585,x586:586,x587:587,x588:588,x589:589,x590:590,x591:591,x592:592,x593:593,x594:594,x595:595,x596:596,x597:597,x598:598,x599:599,x600:600,x601:601,x602:602,x603:603,x604:604,x605:605,x606:606,x607:607,x608:608,x609:609,x610:610,x611:611,x612:612,x613:613,x614:614,x615:615,x616:616,x617:617,x618:618,x619:619,x620:620,x621:621,x622:622,x623:623,x624:624,x625:625,x626:626,x627:627,x628:628,x629:629,x630:630,x631:631,x632:632,x633:633,x634:634,x635:635,x636:636,x637:637,x638:638,x639:639,x640:640,x641:641,x642:642,x643:643,x644:644,x645:645,x646:646,x647:647,x648:648,x649:649,x650:650,x651:651,x652:652,x653:653,x654:654,x655:655,x656:656,x657:657,x658:658,x659:659,x660:660,x661:661,x662:662,x663:663,x664:664,x665:665,x666:666,x667:667,x668:668,x669:669,x670:670,x671:671,x672:672,x673:673,x674:674,x675:675,x676:676,x677:677,x678:678,x679:679,x680:680,x681:681,x682:682,x683:683,x684:684,x685:685,x686:686,x687:687,x688:688,x689:689,x690:690,x691:691,x692:692,x693:693,x694:694,x695:695,x696:696,x697:697,x698:698,x699:699,x700:700,x701:701,x702:702,x703:703,x704:704,x705:705,x706:706,x707:707,x708:708,x709:709,x710:710,x711:711,x712:712,x713:713,x714:714,x715:715,x716:716,x717:717,x718:718,x719:719,x720:720,x721:721,x722:722,x723:723,x724:724,x725:725,x726:726,x727:727,x728:728,x729:729,x730:730,x731:731,x732:732,x733:733,x734:734,x735:735,x736:736,x737:737,x738:738,x739:739,x740:740,x741:741,x742:742,x743:743,x744:744,x745:745,x746:746,x747:747,x748:748,x749:749,x750:750,x751:751,x752:752,x753:753,x754:754,x755:755,x756:756,x757:757,x758:758,x759:759,x760:760,x761:761,x762:762,x763:763,x764:764,x765:765,x766:766,x767:767,x768:768,x769:769,x770:770,x771:771,x772:772,x773:773,x774:774,x775:775,x776:776,x777:777,x778:778,x779:779,x780:780,x781:781,x782:782,x783:783,x784:784,x785:785,x786:786,x787:787,x788:788,x789:789,x790:790,x791:791,x792:792,x793:793,x794:794,x795:795,x796:796,x797:797,x798:798,x799:799,x800:800,x801:801,x802:802,x803:803,x804:804,x805:805,x806:806,x807:807,x808:808,x809:809,x810:810,x811:811,x812:812,x813:813,x814:814,x815:815,x816:816,x817:817,x818:818,x819:819,x820:820,x821:821,x822:822,x823:823,x824:824,x825:825,x826:826,x827:827,x828:828,x829:829,x830:830,x831:831,x832:832,x833:833,x834:834,x835:835,x836:836,x837:837,x838:838,x839:839,x840:840,x841:841,x842:842,x843:843,x844:844,x845:845,x846:846,x847:847,x848:848,x849:849,x850:850,x851:851,x852:852,x853:853,x854:854,x855:855,x856:856,x857:857,x858:858,x859:859,x860:860,x861:861,x862:862,x863:863,x864:864,x865:865,x866:866,x867:867,x868:868,x869:869,x870:870,x871:871,x872:872,x873:873,x874:874,x875:875,x876:876,x877:877,x878:878,x879:879,x880:880,x881:881,x882:882,x883:883,x884:884,x885:885,x886:886,x887:887,x888:888,x889:889,x890:890,x891:891,x892:892,x893:893,x894:894,x895:895,x896:896,x897:897,x898:898,x899:899,x900:900,x901:901,x902:902,x903:903,x904:904,x905:905,x906:906,x907:907,x908:908,x909:909,x910:910,x911:911,x912:912,x913:913,x914:914,x915:915,x916:916,x917:917,x918:918,x919:919,x920:920,x921:921,x922:922,x923:923,x924:924,x925:925,x926:926,x927:927,x928:928,x929:929,x930:930,x931:931,x932:932,x933:933,x934:934,x935:935,x936:936,x937:937,x938:938,x939:939,x940:940,x941:941,x942:942,x943:943,x944:944,x945:945,x946:946,x947:947,x948:948,x949:949,x950:950,x951:951,x952:952,x953:953,x954:954,x955:955,x956:956,x957:957,x958:958,x959:959,x960:960,x961:961,x962:962,x963:963,x964:964,x965:965,x966:966,x967:967,x968:968,x969:969,x970:970,x971:971,x972:972,x973:973,x974:974,x975:975,x976:976,x977:977,x978:978,x979:979,x980:980,x981:981,x982:982,x983:983,x984:984,x985:985,x986:986,x987:987,x988:988,x989:989,x990:990,x991:991,x992:992,x993:993,x994:994,x995:995,x996:996,x997:997,x998:998,x999:999,x1000:1000,x1001:1001,x1002:1002,x1003:1003,x1004:1004,x1005:1005,x1006:1006,x1007:1007,x1008:1008,x1009:1009,x1010:1010,x1011:1011,x1012:1012,x1013:1013,x1014:1014,x1015:1015,x1016:1016,x1017:1017,x1018:1018,x1019:1019,x1020:1020,x1021:1021,x1022:1022,x1023:1023};

a.z = 3.3;
print(a.z);