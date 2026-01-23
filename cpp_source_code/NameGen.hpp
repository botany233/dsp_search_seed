// Original 2022 Copyright https://github.com/crazyyao0.
// Modified by https://github.com/botany233 on 2025.12
#pragma once

#include "defines.hpp"
#include "util.hpp"
#include <string.h>
#include <cstdarg>
#include <set>

const std::string vformat(const char* const zcFormat, ...) {
    va_list vaArgs;
    va_start(vaArgs, zcFormat);
    va_list vaArgsCopy;
    va_copy(vaArgsCopy, vaArgs);
    const int iLen = std::vsnprintf(NULL, 0, zcFormat, vaArgsCopy);
    va_end(vaArgsCopy);
    std::vector<char> zc(iLen + 1);
    std::vsnprintf(zc.data(), zc.size(), zcFormat, vaArgs);
    va_end(vaArgs);
    return std::string(zc.data(), iLen);
}

std::string ReplaceString(std::string subject, const std::string& search,
    const std::string& replace) {
    size_t pos = 0;
    while ((pos = subject.find(search, pos)) != std::string::npos) {
        subject.replace(pos, search.length(), replace);
        pos += replace.length();
    }
    return subject;
}

struct NameGen_t
{
    const char* con0[39]{
    "p", "t", "c", "k", "b", "d", "g", "f", "ph", "s",
    "sh", "th", "h", "v", "z", "th", "r", "ch", "tr", "dr",
    "m", "n", "l", "y", "w", "sp", "st", "sk", "sc", "sl",
    "pl", "cl", "bl", "gl", "fr", "fl", "pr", "br", "cr"
    };
    int con0Length = 39;

    const char* con1[16]{
    "thr", "ex", "ec", "el", "er", "ev", "il", "is", "it", "ir",
    "up", "ut", "ur", "un", "gt", "phr"
    };
    int con1Length = 16;

    const char* vow0[7]{ "a", "an", "am", "al", "o", "u", "xe" };
    int vow0Length = 7;

    const char* vow1[23]
    {
    "ea", "ee", "ie", "i", "e", "a", "er", "a", "u", "oo",
    "u", "or", "o", "oa", "ar", "a", "ei", "ai", "i", "au",
    "ou", "ao", "ir"
    };
    int vow1Length = 23;

    const char* vow2[7]{ "y", "oi", "io", "iur", "ur", "ac", "ic" };
    int vow2Length = 7;

    const char* ending[18]
    {
    "er", "n", "un", "or", "ar", "o", "o", "ans", "us", "ix",
    "us", "iurs", "a", "eo", "urn", "es", "eon", "y"
    };
    int endingLength = 18;

    const char* roman[21]
    {
    "", "I", "II", "III", "IV", "V", "VI", "VII", "VIII", "IX",
    "X", "XI", "XII", "XIII", "XIV", "XV", "XVI", "XVII", "XVIII", "XIX",
    "XX"
    };
    int romanLength = 21;

    const char* constellations[88]
    {
    "Andromedae", "Antliae", "Apodis", "Aquarii", "Aquilae", "Arae", "Arietis", "Aurigae", "Bootis", "Caeli",
    "Camelopardalis", "Cancri", "Canum Venaticorum", "Canis Majoris", "Canis Minoris", "Capricorni", "Carinae", "Cassiopeiae", "Centauri", "Cephei",
    "Ceti", "Chamaeleontis", "Circini", "Columbae", "Comae Berenices", "Coronae Australis", "Coronae Borealis", "Corvi", "Crateris", "Crucis",
    "Cygni", "Delphini", "Doradus", "Draconis", "Equulei", "Eridani", "Fornacis", "Geminorum", "Gruis", "Herculis",
    "Horologii", "Hydrae", "Hydri", "Indi", "Lacertae", "Leonis", "Leonis Minoris", "Leporis", "Librae", "Lupi",
    "Lyncis", "Lyrae", "Mensae", "Microscopii", "Monocerotis", "Muscae", "Normae", "Octantis", "Ophiuchii", "Orionis",
    "Pavonis", "Pegasi", "Persei", "Phoenicis", "Pictoris", "Piscium", "Piscis Austrini", "Puppis", "Pyxidis", "Reticuli",
    "Sagittae", "Sagittarii", "Scorpii", "Sculptoris", "Scuti", "Serpentis", "Sextantis", "Tauri", "Telescopii", "Trianguli",
    "Trianguli Australis", "Tucanae", "Ursae Majoris", "Ursae Minoris", "Velorum", "Virginis", "Volantis", "Vulpeculae"
    };
    int constellationsLength = 88;

