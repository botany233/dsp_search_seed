// Original 2022 Copyright https://github.com/crazyyao0.
// Modified by https://github.com/botany233 on 2025.12
#pragma once

#include <cstdio>
#include <cctype>
#include <set>
#include <string>
#include <array>
#include <string_view>
#include <utility>

#include "defines.hpp"
#include "util.hpp"

template<typename... Args>
inline std::string vformat(std::string_view format,Args... args) {
	const std::string format_string(format);
	const int length = std::snprintf(nullptr,0,format_string.c_str(),args...);
	if(length < 0)
		return {};
	std::string result(length,'\0');
	std::snprintf(result.data(),result.size() + 1,format_string.c_str(),args...);
	return result;
}

inline std::string ReplaceString(std::string subject,const std::string& search, const std::string& replace) {
	size_t pos = 0;
	while((pos = subject.find(search,pos)) != std::string::npos) {
		subject.replace(pos,search.length(),replace);
		pos += replace.length();
	}
	return subject;
}

struct NameGen_t
{
	static constexpr std::array<std::string_view,39> con0{
		"p","t","c","k","b","d","g","f","ph","s",
		"sh","th","h","v","z","th","r","ch","tr","dr",
		"m","n","l","y","w","sp","st","sk","sc","sl",
		"pl","cl","bl","gl","fr","fl","pr","br","cr"
	};

	static constexpr std::array<std::string_view,16> con1{
		"thr","ex","ec","el","er","ev","il","is","it","ir",
		"up","ut","ur","un","gt","phr"
	};

	static constexpr std::array<std::string_view,7> vow0{"a","an","am","al","o","u","xe"};

	static constexpr std::array<std::string_view,23> vow1{
		"ea","ee","ie","i","e","a","er","a","u","oo",
		"u","or","o","oa","ar","a","ei","ai","i","au",
		"ou","ao","ir"
	};

	static constexpr std::array<std::string_view,7> vow2{"y","oi","io","iur","ur","ac","ic"};

	static constexpr std::array<std::string_view,18> ending{
		"er","n","un","or","ar","o","o","ans","us","ix",
		"us","iurs","a","eo","urn","es","eon","y"
	};

	static constexpr std::array<std::string_view,21> roman{
		"","I","II","III","IV","V","VI","VII","VIII","IX",
		"X","XI","XII","XIII","XIV","XV","XVI","XVII","XVIII","XIX",
		"XX"
	};

	static constexpr std::array<std::string_view,88> constellations{
		"Andromedae","Antliae","Apodis","Aquarii","Aquilae","Arae","Arietis","Aurigae","Bootis","Caeli",
		"Camelopardalis","Cancri","Canum Venaticorum","Canis Majoris","Canis Minoris","Capricorni","Carinae","Cassiopeiae","Centauri","Cephei",
		"Ceti","Chamaeleontis","Circini","Columbae","Comae Berenices","Coronae Australis","Coronae Borealis","Corvi","Crateris","Crucis",
		"Cygni","Delphini","Doradus","Draconis","Equulei","Eridani","Fornacis","Geminorum","Gruis","Herculis",
		"Horologii","Hydrae","Hydri","Indi","Lacertae","Leonis","Leonis Minoris","Leporis","Librae","Lupi",
		"Lyncis","Lyrae","Mensae","Microscopii","Monocerotis","Muscae","Normae","Octantis","Ophiuchii","Orionis",
		"Pavonis","Pegasi","Persei","Phoenicis","Pictoris","Piscium","Piscis Austrini","Puppis","Pyxidis","Reticuli",
		"Sagittae","Sagittarii","Scorpii","Sculptoris","Scuti","Serpentis","Sextantis","Tauri","Telescopii","Trianguli",
		"Trianguli Australis","Tucanae","Ursae Majoris","Ursae Minoris","Velorum","Virginis","Volantis","Vulpeculae"
	};

