/* Produced by CVXGEN, 2018-03-26 08:37:22 -0400.  */
/* CVXGEN is Copyright (C) 2006-2017 Jacob Mattingley, jem@cvxgen.com. */
/* The code in this file is Copyright (C) 2006-2017 Jacob Mattingley. */
/* CVXGEN, or solvers produced by CVXGEN, cannot be used for commercial */
/* applications without prior written permission from Jacob Mattingley. */

/* Filename: testsolver.c. */
/* Description: Basic test harness for solver.c. */
#include "solver.h"
Vars vars;
Params params;
Workspace work;
Settings settings;
#define NUMTESTS 0
int main(int argc, char **argv) {
  int num_iters;
#if (NUMTESTS > 0)
  int i;
  double time;
  double time_per;
#endif
  set_defaults();
  setup_indexing();
  load_default_data();
  /* Solve problem instance for the record. */
  settings.verbose = 1;
  num_iters = solve();
#ifndef ZERO_LIBRARY_MODE
#if (NUMTESTS > 0)
  /* Now solve multiple problem instances for timing purposes. */
  settings.verbose = 0;
  tic();
  for (i = 0; i < NUMTESTS; i++) {
    solve();
  }
  time = tocq();
  printf("Timed %d solves over %.3f seconds.\n", NUMTESTS, time);
  time_per = time / NUMTESTS;
  if (time_per > 1) {
    printf("Actual time taken per solve: %.3g s.\n", time_per);
  } else if (time_per > 1e-3) {
    printf("Actual time taken per solve: %.3g ms.\n", 1e3*time_per);
  } else {
    printf("Actual time taken per solve: %.3g us.\n", 1e6*time_per);
  }
#endif
#endif
  return 0;
}
void load_default_data(void) {
  params.x_ss_1[0] = 0.20319161029830202;
  params.x_ss_1[1] = 0.8325912904724193;
  params.x_ss_1[2] = -0.8363810443482227;
  params.x_ss_1[3] = 0.04331042079065206;
  params.x_ss_1[4] = 1.5717878173906188;
  params.x_ss_1[5] = 1.5851723557337523;
  params.Q[0] = 1.1255853104638363;
  params.Q[1] = 1.2072428781381868;
  params.Q[2] = 1.0514672033008299;
  params.Q[3] = 1.4408098436506365;
  params.Q[4] = 1.0298762108785668;
  params.Q[5] = 1.456833224394711;
  params.x_ss_2[0] = 0.596576190459043;
  params.x_ss_2[1] = -0.8860508694080989;
  params.x_ss_2[2] = 0.7050196079205251;
  params.x_ss_2[3] = 0.3634512696654033;
  params.x_ss_2[4] = -1.9040724704913385;
  params.x_ss_2[5] = 0.23541635196352795;
  params.x_ss_3[0] = -0.9629902123701384;
  params.x_ss_3[1] = -0.3395952119597214;
  params.x_ss_3[2] = -0.865899672914725;
  params.x_ss_3[3] = 0.7725516732519853;
  params.x_ss_3[4] = -0.23818512931704205;
  params.x_ss_3[5] = -1.372529046100147;
  params.x_ss_4[0] = 0.17859607212737894;
  params.x_ss_4[1] = 1.1212590580454682;
  params.x_ss_4[2] = -0.774545870495281;
  params.x_ss_4[3] = -1.1121684642712744;
  params.x_ss_4[4] = -0.44811496977740495;
  params.x_ss_4[5] = 1.7455345994417217;
  params.x_ss_5[0] = 1.9039816898917352;
  params.x_ss_5[1] = 0.6895347036512547;
  params.x_ss_5[2] = 1.6113364341535923;
  params.x_ss_5[3] = 1.383003485172717;
  params.x_ss_5[4] = -0.48802383468444344;
  params.x_ss_5[5] = -1.631131964513103;
  params.x_ss_6[0] = 0.6136436100941447;
  params.x_ss_6[1] = 0.2313630495538037;
  params.x_ss_6[2] = -0.5537409477496875;
  params.x_ss_6[3] = -1.0997819806406723;
  params.x_ss_6[4] = -0.3739203344950055;
  params.x_ss_6[5] = -0.12423900520332376;
  params.x_ss_7[0] = -0.923057686995755;
  params.x_ss_7[1] = -0.8328289030982696;
  params.x_ss_7[2] = -0.16925440270808823;
  params.x_ss_7[3] = 1.442135651787706;
  params.x_ss_7[4] = 0.34501161787128565;
  params.x_ss_7[5] = -0.8660485502711608;
  params.x_ss_8[0] = -0.8880899735055947;
  params.x_ss_8[1] = -0.1815116979122129;
  params.x_ss_8[2] = -1.17835862158005;
  params.x_ss_8[3] = -1.1944851558277074;
  params.x_ss_8[4] = 0.05614023926976763;
  params.x_ss_8[5] = -1.6510825248767813;
  params.x_ss_9[0] = -0.06565787059365391;
  params.x_ss_9[1] = -0.5512951504486665;
  params.x_ss_9[2] = 0.8307464872626844;
  params.x_ss_9[3] = 0.9869848924080182;
  params.x_ss_9[4] = 0.7643716874230573;
  params.x_ss_9[5] = 0.7567216550196565;
  params.x_ss_10[0] = -0.5055995034042868;
  params.x_ss_10[1] = 0.6725392189410702;
  params.x_ss_10[2] = -0.6406053441727284;
  params.x_ss_10[3] = 0.29117547947550015;
  params.x_ss_10[4] = -0.6967713677405021;
  params.x_ss_10[5] = -0.21941980294587182;
  params.x_ss_11[0] = -1.753884276680243;
  params.x_ss_11[1] = -1.0292983112626475;
  params.x_ss_11[2] = 1.8864104246942706;
  params.x_ss_11[3] = -1.077663182579704;
  params.x_ss_11[4] = 0.7659100437893209;
  params.x_ss_11[5] = 0.6019074328549583;
  params.x_ss_12[0] = 0.8957565577499285;
  params.x_ss_12[1] = -0.09964555746227477;
  params.x_ss_12[2] = 0.38665509840745127;
  params.x_ss_12[3] = -1.7321223042686946;
  params.x_ss_12[4] = -1.7097514487110663;
  params.x_ss_12[5] = -1.2040958948116867;
  params.x_ss_13[0] = -1.3925560119658358;
  params.x_ss_13[1] = -1.5995826216742213;
  params.x_ss_13[2] = -1.4828245415645833;
  params.x_ss_13[3] = 0.21311092723061398;
  params.x_ss_13[4] = -1.248740700304487;
  params.x_ss_13[5] = 1.808404972124833;
  params.x_ss_14[0] = 0.7264471152297065;
  params.x_ss_14[1] = 0.16407869343908477;
  params.x_ss_14[2] = 0.8287224032315907;
  params.x_ss_14[3] = -0.9444533161899464;
  params.x_ss_14[4] = 1.7069027370149112;
  params.x_ss_14[5] = 1.3567722311998827;
  params.x_ss_15[0] = 0.9052779937121489;
  params.x_ss_15[1] = -0.07904017565835986;
  params.x_ss_15[2] = 1.3684127435065871;
  params.x_ss_15[3] = 0.979009293697437;
  params.x_ss_15[4] = 0.6413036255984501;
  params.x_ss_15[5] = 1.6559010680237511;
  params.x_ss_16[0] = 0.5346622551502991;
  params.x_ss_16[1] = -0.5362376605895625;
  params.x_ss_16[2] = 0.2113782926017822;
  params.x_ss_16[3] = -1.2144776931994525;
  params.x_ss_16[4] = -1.2317108144255875;
  params.x_ss_16[5] = 0.9026784957312834;
  params.x_ss_17[0] = 1.1397468137245244;
  params.x_ss_17[1] = 1.8883934547350631;
  params.x_ss_17[2] = 1.4038856681660068;
  params.x_ss_17[3] = 0.17437730638329096;
  params.x_ss_17[4] = -1.6408365219077408;
  params.x_ss_17[5] = -0.04450702153554875;
  params.x_ss_18[0] = 1.7117453902485025;
  params.x_ss_18[1] = 1.1504727980139053;
  params.x_ss_18[2] = -0.05962309578364744;
  params.x_ss_18[3] = -0.1788825540764547;
  params.x_ss_18[4] = -1.1280569263625857;
  params.x_ss_18[5] = -1.2911464767927057;
  params.x_ss_19[0] = -1.7055053231225696;
  params.x_ss_19[1] = 1.56957275034837;
  params.x_ss_19[2] = 0.5607064675962357;
  params.x_ss_19[3] = -1.4266707301147146;
  params.x_ss_19[4] = -0.3434923211351708;
  params.x_ss_19[5] = -1.8035643024085055;
  params.x_ss_20[0] = -1.1625066019105454;
  params.x_ss_20[1] = 0.9228324965161532;
  params.x_ss_20[2] = 0.6044910817663975;
  params.x_ss_20[3] = -0.0840868104920891;
  params.x_ss_20[4] = -0.900877978017443;
  params.x_ss_20[5] = 0.608892500264739;
  params.x_ss_21[0] = 1.8257980452695217;
  params.x_ss_21[1] = -0.25791777529922877;
  params.x_ss_21[2] = -1.7194699796493191;
  params.x_ss_21[3] = -1.7690740487081298;
  params.x_ss_21[4] = -1.6685159248097703;
  params.x_ss_21[5] = 1.8388287490128845;
  params.x_ss_22[0] = 0.16304334474597537;
  params.x_ss_22[1] = 1.3498497306788897;
  params.x_ss_22[2] = -1.3198658230514613;
  params.x_ss_22[3] = -0.9586197090843394;
  params.x_ss_22[4] = 0.7679100474913709;
  params.x_ss_22[5] = 1.5822813125679343;
  params.x_ss_23[0] = -0.6372460621593619;
  params.x_ss_23[1] = -1.741307208038867;
  params.x_ss_23[2] = 1.456478677642575;
  params.x_ss_23[3] = -0.8365102166820959;
  params.x_ss_23[4] = 0.9643296255982503;
  params.x_ss_23[5] = -1.367865381194024;
  params.x_ss_24[0] = 0.7798537405635035;
  params.x_ss_24[1] = 1.3656784761245926;
  params.x_ss_24[2] = 0.9086083149868371;
  params.x_ss_24[3] = -0.5635699005460344;
  params.x_ss_24[4] = 0.9067590059607915;
  params.x_ss_24[5] = -1.4421315032701587;
  params.x_ss_25[0] = -0.7447235390671119;
  params.x_ss_25[1] = -0.32166897326822186;
  params.x_ss_25[2] = 1.5088481557772684;
  params.x_ss_25[3] = -1.385039165715428;
  params.x_ss_25[4] = 1.5204991609972622;
  params.x_ss_25[5] = 1.1958572768832156;
  params.x_ss_26[0] = 1.8864971883119228;
  params.x_ss_26[1] = -0.5291880667861584;
  params.x_ss_26[2] = -1.1802409243688836;
  params.x_ss_26[3] = -1.037718718661604;
  params.x_ss_26[4] = 1.3114512056856835;
  params.x_ss_26[5] = 1.8609125943756615;
  params.x_ss_27[0] = 0.7952399935216938;
  params.x_ss_27[1] = -0.07001183290468038;
  params.x_ss_27[2] = -0.8518009412754686;
  params.x_ss_27[3] = 1.3347515373726386;
  params.x_ss_27[4] = 1.4887180335977037;
  params.x_ss_27[5] = -1.6314736327976336;
  params.x_ss_28[0] = -1.1362021159208933;
  params.x_ss_28[1] = 1.327044361831466;
  params.x_ss_28[2] = 1.3932155883179842;
  params.x_ss_28[3] = -0.7413880049440107;
  params.x_ss_28[4] = -0.8828216126125747;
  params.x_ss_28[5] = -0.27673991192616;
  params.x_ss_29[0] = 0.15778600105866714;
  params.x_ss_29[1] = -1.6177327399735457;
  params.x_ss_29[2] = 1.3476485548544606;
  params.x_ss_29[3] = 0.13893948140528378;
  params.x_ss_29[4] = 1.0998712601636944;
  params.x_ss_29[5] = -1.0766549376946926;
  params.x_ss_30[0] = 1.8611734044254629;
  params.x_ss_30[1] = 1.0041092292735172;
  params.x_ss_30[2] = -0.6276245424321543;
  params.x_ss_30[3] = 1.794110587839819;
  params.x_ss_30[4] = 0.8020471158650913;
  params.x_ss_30[5] = 1.362244341944948;
  params.x_ss_31[0] = -1.8180107765765245;
  params.x_ss_31[1] = -1.7774338357932473;
  params.x_ss_31[2] = 0.9709490941985153;
  params.x_ss_31[3] = -0.7812542682064318;
  params.x_ss_31[4] = 0.0671374633729811;
  params.x_ss_31[5] = -1.374950305314906;
  params.x_ss_32[0] = 1.9118096386279388;
  params.x_ss_32[1] = 0.011004190697677885;
  params.x_ss_32[2] = 1.3160043138989015;
  params.x_ss_32[3] = -1.7038488148800144;
  params.x_ss_32[4] = -0.08433819112864738;
  params.x_ss_32[5] = -1.7508820783768964;
  params.x_ss_33[0] = 1.536965724350949;
  params.x_ss_33[1] = -0.21675928514816478;
  params.x_ss_33[2] = -1.725800326952653;
  params.x_ss_33[3] = -1.6940148707361717;
  params.x_ss_33[4] = 0.15517063201268;
  params.x_ss_33[5] = -1.697734381979077;
  params.x_ss_34[0] = -1.264910727950229;
  params.x_ss_34[1] = -0.2545716633339441;
  params.x_ss_34[2] = -0.008868675926170244;
  params.x_ss_34[3] = 0.3332476609670296;
  params.x_ss_34[4] = 0.48205072561962936;
  params.x_ss_34[5] = -0.5087540014293261;
  params.x_ss_35[0] = 0.4749463319223195;
  params.x_ss_35[1] = -1.371021366459455;
  params.x_ss_35[2] = -0.8979660982652256;
  params.x_ss_35[3] = 1.194873082385242;
  params.x_ss_35[4] = -1.3876427970939353;
  params.x_ss_35[5] = -1.106708108457053;
  params.x_ss_36[0] = -1.0280872812241797;
  params.x_ss_36[1] = -0.08197078070773234;
  params.x_ss_36[2] = -1.9970179118324083;
  params.x_ss_36[3] = -1.878754557910134;
  params.x_ss_36[4] = -0.15380739340877803;
  params.x_ss_36[5] = -1.349917260533923;
  params.x_ss_37[0] = 0.7180072150931407;
  params.x_ss_37[1] = 1.1808183487065538;
  params.x_ss_37[2] = 0.31265343495084075;
  params.x_ss_37[3] = 0.7790599086928229;
  params.x_ss_37[4] = -0.4361679370644853;
  params.x_ss_37[5] = -1.8148151880282066;
  params.x_ss_38[0] = -0.24231386948140266;
  params.x_ss_38[1] = -0.5120787511622411;
  params.x_ss_38[2] = 0.3880129688013203;
  params.x_ss_38[3] = -1.4631273212038676;
  params.x_ss_38[4] = -1.0891484131126563;
  params.x_ss_38[5] = 1.2591296661091191;
  params.x_ss_39[0] = -0.9426978934391474;
  params.x_ss_39[1] = -0.358719180371347;
  params.x_ss_39[2] = 1.7438887059831263;
  params.x_ss_39[3] = -0.8977901479165817;
  params.x_ss_39[4] = -1.4188401645857445;
  params.x_ss_39[5] = 0.8080805173258092;
  params.x_ss_40[0] = 0.2682662017650985;
  params.x_ss_40[1] = 0.44637534218638786;
  params.x_ss_40[2] = -1.8318765960257055;
  params.x_ss_40[3] = -0.3309324209710929;
  params.x_ss_40[4] = -1.9829342633313622;
  params.x_ss_40[5] = -1.013858124556442;
  params.R[0] = 1.7060561835840065;
  params.R[1] = 1.0615407159206998;
  params.R2[0] = 1.2946934986032799;
  params.R2[1] = 1.988112752812178;
  params.Af[0] = 1.884888920907902;
  params.Af[1] = -0.0726144452811801;
  params.Af[2] = 0.9427735461129836;
  params.Af[3] = 0.5306230967445558;
  params.Af[4] = -0.1372277142250531;
  params.Af[5] = 1.4282657305652786;
  params.Af[6] = -1.309926991335284;
  params.Af[7] = 1.3137276889764422;
  params.Af[8] = -1.8317219061667278;
  params.Af[9] = 1.4678147672511939;
  params.x_0[0] = 0.703986349872991;
  params.x_0[1] = -0.2163435603565258;
  params.x_0[2] = 0.6862809905371079;
  params.x_0[3] = -0.15852598444303245;
  params.x_0[4] = 1.1200128895143409;
  params.x_0[5] = -1.5462236645435308;
  params.Bf[0] = 0.0326297153944215;
  params.Bf[1] = 1.4859581597754916;
  params.A[0] = 1.71011710324809;
  params.A[1] = -1.1186546738067493;
  params.A[2] = -0.9922787897815244;
  params.A[3] = 1.6160498864359547;
  params.A[4] = -0.6179306451394861;
  params.A[5] = -1.7725097038051376;
  params.A[6] = 0.8595466884481313;
  params.A[7] = -0.3423245633865686;
  params.A[8] = 0.9412967499805762;
  params.A[9] = -0.09163346622652258;
  params.B[0] = 0.002262217745727657;
  params.B[1] = -0.3297523583656421;
  params.x_max_3_1[0] = -0.8380604158593941;
  params.x_max_3_2[0] = 1.6028434695494038;
  params.x_max_3_3[0] = 0.675150311940429;
  params.x_max_3_4[0] = 1.1553293733718686;
  params.x_max_3_5[0] = 1.5829581243724693;
  params.x_max_3_6[0] = -0.9992442304425597;
  params.x_max_3_7[0] = 1.6792824558896897;
  params.x_max_3_8[0] = 1.4504203490342324;
  params.x_max_3_9[0] = 0.02434104849994556;
  params.x_max_3_10[0] = 0.27160869657612263;
  params.x_max_3_11[0] = -1.5402710478528858;
  params.x_max_3_12[0] = 1.0484633622310744;
  params.x_max_3_13[0] = -1.3070999712627054;
  params.x_max_3_14[0] = 0.13534416402363814;
  params.x_max_3_15[0] = -1.4942507790851232;
  params.x_max_3_16[0] = -1.708331625671371;
  params.x_max_3_17[0] = 0.436109775042258;
  params.x_max_3_18[0] = -0.03518748153727991;
  params.x_max_3_19[0] = 0.6992397389570906;
  params.x_max_3_20[0] = 1.1634167322171374;
  params.x_max_3_21[0] = 1.9307499705822648;
  params.x_max_3_22[0] = -1.6636772756932747;
  params.x_max_3_23[0] = 0.5248484497343218;
  params.x_max_3_24[0] = 0.30789958152579144;
  params.x_max_3_25[0] = 0.602568707166812;
  params.x_max_3_26[0] = 0.17271781925751872;
  params.x_max_3_27[0] = 0.2294695501208066;
  params.x_max_3_28[0] = 1.4742185345619543;
  params.x_max_3_29[0] = -0.1919535345136989;
  params.x_max_3_30[0] = 0.13990231452144553;
  params.x_max_3_31[0] = 0.7638548150610602;
  params.x_max_3_32[0] = -1.6420200344195646;
  params.x_max_3_33[0] = -0.27229872445076087;
  params.x_max_3_34[0] = -1.5914631171820468;
  params.x_max_3_35[0] = -1.4487604283558668;
  params.x_max_3_36[0] = -1.991497766136364;
  params.x_max_3_37[0] = -1.1611742553535152;
  params.x_max_3_38[0] = -1.133450950247063;
  params.x_max_3_39[0] = 0.06497792493777155;
  params.x_max_3_40[0] = 0.28083295396097263;
  params.x_max_2_1[0] = 1.2958447220129887;
  params.x_max_2_2[0] = -0.05315524470737154;
  params.x_max_2_3[0] = 1.5658183956871667;
  params.x_max_2_4[0] = -0.41975684089933685;
  params.x_max_2_5[0] = 0.97844578833777;
  params.x_max_2_6[0] = 0.2110290496695293;
  params.x_max_2_7[0] = 0.4953003430893044;
  params.x_max_2_8[0] = -0.9184320124667495;
  params.x_max_2_9[0] = 1.750380031759156;
  params.x_max_2_10[0] = 1.0786188614315915;
  params.x_max_2_11[0] = -1.4176198837203735;
  params.x_max_2_12[0] = 0.149737479778294;
  params.x_max_2_13[0] = 1.9831452222223418;
  params.x_max_2_14[0] = -1.8037746699794734;
  params.x_max_2_15[0] = -0.7887206483295461;
  params.x_max_2_16[0] = 0.9632534854086652;
  params.x_max_2_17[0] = -1.8425542093895406;
  params.x_max_2_18[0] = 0.986684363969033;
  params.x_max_2_19[0] = 0.2936851199350441;
  params.x_max_2_20[0] = 0.9268227022482662;
  params.x_max_2_21[0] = 0.20333038350653299;
  params.x_max_2_22[0] = 1.7576139132046351;
  params.x_max_2_23[0] = -0.614393188398918;
  params.x_max_2_24[0] = 0.297877839744912;
  params.x_max_2_25[0] = -1.796880083990895;
  params.x_max_2_26[0] = 0.21373133661742738;
  params.x_max_2_27[0] = -0.32242822540825156;
  params.x_max_2_28[0] = 1.9326471511608059;
  params.x_max_2_29[0] = 1.7824292753481785;
  params.x_max_2_30[0] = -1.4468823405675986;
  params.x_max_2_31[0] = -1.8335374338761512;
  params.x_max_2_32[0] = -1.5172997317243713;
  params.x_max_2_33[0] = -1.229012129120719;
  params.x_max_2_34[0] = 0.9046719772422094;
  params.x_max_2_35[0] = 0.17591181415489432;
  params.x_max_2_36[0] = 0.13970133814112584;
  params.x_max_2_37[0] = -0.14185208214985234;
  params.x_max_2_38[0] = -1.9732231264739348;
  params.x_max_2_39[0] = -0.4301123458221334;
  params.x_max_2_40[0] = 1.9957537650387742;
  params.u_max[0] = 1.2811648216477893;
}