    const char* alphabeta[11]
    {
    "Alpha", "Beta", "Gamma", "Delta", "Epsilon", "Zeta", "Eta", "Theta", "Iota", "Kappa",
    "Lambda"
    };
    int alphabetaLength = 11;

    const char* alphabeta_letter[11]
    {
    "α", "β", "γ", "δ", "ε", "ζ", "η", "θ", "ι", "κ",
    "λ"
    };
    int alphabeta_letterLength = 11;

    const char* raw_star_names[425]
    {
    "Acamar", "Achernar", "Achird", "Acrab", "Acrux", "Acubens", "Adhafera", "Adhara", "Adhil", "Agena",
    "Aladfar", "Albaldah", "Albali", "Albireo", "Alchiba", "Alcor", "Alcyone", "Alderamin", "Aldhibain", "Aldib",
    "Alfecca", "Alfirk", "Algedi", "Algenib", "Algenubi", "Algieba", "Algjebbath", "Algol", "Algomeyla", "Algorab",
    "Alhajoth", "Alhena", "Alifa", "Alioth", "Alkaid", "Alkalurops", "Alkaphrah", "Alkes", "Alkhiba", "Almach",
    "Almeisan", "Almuredin", "AlNa'ir", "Alnasl", "Alnilam", "Alnitak", "Alniyat", "Alphard", "Alphecca", "Alpheratz",
    "Alrakis", "Alrami", "Alrescha", "AlRijil", "Alsahm", "Alsciaukat", "Alshain", "Alshat", "Alshemali", "Alsuhail",
    "Altair", "Altais", "Alterf", "Althalimain", "AlTinnin", "Aludra", "AlulaAustralis", "AlulaBorealis", "Alwaid", "Alwazn",
    "Alya", "Alzirr", "AmazonStar", "Ancha", "Anchat", "AngelStern", "Angetenar", "Ankaa", "Anser", "Antecanis",
    "Apollo", "Arich", "Arided", "Arietis", "Arkab", "ArkebPrior", "Arneb", "Arrioph", "AsadAustralis", "Ascella",
    "Aschere", "AsellusAustralis", "AsellusBorealis", "AsellusPrimus", "Ashtaroth", "Asmidiske", "Aspidiske", "Asterion", "Asterope", "Asuia",
    "Athafiyy", "Atik", "Atlas", "Atria", "Auva", "Avior", "Azelfafage", "Azha", "Azimech", "BatenKaitos",
    "Becrux", "Beid", "Bellatrix", "Benatnasch", "Biham", "Botein", "Brachium", "Bunda", "Cajam", "Calbalakrab",
    "Calx", "Canicula", "Capella", "Caph", "Castor", "Castula", "Cebalrai", "Ceginus", "Celaeno", "Chara",
    "Chertan", "Choo", "Clava", "CorCaroli", "CorHydrae", "CorLeonis", "Cornu", "CorScorpii", "CorSepentis", "CorTauri",
    "Coxa", "Cursa", "Cymbae", "Cynosaura", "Dabih", "DenebAlgedi", "DenebDulfim", "DenebelOkab", "DenebKaitos", "DenebOkab",
    "Denebola", "Dhalim", "Dhur", "Diadem", "Difda", "DifdaalAuwel", "Dnoces", "Dubhe", "Dziban", "Dzuba",
    "Edasich", "ElAcola", "Elacrab", "Electra", "Elgebar", "Elgomaisa", "ElKaprah", "ElKaridab", "Elkeid", "ElKhereb",
    "Elmathalleth", "Elnath", "ElPhekrah", "Eltanin", "Enif", "Erakis", "Errai", "FalxItalica", "Fidis", "Fomalhaut",
    "Fornacis", "FumAlSamakah", "Furud", "Gacrux", "Gallina", "GarnetStar", "Gemma", "Genam", "Giausar", "GiedePrime",
    "Giedi", "Gienah", "Gienar", "Gildun", "Girtab", "Gnosia", "Gomeisa", "Gorgona", "Graffias", "Hadar",
    "Hamal", "Haris", "Hasseleh", "Hastorang", "Hatysa", "Heka", "Hercules", "Heze", "Hoedus", "Homam",
    "HyadumPrimus", "Icalurus", "Iclarkrav", "Izar", "Jabbah", "Jewel", "Jugum", "Juza", "Kabeleced", "Kaff",
    "Kaffa", "Kaffaljidma", "Kaitain", "KalbalAkrab", "Kat", "KausAustralis", "KausBorealis", "KausMedia", "Keid", "KeKouan",
    "Kelb", "Kerb", "Kerbel", "KiffaBoraelis", "Kitalpha", "Kochab", "Kornephoros", "Kraz", "Ksora", "Kuma",
    "Kurhah", "Kursa", "Lesath", "Maasym", "Maaz", "Mabsuthat", "Maia", "Marfik", "Markab", "Marrha",
    "Matar", "Mebsuta", "Megres", "Meissa", "Mekbuda", "Menkalinan", "Menkar", "Menkent", "Menkib", "Merak",
    "Meres", "Merga", "Meridiana", "Merope", "Mesartim", "Metallah", "Miaplacidus", "Mimosa", "Minelauva", "Minkar",
    "Mintaka", "Mirac", "Mirach", "Miram", "Mirfak", "Mirzam", "Misam", "Mismar", "Mizar", "Muhlifain",
    "Muliphein", "Muphrid", "Muscida", "NairalSaif", "NairalZaurak", "Naos", "Nash", "Nashira", "Navi", "Nekkar",
    "Nicolaus", "Nihal", "Nodus", "Nunki", "Nusakan", "OculusBoreus", "Okda", "Osiris", "OsPegasi", "Palilicium",
    "Peacock", "Phact", "Phecda", "Pherkad", "PherkadMinor", "Pherkard", "Phoenice", "Phurad", "Pishpai", "Pleione",
    "Polaris", "Pollux", "Porrima", "Postvarta", "Praecipua", "Procyon", "Propus", "Protrygetor", "Pulcherrima", "Rana",
    "RanaSecunda", "Rasalas", "Rasalgethi", "Rasalhague", "Rasalmothallah", "RasHammel", "Rastaban", "Reda", "Regor", "Regulus",
    "Rescha", "RigilKentaurus", "RiglalAwwa", "Rotanen", "Ruchba", "Ruchbah", "Rukbat", "Rutilicus", "Saak", "Sabik",
    "Sadachbia", "Sadalbari", "Sadalmelik", "Sadalsuud", "Sadatoni", "Sadira", "Sadr", "Saidak", "Saiph", "Salm",
    "Sargas", "Sarin", "Sartan", "Sceptrum", "Scheat", "Schedar", "Scheddi", "Schemali", "Scutulum", "SeatAlpheras",
    "Segin", "Seginus", "Shaula", "Shedir", "Sheliak", "Sheratan", "Singer", "Sirius", "Sirrah", "Situla",
    "Skat", "Spica", "Sterope", "Subra", "Suha", "Suhail", "SuhailHadar", "SuhailRadar", "Suhel", "Sulafat",
    "Superba", "Svalocin", "Syrma", "Tabit", "Tais", "Talitha", "TaniaAustralis", "TaniaBorealis", "Tarazed", "Tarf",
    "TaTsun", "Taygeta", "Tegmen", "Tejat", "TejatPrior", "Terebellum", "Theemim", "Thuban", "Tolimann", "Tramontana",
    "Tsih", "Tureis", "Unukalhai", "Vega", "Venabulum", "Venator", "Vendemiatrix", "Vespertilio", "Vildiur", "Vindemiatrix",
    "Wasat", "Wazn", "YedPosterior", "YedPrior", "Zaniah", "Zaurak", "Zavijava", "ZenithStar", "Zibel", "Zosma",
    "Zubenelakrab", "ZubenElgenubi", "Zubeneschamali", "ZubenHakrabi", "Zubra"
    };
    int raw_star_namesLength = 425;