	static constexpr std::array<std::string_view,88> constellations_zhcn{
		"仙女座","唧筒座","天燕座","宝瓶座","天鹰座","天坛座","白羊座","御夫座","牧夫座","雕具座",
		"鹿豹座","巨蟹座","猎犬座","大犬座","小犬座","摩羯座","船底座","仙后座","半人马座","仙王座",
		"鲸鱼座","蝘蜓座","圆规座","天鸽座","后发座","南冕座","北冕座","乌鸦座","巨爵座","南十字座",
		"天鹅座","海豚座","剑鱼座","天龙座","小马座","波江座","天炉座","双子座","天鹤座","武仙座",
		"时钟座","长蛇座","水蛇座","印第安座","蝎虎座","狮子座","小狮座","天兔座","天秤座","豺狼座",
		"天猫座","天琴座","山案座","显微镜座","麒麟座","苍蝇座","矩尺座","南极座","蛇夫座","猎户座",
		"孔雀座","飞马座","英仙座","凤凰座","绘架座","双鱼座","南鱼座","船尾座","罗盘座","网罟座",
		"天箭座","人马座","天蝎座","玉夫座","盾牌座","巨蛇座","六分仪座","金牛座","望远镜座","三角座",
		"南三角座","杜鹃座","大熊座","小熊座","船帆座","室女座","飞鱼座","狐狸座"
	};

	static constexpr std::array<std::string_view,11> alphabeta{
		"Alpha","Beta","Gamma","Delta","Epsilon","Zeta","Eta","Theta","Iota","Kappa",
		"Lambda"
	};

	static constexpr std::array<std::string_view,11> alphabeta_letter{
		"α","β","γ","δ","ε","ζ","η","θ","ι","κ",
		"λ"
	};