    const char* raw_giant_names[60]
    {
    "AH Scorpii", "Aldebaran", "Alpha Herculis", "Antares", "Arcturus", "AV Persei", "BC Cygni", "Betelgeuse", "BI Cygni", "BO Carinae",
    "Canopus", "CE Tauri", "CK Carinae", "CW Leonis", "Deneb", "Epsilon Aurigae", "Eta Carinae", "EV Carinae", "IX Carinae", "KW Sagittarii",
    "KY Cygni", "Mira", "Mu Cephei", "NML Cygni", "NR Vulpeculae", "PZ Cassiopeiae", "R Doradus", "R Leporis", "Rho Cassiopeiae", "Rigel",
    "RS Persei", "RT Carinae", "RU Virginis", "RW Cephei", "S Cassiopeiae", "S Cephei", "S Doradus", "S Persei", "SU Persei", "TV Geminorum",
    "U Lacertae", "UY Scuti", "V1185 Scorpii", "V354 Cephei", "V355 Cepheus", "V382 Carinae", "V396 Centauri", "V437 Scuti", "V509 Cassiopeiae", "V528 Carinae",
    "V602 Carinae", "V648 Cassiopeiae", "V669 Cassiopeiae", "V838 Monocerotis", "V915 Scorpii", "VV Cephei", "VX Sagittarii", "VY Canis Majoris", "WOH G64", "XX Persei"
    };
    int raw_giant_namesLength = 60;

    const char* giant_name_formats[7]{ "HD %04d%02d", "HDE %04d%02d", "HR %04d", "HV %04d", "LBV %04d-%02d", "NSV %04d", "YSC %04d-%02d" };
    int giant_name_formatsLength = 7;

    const char* neutron_star_name_formats[2]{ "NTR J%02d%02d+%02d", "NTR J%02d%02d-%02d" };
    int neutron_star_name_formatsLength = 2;

    const char* black_hole_name_formats[2]{ "DSR J%02d%02d+%02d", "DSR J%02d%02d-%02d" };
    int black_hole_name_formatsLength = 2;

    //std::set<std::string> starnames;

    std::string RandomName(int seed)
    {
        DotNet35Random dotNet35Random(seed);
        int num = (int)(dotNet35Random.NextDouble() * 1.8 + 2.3);
        std::string text = "";
        for (int i = 0; i < num; i++)
        {
            if (!(dotNet35Random.NextDouble() < 0.05000000074505806) || i != 0)
            {
                text = ((!(dotNet35Random.NextDouble() < 0.97000002861022949) && num < 4) ? (text + con1[dotNet35Random.Next(con1Length)]) : (text + con0[dotNet35Random.Next(con0Length)]));
                text = ((i == num - 1 && dotNet35Random.NextDouble() < 0.89999997615814209) ? (text + ending[dotNet35Random.Next(endingLength)]) : ((!(dotNet35Random.NextDouble() < 0.97000002861022949)) ? (text + vow2[dotNet35Random.Next(vow2Length)]) : (text + vow1[dotNet35Random.Next(vow1Length)])));
            }
            else
            {
                text += vow0[dotNet35Random.Next(vow0Length)];
            }
        }

        text = ReplaceString(text, "uu", "u");
        text = ReplaceString(text, "ooo", "oo");
        text = ReplaceString(text, "eee", "ee");
        text = ReplaceString(text, "eea", "ea");
        text = ReplaceString(text, "aa", "a");
        text = ReplaceString(text, "yy", "y");
        text[0] = toupper(text[0]);
        return text;
    }

    std::string RandomStarName(int seed,const StarClass& starData,std::set<std::string>& starnames)
    {
        DotNet35Random dotNet35Random(seed);
        for (int i = 0; i < 256; i++)
        {
            std::string text = _RandomStarName(dotNet35Random.Next(), starData);
            if (starnames.find(text) != starnames.end())
                continue;
            starnames.insert(text);
            return text;
        }
        return "XStar";
    }