	static constexpr std::array<std::string_view,425> raw_star_names{
		"Acamar","Achernar","Achird","Acrab","Acrux","Acubens","Adhafera","Adhara","Adhil","Agena",
		"Aladfar","Albaldah","Albali","Albireo","Alchiba","Alcor","Alcyone","Alderamin","Aldhibain","Aldib",
		"Alfecca","Alfirk","Algedi","Algenib","Algenubi","Algieba","Algjebbath","Algol","Algomeyla","Algorab",
		"Alhajoth","Alhena","Alifa","Alioth","Alkaid","Alkalurops","Alkaphrah","Alkes","Alkhiba","Almach",
		"Almeisan","Almuredin","AlNa'ir","Alnasl","Alnilam","Alnitak","Alniyat","Alphard","Alphecca","Alpheratz",
		"Alrakis","Alrami","Alrescha","AlRijil","Alsahm","Alsciaukat","Alshain","Alshat","Alshemali","Alsuhail",
		"Altair","Altais","Alterf","Althalimain","AlTinnin","Aludra","AlulaAustralis","AlulaBorealis","Alwaid","Alwazn",
		"Alya","Alzirr","AmazonStar","Ancha","Anchat","AngelStern","Angetenar","Ankaa","Anser","Antecanis",
		"Apollo","Arich","Arided","Arietis","Arkab","ArkebPrior","Arneb","Arrioph","AsadAustralis","Ascella",
		"Aschere","AsellusAustralis","AsellusBorealis","AsellusPrimus","Ashtaroth","Asmidiske","Aspidiske","Asterion","Asterope","Asuia",
		"Athafiyy","Atik","Atlas","Atria","Auva","Avior","Azelfafage","Azha","Azimech","BatenKaitos",
		"Becrux","Beid","Bellatrix","Benatnasch","Biham","Botein","Brachium","Bunda","Cajam","Calbalakrab",
		"Calx","Canicula","Capella","Caph","Castor","Castula","Cebalrai","Ceginus","Celaeno","Chara",
		"Chertan","Choo","Clava","CorCaroli","CorHydrae","CorLeonis","Cornu","CorScorpii","CorSepentis","CorTauri",
		"Coxa","Cursa","Cymbae","Cynosaura","Dabih","DenebAlgedi","DenebDulfim","DenebelOkab","DenebKaitos","DenebOkab",
		"Denebola","Dhalim","Dhur","Diadem","Difda","DifdaalAuwel","Dnoces","Dubhe","Dziban","Dzuba",
		"Edasich","ElAcola","Elacrab","Electra","Elgebar","Elgomaisa","ElKaprah","ElKaridab","Elkeid","ElKhereb",
		"Elmathalleth","Elnath","ElPhekrah","Eltanin","Enif","Erakis","Errai","FalxItalica","Fidis","Fomalhaut",
		"Fornacis","FumAlSamakah","Furud","Gacrux","Gallina","GarnetStar","Gemma","Genam","Giausar","GiedePrime",
		"Giedi","Gienah","Gienar","Gildun","Girtab","Gnosia","Gomeisa","Gorgona","Graffias","Hadar",
		"Hamal","Haris","Hasseleh","Hastorang","Hatysa","Heka","Hercules","Heze","Hoedus","Homam",
		"HyadumPrimus","Icalurus","Iclarkrav","Izar","Jabbah","Jewel","Jugum","Juza","Kabeleced","Kaff",
		"Kaffa","Kaffaljidma","Kaitain","KalbalAkrab","Kat","KausAustralis","KausBorealis","KausMedia","Keid","KeKouan",
		"Kelb","Kerb","Kerbel","KiffaBoraelis","Kitalpha","Kochab","Kornephoros","Kraz","Ksora","Kuma",
		"Kurhah","Kursa","Lesath","Maasym","Maaz","Mabsuthat","Maia","Marfik","Markab","Marrha",
		"Matar","Mebsuta","Megres","Meissa","Mekbuda","Menkalinan","Menkar","Menkent","Menkib","Merak",
		"Meres","Merga","Meridiana","Merope","Mesartim","Metallah","Miaplacidus","Mimosa","Minelauva","Minkar",
		"Mintaka","Mirac","Mirach","Miram","Mirfak","Mirzam","Misam","Mismar","Mizar","Muhlifain",
		"Muliphein","Muphrid","Muscida","NairalSaif","NairalZaurak","Naos","Nash","Nashira","Navi","Nekkar",
		"Nicolaus","Nihal","Nodus","Nunki","Nusakan","OculusBoreus","Okda","Osiris","OsPegasi","Palilicium",
		"Peacock","Phact","Phecda","Pherkad","PherkadMinor","Pherkard","Phoenice","Phurad","Pishpai","Pleione",
		"Polaris","Pollux","Porrima","Postvarta","Praecipua","Procyon","Propus","Protrygetor","Pulcherrima","Rana",
		"RanaSecunda","Rasalas","Rasalgethi","Rasalhague","Rasalmothallah","RasHammel","Rastaban","Reda","Regor","Regulus",
		"Rescha","RigilKentaurus","RiglalAwwa","Rotanen","Ruchba","Ruchbah","Rukbat","Rutilicus","Saak","Sabik",
		"Sadachbia","Sadalbari","Sadalmelik","Sadalsuud","Sadatoni","Sadira","Sadr","Saidak","Saiph","Salm",
		"Sargas","Sarin","Sartan","Sceptrum","Scheat","Schedar","Scheddi","Schemali","Scutulum","SeatAlpheras",
		"Segin","Seginus","Shaula","Shedir","Sheliak","Sheratan","Singer","Sirius","Sirrah","Situla",
		"Skat","Spica","Sterope","Subra","Suha","Suhail","SuhailHadar","SuhailRadar","Suhel","Sulafat",
		"Superba","Svalocin","Syrma","Tabit","Tais","Talitha","TaniaAustralis","TaniaBorealis","Tarazed","Tarf",
		"TaTsun","Taygeta","Tegmen","Tejat","TejatPrior","Terebellum","Theemim","Thuban","Tolimann","Tramontana",
		"Tsih","Tureis","Unukalhai","Vega","Venabulum","Venator","Vendemiatrix","Vespertilio","Vildiur","Vindemiatrix",
		"Wasat","Wazn","YedPosterior","YedPrior","Zaniah","Zaurak","Zavijava","ZenithStar","Zibel","Zosma",
		"Zubenelakrab","ZubenElgenubi","Zubeneschamali","ZubenHakrabi","Zubra"
	};

	static constexpr std::array<std::string_view,548> raw_star_names_zhcn{
		"Kat","十字架二","仙女座ξ","天琴座η","天龙座η","天龙座ζ","狐狸座α","天龙座γ","亚马逊星","阿波罗",
		"室女座γ","牧夫座θ","天龙座β","天龙座σ","天鹅座π1","十字架三","宝瓶座ξ","猎犬座β","三角座α","天炉座α",
		"鲸鱼座α","三角座α","十字架三","乌鸦座ε","尼古拉斯星","凤凰座α","金牛座ε","欧西里斯星","室女座γ","天龙座ν",
		"波江座ζ","三角座α","飞马座τ","北门一","北门二","北门三","北门四","北门五","四辅一","四辅二",
		"四辅三","四辅四","勾陈一","勾陈二","勾陈三","勾陈四","勾陈五","勾陈六","天柱一","天柱二",
		"天柱三","天柱四","天柱五","尚书一","尚书二","尚书三","尚书四","尚书五","天床一","天床二",
		"天床三","天床四","天床五","天床六","大理一","大理二","六甲一","六甲二","六甲三","六甲四",
		"六甲五","六甲六","五帝内座一","五帝内座二","五帝内座三","五帝内座四","五帝内座五","华盖一","华盖二","华盖三",
		"华盖四","华盖五","华盖六","华盖七","紫微右垣一","紫微右垣二","紫微右垣三","紫微右垣四","紫微右垣五","紫微右垣六",
		"紫微右垣七","紫微左垣一","紫微左垣二","紫微左垣三","紫微左垣四","紫微左垣五","紫微左垣六","紫微左垣七","紫微左垣八","天乙一",
		"太乙一","内厨一","内厨二","天枢","天璇","天玑","天权","玉衡","开阳","摇光",
		"天枪一","天枪二","天枪三","玄戈一","天理一","天理二","天理三","天理四","太阳守一","太尊一",
		"天牢一","天牢二","天牢三","天牢四","天牢五","天牢六","文昌一","文昌二","文昌三","文昌四",
		"文昌五","文昌六","内阶一","内阶二","内阶三","内阶四","内阶五","内阶六","三师一","三师二",
		"三师三","八谷一","八谷二","八谷三","八谷四","八谷五","八谷六","八谷七","八谷八","传舍一",
		"传舍二","传舍三","传舍四","传舍五","传舍六","传舍七","传舍八","传舍九","天厨一","天厨二",
		"天厨三","天厨四","天厨五","天厨六","天棓一","天棓二","天棓三","天棓四","天棓五","五帝座一",
		"五帝座二","五帝座三","五帝座四","五帝座五","九卿一","九卿二","九卿三","三公一","三公二","三公三",
		"内屏一","内屏二","内屏三","内屏四","太微右垣一","太微右垣二","太微右垣三","太微右垣四","太微右垣五","太微左垣一",
		"太微左垣二","太微左垣三","太微左垣四","太微左垣五","郎位一","郎位二","郎位三","郎位四","郎位五","郎位六",
		"郎位七","郎位八","郎位九","郎位十","郎位十一","郎位十二","郎位十三","郎位十四","郎位十五","常陈一",
		"常陈二","常陈三","常陈四","常陈五","常陈六","常陈七","上台一","上台二","中台一","中台二",
		"下台一","下台二","少微一","少微二","少微三","少微四","长垣一","长垣二","长垣三","长垣四",
		"灵台一","灵台二","灵台三","明堂一","明堂二","明堂三","帝座一","市楼一","市楼二","市楼三",
		"市楼四","市楼五","市楼六","帛度一","帛度二","天市右垣一","天市右垣二","天市右垣三","天市右垣四","天市右垣五",
		"天市右垣六","天市右垣七","天市右垣八","天市右垣九","天市右垣十","天市右垣十一","天市左垣一","天市左垣二","天市左垣三","天市左垣四",
		"天市左垣五","天市左垣六","天市左垣七","天市左垣八","天市左垣九","天市左垣十","天市左垣十一","天纪一","天纪二","天纪三",
		"天纪四","天纪五","天纪六","天纪七","天纪八","天纪九","贯索一","贯索二","贯索三","贯索四",
		"贯索五","贯索六","贯索七","贯索八","贯索九","七公一","七公二","七公三","七公四","七公五",
		"七公六","七公七","角宿一","角宿二","天门一","天门二","南门一","南门二","亢宿一","亢宿二",
		"亢宿三","亢宿四","大角","氐宿一","氐宿二","氐宿三","氐宿四","招摇","房宿一","房宿二",
		"房宿三","房宿四","心宿一","心宿二","心宿三","尾宿一","尾宿二","尾宿三","尾宿四","尾宿五",
		"尾宿六","尾宿七","尾宿八","尾宿九","天河一","天河二","天河三","天河四","箕宿一","箕宿二",
		"箕宿三","箕宿四","斗宿一","斗宿二","斗宿三","斗宿四","斗宿五","斗宿六","牛宿一","牛宿二",
		"牛宿三","牛宿四","牛宿五","牛宿六","织女一","织女二","织女三","女宿一","女宿二","女宿三",
		"女宿四","天津一","天津二","天津三","天津四","奚仲一","奚仲二","奚仲三","奚仲四","奚仲五",
		"虚宿一","虚宿二","司命一","司命二","司危一","司危二","璃瑜一","璃瑜二","璃瑜三","璃瑜四",
		"璃瑜五","危宿一","危宿二","危宿三","天钩一","天钩二","室宿一","室宿二","天纲","北落师门",
		"离宫一","离宫二","壁宿一","壁宿二","奎宿一","奎宿二","奎宿三","奎宿四","奎宿五","奎宿六",
		"奎宿七","奎宿八","奎宿九","奎宿十","奎宿十一","奎宿十二","奎宿十三","奎宿十四","奎宿十五","奎宿十六",
		"王良一","王良二","王良三","王良四","王良五","娄宿一","娄宿二","娄宿三","胃宿一","胃宿二",
		"胃宿三","太陵一","太陵二","太陵三","昴宿一","昴宿二","昴宿三","昴宿四","昴宿五","昴宿六",
		"昴宿七","太阿","毕宿一","毕宿二","毕宿三","毕宿四","毕宿五","毕宿六","毕宿七","毕宿八",
		"五车一","五车二","五车三","五车四","五车五","觜宿一","觜宿二","觜宿三","参宿一","参宿二",
		"参宿三","参宿四","参宿五","参宿六","参宿七","井宿一","井宿二","井宿三","井宿四","井宿五",
		"井宿六","井宿七","井宿八","北河一","北河二","北河三","南河一","南河二","南河三","阙丘一",
		"阙丘二","弧矢一","弧矢二","弧矢三","鬼宿一","鬼宿二","鬼宿三","鬼宿四","柳宿一","柳宿二",
		"柳宿三","柳宿四","柳宿五","柳宿六","柳宿七","柳宿八","星宿一","星宿二","星宿三","星宿四",
		"星宿五","星宿六","星宿七","轩辕一","轩辕二","轩辕三","轩辕四","轩辕五","轩辕六","轩辕七",
		"轩辕八","轩辕九","轩辕十","轩辕十一","轩辕十二","轩辕十三","轩辕十四","轩辕十五","张宿一","张宿二",
		"张宿三","张宿四","张宿五","张宿六","翼宿一","翼宿二","翼宿三","翼宿四","翼宿五","翼宿六",
		"翼宿七","翼宿八","翼宿九","翼宿十","翼宿十一","翼宿十二","翼宿十三","翼宿十四","翼宿十五","翼宿十六",
		"翼宿十七","翼宿十八","翼宿十九","翼宿二十","翼宿二十一","翼宿二十二","天庙","轸宿一","轸宿二","轸宿三",
		"青丘","鹑首","鹑火","鹑尾","阏逢","上章","重光","昭阳","大荒","大渊",
		"重华","九川","羽山","青阳","长嬴","天驷","大辰","北陆"
	};