    std::string _RandomStarName(int seed,const StarClass& starData)
    {
        DotNet35Random dotNet35Random(seed);
        int seed2 = dotNet35Random.Next();
        double num = dotNet35Random.NextDouble();
        double num2 = dotNet35Random.NextDouble();
        if (starData.type == EStarType::GiantStar)
        {
            if (num2 < 0.40000000596046448)
            {
                return RandomGiantStarNameFromRawNames(seed2);
            }
            if (num2 < 0.699999988079071)
            {
                return RandomGiantStarNameWithConstellationAlpha(seed2);
            }
            return RandomGiantStarNameWithFormat(seed2);
        }
        if (starData.type == EStarType::NeutronStar)
        {
            return RandomNeutronStarNameWithFormat(seed2);
        }
        if (starData.type == EStarType::BlackHole)
        {
            return RandomBlackHoleNameWithFormat(seed2);
        }
        if (num < 0.60000002384185791)
        {
            return RandomStarNameFromRawNames(seed2);
        }
        if (num < 0.93000000715255737)
        {
            return RandomStarNameWithConstellationAlpha(seed2);
        }
        return RandomStarNameWithConstellationNumber(seed2);
    }

    std::string RandomStarNameFromRawNames(int seed)
    {
        int num = DotNet35Random(seed).Next();
        num %= raw_star_namesLength;
        return raw_star_names[num];
    }

    std::string RandomStarNameWithConstellationAlpha(int seed)
    {
        DotNet35Random dotNet35Random(seed);
        int num = dotNet35Random.Next();
        int num2 = dotNet35Random.Next();
        num %= constellationsLength;
        num2 %= alphabetaLength;
        const char* text = constellations[num];
        if (strlen(text) > 10)
        {
            return vformat("%s %s", alphabeta_letter[num2], text);
        }
        return vformat("%s %s", alphabeta[num2], text);
    }

    std::string RandomStarNameWithConstellationNumber(int seed)
    {
        DotNet35Random dotNet35Random(seed);
        int num = dotNet35Random.Next();
        int num2 = dotNet35Random.Next(27, 75);
        num %= constellationsLength;
        return vformat("%d %s", num2, constellations[num]);
    }

    std::string RandomGiantStarNameFromRawNames(int seed)
    {
        int num = DotNet35Random(seed).Next();
        num %= raw_giant_namesLength;
        return raw_giant_names[num];
    }

    std::string RandomGiantStarNameWithConstellationAlpha(int seed)
    {
        DotNet35Random dotNet35Random(seed);
        int num = dotNet35Random.Next();
        int num2 = dotNet35Random.Next(15, 26);
        int num3 = dotNet35Random.Next(0, 26);
        num %= constellationsLength;
        unsigned short num4 = (unsigned short)(65 + num2);
        char c = (char)(65 + num3);
        return vformat("%d %s", (num4 + c), constellations[num]);
    }

    std::string RandomGiantStarNameWithFormat(int seed)
    {
        DotNet35Random dotNet35Random(seed);
        int num = dotNet35Random.Next();
        int num2 = dotNet35Random.Next(10000);
        int num3 = dotNet35Random.Next(100);
        num %= giant_name_formatsLength;
        
        return vformat(giant_name_formats[num], num2, num3);
    }

    std::string RandomNeutronStarNameWithFormat(int seed)
    {
        DotNet35Random dotNet35Random(seed);
        int num = dotNet35Random.Next();
        int num2 = dotNet35Random.Next(24);
        int num3 = dotNet35Random.Next(60);
        int num4 = dotNet35Random.Next(0, 60);
        num %= neutron_star_name_formatsLength;
        return vformat(neutron_star_name_formats[num], num2, num3, num4);
    }

    std::string RandomBlackHoleNameWithFormat(int seed)
    {
        DotNet35Random dotNet35Random(seed);
        int num = dotNet35Random.Next();
        int num2 = dotNet35Random.Next(24);
        int num3 = dotNet35Random.Next(60);
        int num4 = dotNet35Random.Next(0, 60);
        num %= black_hole_name_formatsLength;
        return vformat(black_hole_name_formats[num], num2, num3, num4);
    }
}NameGen;