	static constexpr std::array<std::string_view,60> raw_giant_names{
		"AH Scorpii","Aldebaran","Alpha Herculis","Antares","Arcturus","AV Persei","BC Cygni","Betelgeuse","BI Cygni","BO Carinae",
		"Canopus","CE Tauri","CK Carinae","CW Leonis","Deneb","Epsilon Aurigae","Eta Carinae","EV Carinae","IX Carinae","KW Sagittarii",
		"KY Cygni","Mira","Mu Cephei","NML Cygni","NR Vulpeculae","PZ Cassiopeiae","R Doradus","R Leporis","Rho Cassiopeiae","Rigel",
		"RS Persei","RT Carinae","RU Virginis","RW Cephei","S Cassiopeiae","S Cephei","S Doradus","S Persei","SU Persei","TV Geminorum",
		"U Lacertae","UY Scuti","V1185 Scorpii","V354 Cephei","V355 Cepheus","V382 Carinae","V396 Centauri","V437 Scuti","V509 Cassiopeiae","V528 Carinae",
		"V602 Carinae","V648 Cassiopeiae","V669 Cassiopeiae","V838 Monocerotis","V915 Scorpii","VV Cephei","VX Sagittarii","VY Canis Majoris","WOH G64","XX Persei"
	};

	static constexpr std::array<std::string_view,60> raw_giant_names_zhcn{
		"天蝎座AH","毕宿五","武仙座α","心宿二","大角","英仙座AV","天鹅座BC","参宿四","天鹅座BI","船底座BO",
		"老人星","金牛座CE","船底座CK","狮子座CW","天津四","御夫座ε","船底座η","船底座EV","船底座IX","人马座KW",
		"天鹅座KY","米拉变星","造父四","天鹅座NML","狐狸座NR","仙后座PZ","剑鱼座R","天兔座R","仙后座ρ","参宿七",
		"英仙座RS","船底座RT","室女座RU","仙王座RW","仙后座S","仙王座S","剑鱼座S","英仙座S","英仙座SU","双子座TV",
		"蝎虎座U","盾牌座UY","天蝎座V1185","仙王座V354","仙王座V355","船底座V382","半人马座V396","盾牌座V437","仙后座V509","船底座V528",
		"船底座V602","仙后座V648","仙后座V669","麒麟座V838","天蝎座V915","仙王座VV","人马座VX","大犬座VY","WOH G64","英仙座XX"
	};

    static constexpr std::array<std::string_view,7> giant_name_formats{ "HD %04d%02d", "HDE %04d%02d", "HR %04d", "HV %04d", "LBV %04d-%02d", "NSV %04d", "YSC %04d-%02d" };

    static constexpr std::array<std::string_view,2> neutron_star_name_formats{ "NTR J%02d%02d+%02d", "NTR J%02d%02d-%02d" };

    static constexpr std::array<std::string_view,2> black_hole_name_formats{ "DSR J%02d%02d+%02d", "DSR J%02d%02d-%02d" };
	
    std::string RandomName(int seed)
    {
        DotNet35Random dotNet35Random(seed);
        int num = (int)(dotNet35Random.NextDouble() * 1.8 + 2.3);
        std::string text = "";
        for (int i = 0; i < num; i++)
        {
            if (!(dotNet35Random.NextDouble() < 0.05000000074505806) || i != 0)
            {
                text += ((!(dotNet35Random.NextDouble() < 0.97000002861022949) && num < 4)
					? con1[dotNet35Random.Next(static_cast<int>(con1.size()))]
					: con0[dotNet35Random.Next(static_cast<int>(con0.size()))]);
                text += ((i == num - 1 && dotNet35Random.NextDouble() < 0.89999997615814209)
					? ending[dotNet35Random.Next(static_cast<int>(ending.size()))]
					: ((!(dotNet35Random.NextDouble() < 0.97000002861022949))
					? vow2[dotNet35Random.Next(static_cast<int>(vow2.size()))]
					: vow1[dotNet35Random.Next(static_cast<int>(vow1.size()))]));
            }
            else
            {
                text += vow0[dotNet35Random.Next(static_cast<int>(vow0.size()))];
            }
        }

        text = ReplaceString(text, "uu", "u");
        text = ReplaceString(text, "ooo", "oo");
        text = ReplaceString(text, "eee", "ee");
        text = ReplaceString(text, "eea", "ea");
        text = ReplaceString(text, "aa", "a");
        text = ReplaceString(text, "yy", "y");
        text[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(text[0])));
        return text;
    }

    std::pair<std::string,std::string> RandomStarName(int seed,const StarClass& starData,std::set<std::string>& starnames_enus,std::set<std::string>& starnames_zhcn)
    {
        DotNet35Random dotNet35Random(seed);
        std::pair<std::string,std::string> names{"XStar","XStar"};
        bool found_enus = false;
        bool found_zhcn = false;
        for (int i = 0; i < 256 && (!found_enus || !found_zhcn); i++)
        {
            auto candidate = _RandomStarName(dotNet35Random.Next(), starData);
            if (!found_enus && !starnames_enus.contains(candidate.first))
            {
                names.first = std::move(candidate.first);
                starnames_enus.insert(names.first);
                found_enus = true;
            }
            if (!found_zhcn && !starnames_zhcn.contains(candidate.second))
            {
                names.second = std::move(candidate.second);
                starnames_zhcn.insert(names.second);
                found_zhcn = true;
            }
        }
        return names;
    }

    std::pair<std::string,std::string> _RandomStarName(int seed,const StarClass& starData)
    {
        DotNet35Random dotNet35Random(seed);
        int seed2 = dotNet35Random.Next();
        double num = dotNet35Random.NextDouble();
        double num2 = dotNet35Random.NextDouble();
        if (starData.type == EStarType::GiantStar)
        {
            if (num2 < 0.40000000596046448)
            {
                const auto names = RandomGiantStarNameFromRawNames(seed2);
                return {std::string(names.first),std::string(names.second)};
            }
            if (num2 < 0.699999988079071)
            {
                return RandomGiantStarNameWithConstellationAlpha(seed2);
            }
            const std::string name = RandomGiantStarNameWithFormat(seed2);
            return {name,name};
        }
        if (starData.type == EStarType::NeutronStar)
        {
            const std::string name = RandomNeutronStarNameWithFormat(seed2);
            return {name,name};
        }
        if (starData.type == EStarType::BlackHole)
        {
            const std::string name = RandomBlackHoleNameWithFormat(seed2);
            return {name,name};
        }
        if (num < 0.60000002384185791)
        {
            const auto names = RandomStarNameFromRawNames(seed2);
            return {std::string(names.first),std::string(names.second)};
        }
        if (num < 0.93000000715255737)
        {
            return RandomStarNameWithConstellationAlpha(seed2);
        }
        return RandomStarNameWithConstellationNumber(seed2);
    }

    std::pair<std::string_view,std::string_view> RandomStarNameFromRawNames(int seed)
    {
        int num = DotNet35Random(seed).Next();
        return {raw_star_names[num % raw_star_names.size()],raw_star_names_zhcn[num % raw_star_names_zhcn.size()]};
    }

    std::pair<std::string,std::string> RandomStarNameWithConstellationAlpha(int seed)
    {
        DotNet35Random dotNet35Random(seed);
        int num = dotNet35Random.Next();
        int num2 = dotNet35Random.Next();
        num2 %= alphabeta.size();
        int num_zhcn = dotNet35Random.Next();
        num_zhcn %= constellations_zhcn.size();
        const std::string_view text = constellations[num % constellations.size()];
        std::string name_enus;
        if (text.size() > 10)
        {
            name_enus = std::string(alphabeta_letter[num2]) + " " + std::string(text);
        }
        else
        {
            name_enus = std::string(alphabeta[num2]) + " " + std::string(text);
        }
        return {name_enus,std::string(constellations_zhcn[num_zhcn]) + std::string(alphabeta_letter[num2])};
    }

    std::pair<std::string,std::string> RandomStarNameWithConstellationNumber(int seed)
    {
        DotNet35Random dotNet35Random(seed);
        int num = dotNet35Random.Next();
        int num2 = dotNet35Random.Next(27, 75);
        int num_zhcn = dotNet35Random.Next();
        num_zhcn %= constellations_zhcn.size();
        const std::string number = std::to_string(num2);
        return {number + " " + std::string(constellations[num % constellations.size()]),
            std::string(constellations_zhcn[num_zhcn]) + " " + number};
    }

    std::pair<std::string_view,std::string_view> RandomGiantStarNameFromRawNames(int seed)
    {
        int num = DotNet35Random(seed).Next();
        return {raw_giant_names[num % raw_giant_names.size()],raw_giant_names_zhcn[num % raw_giant_names_zhcn.size()]};
    }

    std::pair<std::string,std::string> RandomGiantStarNameWithConstellationAlpha(int seed)
    {
        DotNet35Random dotNet35Random(seed);
        int num = dotNet35Random.Next();
        int num2 = dotNet35Random.Next(15, 26);
        int num3 = dotNet35Random.Next(0, 26);
        char c = static_cast<char>('A' + num2);
        char c2 = static_cast<char>('A' + num3);
        const std::string letters{c,c2};
        return {letters + " " + std::string(constellations[num % constellations.size()]),
            std::string(constellations_zhcn[num % constellations_zhcn.size()]) + letters};
    }

    std::string RandomGiantStarNameWithFormat(int seed)
    {
        DotNet35Random dotNet35Random(seed);
        int num = dotNet35Random.Next();
        int num2 = dotNet35Random.Next(10000);
        int num3 = dotNet35Random.Next(100);
        num %= giant_name_formats.size();
        
        return vformat(giant_name_formats[num], num2, num3);
    }

    std::string RandomNeutronStarNameWithFormat(int seed)
    {
        DotNet35Random dotNet35Random(seed);
        int num = dotNet35Random.Next();
        int num2 = dotNet35Random.Next(24);
        int num3 = dotNet35Random.Next(60);
        int num4 = dotNet35Random.Next(0, 60);
        num %= neutron_star_name_formats.size();
        return vformat(neutron_star_name_formats[num], num2, num3, num4);
    }

    std::string RandomBlackHoleNameWithFormat(int seed)
    {
        DotNet35Random dotNet35Random(seed);
        int num = dotNet35Random.Next();
        int num2 = dotNet35Random.Next(24);
        int num3 = dotNet35Random.Next(60);
        int num4 = dotNet35Random.Next(0, 60);
        num %= black_hole_name_formats.size();
        return vformat(black_hole_name_formats[num], num2, num3, num4);
    }
};

extern NameGen_t NameGen;
