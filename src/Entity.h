```cpp
typedef struct tagENTITYLIST
{
	UINT	dUniCode;
	CHAR	acCodeA[15];
	TCHAR	atCodeW[15];
	LPTSTR	aptInfo;

} ENTITYLIST, *LPENTITYLIST;

CONST ENTITYLIST	gstEttySP[] = {
	{ 0x0022,	("&quot;"),		TEXT("&quot;"),		TEXT("인용 부호") },
	{ 0x0026,	("&amp;"),		TEXT("&amp;"),		TEXT("앰퍼샌드") },
	{ 0x003C,	("&lt;"),		TEXT("&lt;"),		TEXT("작음") },
	{ 0x003E,	("&gt;"),		TEXT("&gt;"),		TEXT("큼") },
	{ 0x0000,	(" "),			TEXT(" "),			TEXT("말단 더미") }
};

CONST ENTITYLIST	gstEtyAlpha[] = {
	{ 0x00C1,	("&Aacute;"),	TEXT("&Aacute;"),	TEXT("아큐트 악센트가 있는 A") },
	{ 0x00E1,	("&aacute;"),	TEXT("&aacute;"),	TEXT("아큐트 악센트가 있는 소문자 a") },
	{ 0x00C2,	("&Acirc;"),	TEXT("&Acirc;"),	TEXT("서컴플렉스 악센트가 있는 A") },
	{ 0x00E2,	("&acirc;"),	TEXT("&acirc;"),	TEXT("서컴플렉스 악센트가 있는 소문자 a") },
	{ 0x00B4,	("&acute;"),	TEXT("&acute;"),	TEXT("악상테귀") },
	{ 0x00C6,	("&AElig;"),	TEXT("&AElig;"),	TEXT("AE") },
	{ 0x00E6,	("&aelig;"),	TEXT("&aelig;"),	TEXT("소문자 ae") },
	{ 0x00C0,	("&Agrave;"),	TEXT("&Agrave;"),	TEXT("그레이브 악센트가 있는 A") },
	{ 0x00E0,	("&agrave;"),	TEXT("&agrave;"),	TEXT("그레이브 악센트가 있는 소문자 a") },
	{ 0x2135,	("&alefsym;"),	TEXT("&alefsym;"),	TEXT("알레프") },
	{ 0x0391,	("&Alpha;"),	TEXT("&Alpha;"),	TEXT("대문자 알파") },
	{ 0x03B1,	("&alpha;"),	TEXT("&alpha;"),	TEXT("소문자 알파") },
	{ 0x2227,	("&and;"),		TEXT("&and;"),		TEXT("그리고 (논리곱)") },
	{ 0x2220,	("&ang;"),		TEXT("&ang;"),		TEXT("각도") },
	{ 0x00C5,	("&Aring;"),	TEXT("&Aring;"),	TEXT("위에 링이 있는 A") },
	{ 0x00E5,	("&aring;"),	TEXT("&aring;"),	TEXT("위에 링이 있는 소문자 a") },
	{ 0x2248,	("&asymp;"),	TEXT("&asymp;"),	TEXT("대략적으로 같음") },
	{ 0x00C3,	("&Atilde;"),	TEXT("&Atilde;"),	TEXT("틸드가 있는 A") },
	{ 0x00E3,	("&atilde;"),	TEXT("&atilde;"),	TEXT("틸드가 있는 소문자 a") },
	{ 0x00C4,	("&Auml;"),		TEXT("&Auml;"),		TEXT("다이아레시스가 있는 A") },
	{ 0x00E4,	("&auml;"),		TEXT("&auml;"),		TEXT("다이아레시스가 있는 소문자 a") },
	{ 0x0000,	(" "),			TEXT(" "),			TEXT("말단 더미") }
};

CONST ENTITYLIST	gstEtyBravo[] = {
	{ 0x201E,	("&bdquo;"),	TEXT("&bdquo;"),	TEXT("아래쪽 이중 인용 부호") },
	{ 0x0392,	("&Beta;"),		TEXT("&Beta;"),		TEXT("대문자 베타") },
	{ 0x03B2,	("&beta;"),		TEXT("&beta;"),		TEXT("소문자 베타") },
	{ 0x00A6,	("&brvbar;"),	TEXT("&brvbar;"),	TEXT("끊어진 막대") },
	{ 0x2022,	("&bull;"),		TEXT("&bull;"),		TEXT("검은 점") },
	{ 0x0000,	(" "),			TEXT(" "),			TEXT("말단 더미") }
};

CONST ENTITYLIST	gstEtyCharlie[] = {
	{ 0x2229,	("&cap;"),		TEXT("&cap;"),		TEXT("교집합") },
	{ 0x00C7,	("&Ccedil;"),	TEXT("&Ccedil;"),	TEXT("세디유가 있는 C") },
	{ 0x00E7,	("&ccedil;"),	TEXT("&ccedil;"),	TEXT("세디유가 있는 소문자 c") },
	{ 0x00B8,	("&cedil;"),	TEXT("&cedil;"),	TEXT("세디유") },
	{ 0x00A2,	("&cent;"),		TEXT("&cent;"),		TEXT("센트 기호") },
	{ 0x03A7,	("&Chi;"),		TEXT("&Chi;"),		TEXT("대문자 카이") },
	{ 0x03C7,	("&chi;"),		TEXT("&chi;"),		TEXT("소문자 카이") },
	{ 0x02C6,	("&circ;"),		TEXT("&circ;"),		TEXT("서컴플렉스") },
	{ 0x2663,	("&clubs;"),	TEXT("&clubs;"),	TEXT("클럽") },
	{ 0x2245,	("&cong;"),		TEXT("&cong;"),		TEXT("동형") },
	{ 0x00A9,	("&copy;"),		TEXT("&copy;"),		TEXT("저작권 표시 기호") },
	{ 0x21B5,	("&crarr;"),	TEXT("&crarr;"),	TEXT("캐리지 리턴") },
	{ 0x222A,	("&cup;"),		TEXT("&cup;"),		TEXT("합집합") },
	{ 0x00A4,	("&curren;"),	TEXT("&curren;"),	TEXT("통화 기호") },
	{ 0x0000,	(" "),			TEXT(" "),			TEXT("말단 더미") }
};

CONST ENTITYLIST	gstEtyDelta[] = {
	{ 0x2020,	("&dagger;"),	TEXT("&dagger;"),	TEXT("단검") },
	{ 0x2021,	("&Dagger;"),	TEXT("&Dagger;"),	TEXT("이중 단검") },
	{ 0x2193,	("&darr;"),		TEXT("&darr;"),		TEXT("아래쪽 화살표") },
	{ 0x21D3,	("&dArr;"),		TEXT("&dArr;"),		TEXT("아래쪽 이중 화살표") },
	{ 0x00B0,	("&deg;"),		TEXT("&deg;"),		TEXT("도") },
	{ 0x0394,	("&Delta;"),	TEXT("&Delta;"),	TEXT("대문자 델타") },
	{ 0x03B4,	("&delta;"),	TEXT("&delta;"),	TEXT("소문자 델타") },
	{ 0x2666,	("&diams;"),	TEXT("&diams;"),	TEXT("다이아몬드") },
	{ 0x00F7,	("&divide;"),	TEXT("&divide;"),	TEXT("나눗셈 기호") },
	{ 0x0000,	(" "),			TEXT(" "),			TEXT("말단 더미") }
};

CONST ENTITYLIST	gstEtyEcho[] = {
	{ 0x00C9,	("&Eacute;"),	TEXT("&Eacute;"),	TEXT("아큐트 악센트가 있는 E") },
	{ 0x00E9,	("&eacute;"),	TEXT("&eacute;"),	TEXT("아큐트 악센트가 있는 소문자 e") },
	{ 0x00CA,	("&Ecirc;"),	TEXT("&Ecirc;"),	TEXT("서컴플렉스 악센트가 있는 E") },
	{ 0x00EA,	("&ecirc;"),	TEXT("&ecirc;"),	TEXT("서컴플렉스 악센트가 있는 소문자 e") },
	{ 0x00C8,	("&Egrave;"),	TEXT("&Egrave;"),	TEXT("그레이브 악센트가 있는 E") },
	{ 0x00E8,	("&egrave;"),	TEXT("&egrave;"),	TEXT("그레이브 악센트가 있는 소문자 e") },
	{ 0x2205,	("&empty;"),	TEXT("&empty;"),	TEXT("공집합") },
	{ 0x2003,	("&emsp;"),		TEXT("&emsp;"),		TEXT("m자 폭의 공백 (전각 공백)") },
	{ 0x2002,	("&ensp;"),		TEXT("&ensp;"),		TEXT("n자 폭의 공백 (반각 공백)") },
	{ 0x0395,	("&Epsilon;"),	TEXT("&Epsilon;"),	TEXT("대문자 엡실론") },
	{ 0x03B5,	("&epsilon;"),	TEXT("&epsilon;"),	TEXT("소문자 엡실론") },
	{ 0x2261,	("&equiv;"),	TEXT("&equiv;"),	TEXT("항상 같음, 합동") },
	{ 0x0397,	("&Eta;"),		TEXT("&Eta;"),		TEXT("대문자 에타") },
	{ 0x03B7,	("&eta;"),		TEXT("&eta;"),		TEXT("소문자 에타") },
	{ 0x00D0,	("&ETH;"),		TEXT("&ETH;"),		TEXT("아이슬란드어 ETH") },
	{ 0x00F0,	("&eth;"),		TEXT("&eth;"),		TEXT("아이슬란드어 소문자 eth") },
	{ 0x00CB,	("&Euml;"),		TEXT("&Euml;"),		TEXT("다이아레시스가 있는 E") },
	{ 0x00EB,	("&euml;"),		TEXT("&euml;"),		TEXT("다이아레시스가 있는 소문자 e") },
	{ 0x20AC,	("&euro;"),		TEXT("&euro;"),		TEXT("유로 기호") },
	{ 0x2203,	("&exist;"),	TEXT("&exist;"),	TEXT("존재 (존재 한정자)") },
	{ 0x0000,	(" "),			TEXT(" "),			TEXT("말단 더미") }
};

CONST ENTITYLIST	gstEtyFoxtrot[] = {
	{ 0x0192,	("&fnof;"),		TEXT("&fnof;"),		TEXT("소문자 f (후크 포함), 함수") },
	{ 0x2200,	("&forall;"),	TEXT("&forall;"),	TEXT("모든 (보편 한정자)") },
	{ 0x00BD,	("&frac12;"),	TEXT("&frac12;"),	TEXT("1/2") },
	{ 0x00BC,	("&frac14;"),	TEXT("&frac14;"),	TEXT("1/4") },
	{ 0x00BE,	("&frac34;"),	TEXT("&frac34;"),	TEXT("3/4") },
	{ 0x2044,	("&frasl;"),	TEXT("&frasl;"),	TEXT("분수의 슬래시") },
	{ 0x0000,	(" "),			TEXT(" "),			TEXT("말단 더미") }
};

CONST ENTITYLIST	gstEtyGolf[] = {
	{ 0x0393,	("&Gamma;"),	TEXT("&Gamma;"),	TEXT("대문자 감마") },
	{ 0x03B3,	("&gamma;"),	TEXT("&gamma;"),	TEXT("소문자 감마") },
	{ 0x2265,	("&ge;"),		TEXT("&ge;"),		TEXT("크거나 같음") },
	{ 0x0000,	(" "),			TEXT(" "),			TEXT("말단 더미") }
};

CONST ENTITYLIST	gstEtyHotel[] = {
	{ 0x2194,	("&harr;"),		TEXT("&harr;"),		TEXT("좌우 화살표") },
	{ 0x21D4,	("&hArr;"),		TEXT("&hArr;"),		TEXT("동치") },
	{ 0x2665,	("&hearts;"),	TEXT("&hearts;"),	TEXT("하트") },
	{ 0x2026,	("&hellip;"),	TEXT("&hellip;"),	TEXT("생략 부호") },
	{ 0x0000,	(" "),			TEXT(" "),			TEXT("말단 더미") }
};

CONST ENTITYLIST	gstEtyIndia[] = {
	{ 0x00CD,	("&Iacute;"),	TEXT("&Iacute;"),	TEXT("아큐트 악센트가 있는 I") },
	{ 0x00ED,	("&iacute;"),	TEXT("&iacute;"),	TEXT("아큐트 악센트가 있는 소문자 i") },
	{ 0x00CE,	("&Icirc;"),	TEXT("&Icirc;"),	TEXT("서컴플렉스 악센트가 있는 I") },
	{ 0x00EE,	("&icirc;"),	TEXT("&icirc;"),	TEXT("서컴플렉스 악센트가 있는 소문자 i") },
	{ 0x00A1,	("&iexcl;"),	TEXT("&iexcl;"),	TEXT("역 감탄 부호") },
	{ 0x00CC,	("&Igrave;"),	TEXT("&Igrave;"),	TEXT("그레이브 악센트가 있는 I") },
	{ 0x00EC,	("&igrave;"),	TEXT("&igrave;"),	TEXT("그레이브 악센트가 있는 소문자 i") },
	{ 0x2111,	("&image;"),	TEXT("&image;"),	TEXT("대문자 I (블랙체), 허수부") },
	{ 0x221E,	("&infin;"),	TEXT("&infin;"),	TEXT("무한대") },
	{ 0x222B,	("&int;"),		TEXT("&int;"),		TEXT("적분 기호") },
	{ 0x0399,	("&Iota;"),		TEXT("&Iota;"),		TEXT("대문자 이오타") },
	{ 0x03B9,	("&iota;"),		TEXT("&iota;"),		TEXT("소문자 이오타") },
	{ 0x00BF,	("&iquest;"),	TEXT("&iquest;"),	TEXT("역 물음표") },
	{ 0x2208,	("&isin;"),		TEXT("&isin;"),		TEXT("속함") },
	{ 0x00CF,	("&Iuml;"),		TEXT("&Iuml;"),		TEXT("다이아레시스가 있는 I") },
	{ 0x00EF,	("&iuml;"),		TEXT("&iuml;"),		TEXT("다이아레시스가 있는 소문자 i") },
	{ 0x0000,	(" "),			TEXT(" "),			TEXT("말단 더미") }
};

CONST ENTITYLIST	gstEtyJuliette[] = {
	{ 0x0000,	(" "),			TEXT(" "),			TEXT("말단 더미") }
};

CONST ENTITYLIST	gstEtyKilo[] = {
	{ 0x039A,	("&Kappa;"),	TEXT("&Kappa;"),	TEXT("대문자 카파") },
	{ 0x03BA,	("&kappa;"),	TEXT("&kappa;"),	TEXT("소문자 카파") },
	{ 0x0000,	(" "),			TEXT(" "),			TEXT("말단 더미") }
};

CONST ENTITYLIST	gstEtyLima[] = {
	{ 0x039B,	("&Lambda;"),	TEXT("&Lambda;"),	TEXT("대문자 람다") },
	{ 0x03BB,	("&lambda;"),	TEXT("&lambda;"),	TEXT("소문자 람다") },
	{ 0x2329,	("&lang;"),		TEXT("&lang;"),		TEXT("왼쪽 각 괄호") },
	{ 0x00AB,	("&laquo;"),	TEXT("&laquo;"),	TEXT("왼쪽 이중 꺾쇠 괄호") },
	{ 0x2190,	("&larr;"),		TEXT("&larr;"),		TEXT("왼쪽 화살표") },
	{ 0x21D0,	("&lArr;"),		TEXT("&lArr;"),		TEXT("왼쪽 이중 화살표") },
	{ 0x2308,	("&lceil;"),	TEXT("&lceil;"),	TEXT("왼쪽 상한") },
	{ 0x201C,	("&ldquo;"),	TEXT("&ldquo;"),	TEXT("왼쪽 이중 인용 부호") },
	{ 0x2264,	("&le;"),		TEXT("&le;"),		TEXT("작거나 같음") },
	{ 0x230A,	("&lfloor;"),	TEXT("&lfloor;"),	TEXT("왼쪽 하한") },
	{ 0x2217,	("&lowast;"),	TEXT("&lowast;"),	TEXT("아스터리스크 연산자") },
	{ 0x25CA,	("&loz;"),		TEXT("&loz;"),		TEXT("마름모") },
	{ 0x200E,	("&lrm;"),		TEXT("&lrm;"),		TEXT("텍스트 방향 제어 (왼쪽에서 오른쪽으로)") },
	{ 0x2039,	("&lsaquo;"),	TEXT("&lsaquo;"),	TEXT("왼쪽 꺾쇠 괄호") },
	{ 0x2018,	("&lsquo;"),	TEXT("&lsquo;"),	TEXT("왼쪽 단일 인용 부호") },
	{ 0x0000,	(" "),			TEXT(" "),			TEXT("말단 더미") }
};

CONST ENTITYLIST	gstEtyMike[] = {
	{ 0x00AF,	("&macr;"),		TEXT("&macr;"),		TEXT("마크론") },
	{ 0x2014,	("&mdash;"),	TEXT("&mdash;"),	TEXT("m자 폭의 대시 (전각 대시)") },
	{ 0x00B5,	("&micro;"),	TEXT("&micro;"),	TEXT("마이크로 기호") },
	{ 0x00B7,	("&middot;"),	TEXT("&middot;"),	TEXT("중점 (라틴)") },
	{ 0x2212,	("&minus;"),	TEXT("&minus;"),	TEXT("마이너스 기호, 뺄셈 기호") },
	{ 0x039C,	("&Mu;"),		TEXT("&Mu;"),		TEXT("대문자 뮤") },
	{ 0x03BC,	("&mu;"),		TEXT("&mu;"),		TEXT("소문자 뮤") },
	{ 0x0000,	(" "),			TEXT(" "),			TEXT("말단 더미") }
};

CONST ENTITYLIST	gstEtyNovember[] = {
	{ 0x2207,	("&nabla;"),	TEXT("&nabla;"),	TEXT("나블라") },
	{ 0x00A0,	("&nbsp;"),		TEXT("&nbsp;"),		TEXT("공백 없음") },
	{ 0x2013,	("&ndash;"),	TEXT("&ndash;"),	TEXT("n자 폭의 대시 (반각 대시)") },
	{ 0x2260,	("&ne;"),		TEXT("&ne;"),		TEXT("같지 않음") },
	{ 0x220B,	("&ni;"),		TEXT("&ni;"),		TEXT("원소로 포함") },
	{ 0x00AC,	("&not;"),		TEXT("&not;"),		TEXT("부정 기호") },
	{ 0x2209,	("&notin;"),	TEXT("&notin;"),	TEXT("원소가 아님") },
	{ 0x2284,	("&nsub;"),		TEXT("&nsub;"),		TEXT("진부분집합이 아님") },
	{ 0x00D1,	("&Ntilde;"),	TEXT("&Ntilde;"),	TEXT("틸드가 있는 N") },
	{ 0x00F1,	("&ntilde;"),	TEXT("&ntilde;"),	TEXT("틸드가 있는 소문자 n") },
	{ 0x039D,	("&Nu;"),		TEXT("&Nu;"),		TEXT("대문자 누") },
	{ 0x03BD,	("&nu;"),		TEXT("&nu;"),		TEXT("소문자 누") },
	{ 0x0000,	(" "),			TEXT(" "),			TEXT("말단 더미") }
};

CONST ENTITYLIST	gstEtyOscar[] = {
	{ 0x00D3,	("&Oacute;"),	TEXT("&Oacute;"),	TEXT("아큐트 악센트가 있는 O") },
	{ 0x00F3,	("&oacute;"),	TEXT("&oacute;"),	TEXT("아큐트 악센트가 있는 소문자 o") },
	{ 0x00D4,	("&Ocirc;"),	TEXT("&Ocirc;"),	TEXT("서컴플렉스 악센트가 있는 O") },
	{ 0x00F4,	("&ocirc;"),	TEXT("&ocirc;"),	TEXT("서컴플렉스 악센트가 있는 소문자 o") },
	{ 0x0152,	("&OElig;"),	TEXT("&OElig;"),	TEXT("리게이처 OE 대문자") },
	{ 0x0153,	("&oelig;"),	TEXT("&oelig;"),	TEXT("리게이처 oe 소문자") },
	{ 0x00D2,	("&Ograve;"),	TEXT("&Ograve;"),	TEXT("그레이브 악센트가 있는 O") },
	{ 0x00F2,	("&ograve;"),	TEXT("&ograve;"),	TEXT("그레이브 악센트가 있는 소문자 o") },
	{ 0x203E,	("&oline;"),	TEXT("&oline;"),	TEXT("오버라인, 논리 부정 기호") },
	{ 0x03A9,	("&Omega;"),	TEXT("&Omega;"),	TEXT("대문자 오메가") },
	{ 0x03C9,	("&omega;"),	TEXT("&omega;"),	TEXT("소문자 오메가") },
	{ 0x039F,	("&Omicron;"),	TEXT("&Omicron;"),	TEXT("대문자 오미크론") },
	{ 0x03BF,	("&omicron;"),	TEXT("&omicron;"),	TEXT("소문자 오미크론") },
	{ 0x2295,	("&oplus;"),	TEXT("&oplus;"),	TEXT("직합") },
	{ 0x2228,	("&or;"),		TEXT("&or;"),		TEXT("또는 (논리합)") },
	{ 0x00AA,	("&ordf;"),		TEXT("&ordf;"),		TEXT("여성 서수 표시") },
	{ 0x00BA,	("&ordm;"),		TEXT("&ordm;"),		TEXT("남성 서수 표시") },
	{ 0x00D8,	("&Oslash;"),	TEXT("&Oslash;"),	TEXT("스트로크가 있는 O") },
	{ 0x00F8,	("&oslash;"),	TEXT("&oslash;"),	TEXT("스트로크가 있는 소문자 o") },
	{ 0x00D5,	("&Otilde;"),	TEXT("&Otilde;"),	TEXT("틸드가 있는 O") },
	{ 0x00F5,	("&otilde;"),	TEXT("&otilde;"),	TEXT("틸드가 있는 소문자 o") },
	{ 0x2297,	("&otimes;"),	TEXT("&otimes;"),	TEXT("텐서 곱") },
	{ 0x00D6,	("&Ouml;"),		TEXT("&Ouml;"),		TEXT("다이아레시스가 있는 O") },
	{ 0x00F6,	("&ouml;"),		TEXT("&ouml;"),		TEXT("다이아레시스가 있는 소문자 o") },
	{ 0x0000,	(" "),			TEXT(" "),			TEXT("말단 더미") }
};

CONST ENTITYLIST	gstEtyPapa[] = {
	{ 0x00B6,	("&para;"),		TEXT("&para;"),		TEXT("단락 기호") },
	{ 0x2202,	("&part;"),		TEXT("&part;"),		TEXT("델, 라운드 디") },
	{ 0x2030,	("&permil;"),	TEXT("&permil;"),	TEXT("퍼밀") },
	{ 0x22A5,	("&perp;"),		TEXT("&perp;"),		TEXT("수직") },
	{ 0x03A6,	("&Phi;"),		TEXT("&Phi;"),		TEXT("대문자 파이") },
	{ 0x03C6,	("&phi;"),		TEXT("&phi;"),		TEXT("소문자 파이") },
	{ 0x03A0,	("&Pi;"),		TEXT("&Pi;"),		TEXT("대문자 파이") },
	{ 0x03C0,	("&pi;"),		TEXT("&pi;"),		TEXT("소문자 파이") },
	{ 0x03D6,	("&piv;"),		TEXT("&piv;"),		TEXT("파이 기호") },
	{ 0x00B1,	("&plusmn;"),	TEXT("&plusmn;"),	TEXT("플러스 마이너스 기호") },
	{ 0x00A3,	("&pound;"),	TEXT("&pound;"),	TEXT("파운드 기호") },
	{ 0x2032,	("&prime;"),	TEXT("&prime;"),	TEXT("분") },
	{ 0x2033,	("&Prime;"),	TEXT("&Prime;"),	TEXT("초") },
	{ 0x220F,	("&prod;"),		TEXT("&prod;"),		TEXT("곱셈") },
	{ 0x221D,	("&prop;"),		TEXT("&prop;"),		TEXT("비례") },
	{ 0x03A8,	("&Psi;"),		TEXT("&Psi;"),		TEXT("대문자 프사이") },
	{ 0x03C8,	("&psi;"),		TEXT("&psi;"),		TEXT("소문자 프사이") },
	{ 0x0000,	(" "),			TEXT(" "),			TEXT("말단 더미") }
};

CONST ENTITYLIST	gstEtyQuebec[] = {
	{ 0x0000,	(" "),			TEXT(" "),			TEXT("말단 더미") }
};

CONST ENTITYLIST	gstEtyRomeo[] = {
	{ 0x221A,	("&radic;"),	TEXT("&radic;"),	TEXT("근호") },
	{ 0x232A,	("&rang;"),		TEXT("&rang;"),		TEXT("오른쪽 각 괄호") },
	{ 0x00BB,	("&raquo;"),	TEXT("&raquo;"),	TEXT("오른쪽 이중 꺾쇠 괄호") },
	{ 0x2192,	("&rarr;"),		TEXT("&rarr;"),		TEXT("오른쪽 화살표") },
	{ 0x21D2,	("&rArr;"),		TEXT("&rArr;"),		TEXT("그러면 (합의)") },
	{ 0x2309,	("&rceil;"),	TEXT("&rceil;"),	TEXT("오른쪽 상한") },
	{ 0x201D,	("&rdquo;"),	TEXT("&rdquo;"),	TEXT("오른쪽 이중 인용 부호") },
	{ 0x211C,	("&real;"),		TEXT("&real;"),		TEXT("대문자 R (블랙체), 실수부") },
	{ 0x00AE,	("&reg;"),		TEXT("&reg;"),		TEXT("등록 상표 기호") },
	{ 0x230B,	("&rfloor;"),	TEXT("&rfloor;"),	TEXT("오른쪽 하한") },
	{ 0x03A1,	("&Rho;"),		TEXT("&Rho;"),		TEXT("대문자 로") },
	{ 0x03C1,	("&rho;"),		TEXT("&rho;"),		TEXT("소문자 로") },
	{ 0x200F,	("&rlm;"),		TEXT("&rlm;"),		TEXT("텍스트 방향 제어 (오른쪽에서 왼쪽으로)") },
	{ 0x203A,	("&rsaquo;"),	TEXT("&rsaquo;"),	TEXT("오른쪽 꺾쇠 괄호") },
	{ 0x2019,	("&rsquo;"),	TEXT("&rsquo;"),	TEXT("오른쪽 단일 인용 부호") },
	{ 0x0000,	(" "),			TEXT(" "),			TEXT("말단 더미") }
};

CONST ENTITYLIST	gstEtySierra[] = {
	{ 0x201A,	("&sbquo;"),	TEXT("&sbquo;"),	TEXT("아래쪽 인용 부호") },
	{ 0x0160,	("&Scaron;"),	TEXT("&Scaron;"),	TEXT("캐론이 있는 S") },
	{ 0x0161,	("&scaron;"),	TEXT("&scaron;"),	TEXT("캐론이 있는 소문자 s") },
	{ 0x22C5,	("&sdot;"),		TEXT("&sdot;"),		TEXT("점 연산자") },
	{ 0x00A7,	("&sect;"),		TEXT("&sect;"),		TEXT("절 기호") },
	{ 0x00AD,	("&shy;"),		TEXT("&shy;"),		TEXT("소프트 하이픈") },
	{ 0x03A3,	("&Sigma;"),	TEXT("&Sigma;"),	TEXT("대문자 시그마") },
	{ 0x03C3,	("&sigma;"),	TEXT("&sigma;"),	TEXT("소문자 시그마") },
	{ 0x03C2,	("&sigmaf;"),	TEXT("&sigmaf;"),	TEXT("소문자 파이널 시그마") },
	{ 0x223C,	("&sim;"),		TEXT("&sim;"),		TEXT("틸드 연산자") },
	{ 0x2660,	("&spades;"),	TEXT("&spades;"),	TEXT("스페이드") },
	{ 0x2282,	("&sub;"),		TEXT("&sub;"),		TEXT("진부분집합") },
	{ 0x2286,	("&sube;"),		TEXT("&sube;"),		TEXT("부분집합") },
	{ 0x2211,	("&sum;"),		TEXT("&sum;"),		TEXT("합계") },
	{ 0x2283,	("&sup;"),		TEXT("&sup;"),		TEXT("진부분집합 (역방향)") },
	{ 0x00B9,	("&sup1;"),		TEXT("&sup1;"),		TEXT("위 첨자 1") },
	{ 0x00B2,	("&sup2;"),		TEXT("&sup2;"),		TEXT("위 첨자 2") },
	{ 0x00B3,	("&sup3;"),		TEXT("&sup3;"),		TEXT("위 첨자 3") },
	{ 0x2287,	("&supe;"),		TEXT("&supe;"),		TEXT("부분집합 (역방향)") },
	{ 0x00DF,	("&szlig;"),	TEXT("&szlig;"),	TEXT("독일어 에스체트") },
	{ 0x0000,	(" "),			TEXT(" "),			TEXT("말단 더미") }
};

CONST ENTITYLIST	gstEtyTango[] = {
	{ 0x03A4,	("&Tau;"),		TEXT("&Tau;"),		TEXT("대문자 타우") },
	{ 0x03C4,	("&tau;"),		TEXT("&tau;"),		TEXT("소문자 타우") },
	{ 0x2234,	("&there4;"),	TEXT("&there4;"),	TEXT("그러므로") },
	{ 0x0398,	("&Theta;"),	TEXT("&Theta;"),	TEXT("대문자 세타") },
	{ 0x03B8,	("&theta;"),	TEXT("&theta;"),	TEXT("소문자 세타") },
	{ 0x03D1,	("&thetasym;"),	TEXT("&thetasym;"),	TEXT("소문자 세타 기호") },
	{ 0x2009,	("&thinsp;"),	TEXT("&thinsp;"),	TEXT("좁은 공백") },
	{ 0x00DE,	("&THORN;"),	TEXT("&THORN;"),	TEXT("아이슬란드어 THORN") },
	{ 0x00FE,	("&thorn;"),	TEXT("&thorn;"),	TEXT("아이슬란드어 소문자 thorn") },
	{ 0x02DC,	("&tilde;"),	TEXT("&tilde;"),	TEXT("작은 틸드") },
	{ 0x00D7,	("&times;"),	TEXT("&times;"),	TEXT("곱셈 기호") },
	{ 0x2122,	("&trade;"),	TEXT("&trade;"),	TEXT("상표 기호") },
	{ 0x0000,	(" "),			TEXT(" "),			TEXT("말단 더미") }
};

CONST ENTITYLIST	gstEtyUniform[] = {
	{ 0x00DA,	("&Uacute;"),	TEXT("&Uacute;"),	TEXT("아큐트 악센트가 있는 U") },
	{ 0x00FA,	("&uacute;"),	TEXT("&uacute;"),	TEXT("아큐트 악센트가 있는 소문자 u") },
	{ 0x2191,	("&uarr;"),		TEXT("&uarr;"),		TEXT("위쪽 화살표") },
	{ 0x21D1,	("&uArr;"),		TEXT("&uArr;"),		TEXT("위쪽 이중 화살표") },
	{ 0x00DB,	("&Ucirc;"),	TEXT("&Ucirc;"),	TEXT("서컴플렉스 악센트가 있는 U") },
	{ 0x00FB,	("&ucirc;"),	TEXT("&ucirc;"),	TEXT("서컴플렉스 악센트가 있는 소문자 u") },
	{ 0x00D9,	("&Ugrave;"),	TEXT("&Ugrave;"),	TEXT("그레이브 악센트가 있는 U") },
	{ 0x00F9,	("&ugrave;"),	TEXT("&ugrave;"),	TEXT("그레이브 악센트가 있는 소문자 u") },
	{ 0x00A8,	("&uml;"),		TEXT("&uml;"),		TEXT("움라우트, 다이아레시스") },
	{ 0x03D2,	("&upsih;"),	TEXT("&upsih;"),	TEXT("훅 기호가 있는 업실론") },
	{ 0x03A5,	("&Upsilon;"),	TEXT("&Upsilon;"),	TEXT("대문자 업실론") },
	{ 0x03C5,	("&upsilon;"),	TEXT("&upsilon;"),	TEXT("소문자 업실론") },
	{ 0x00DC,	("&Uuml;"),		TEXT("&Uuml;"),		TEXT("다이아레시스가 있는 U") },
	{ 0x00FC,	("&uuml;"),		TEXT("&uuml;"),		TEXT("다이아레시스가 있는 소문자 u") },
	{ 0x0000,	(" "),			TEXT(" "),			TEXT("말단 더미") }
};

CONST ENTITYLIST	gstEtyVictor[] = {
	{ 0x0000,	(" "),			TEXT(" "),			TEXT("말단 더미") }
};

CONST ENTITYLIST	gstEtyWhiskey[] = {
	{ 0x2118,	("&weierp;"),	TEXT("&weierp;"),	TEXT("대문자 P (스크립트체), Weierstrass의 P 함수") },
	{ 0x0000,	(" "),			TEXT(" "),			TEXT("말단 더미") }
};

CONST ENTITYLIST	gstEtyXray[] = {
	{ 0x039E,	("&Xi;"),		TEXT("&Xi;"),		TEXT("대문자 크사이") },
	{ 0x03BE,	("&xi;"),		TEXT("&xi;"),		TEXT("소문자 크사이") },
	{ 0x0000,	(" "),			TEXT(" "),			TEXT("말단 더미") }
};

CONST ENTITYLIST	gstEtyYankee[] = {
	{ 0x00DD,	("&Yacute;"),	TEXT("&Yacute;"),	TEXT("아큐트 악센트가 있는 Y") },
	{ 0x00FD,	("&yacute;"),	TEXT("&yacute;"),	TEXT("아큐트 악센트가 있는 소문자 y") },
	{ 0x00A5,	("&yen;"),		TEXT("&yen;"),		TEXT("엔 기호") },
	{ 0x00FF,	("&yuml;"),		TEXT("&yuml;"),		TEXT("다이아레시스가 있는 소문자 y") },
	{ 0x0178,	("&Yuml;"),		TEXT("&Yuml;"),		TEXT("다이아레시스가 있는 Y") },
	{ 0x0000,	(" "),			TEXT(" "),			TEXT("말단 더미") }
};

CONST ENTITYLIST	gstEtyZulu[] = {
	{ 0x0396,	("&Zeta;"),		TEXT("&Zeta;"),		TEXT("대문자 제타") },
	{ 0x03B6,	("&zeta;"),		TEXT("&zeta;"),		TEXT("소문자 제타") },
	{ 0x200D,	("&zwj;"),		TEXT("&zwj;"),		TEXT("폭 없는 조인자") },
	{ 0x200C,	("&zwnj;"),		TEXT("&zwnj;"),		TEXT("폭 없는 비조인자") },
	{ 0x0000,	(" "),			TEXT(" "),			TEXT("말단 더미") }
};

CONST ENTITYLIST	*gpstEntitys[] = { gstEtyAlpha, gstEtyBravo,
	gstEtyCharlie, gstEtyDelta, gstEtyEcho, gstEtyFoxtrot, gstEtyGolf,
	gstEtyHotel, gstEtyIndia, gstEtyJuliette, gstEtyKilo, gstEtyLima,
	gstEtyMike, gstEtyNovember, gstEtyOscar, gstEtyPapa, gstEtyQuebec,
	gstEtyRomeo, gstEtySierra, gstEtyTango, gstEtyUniform, gstEtyVictor,
	gstEtyWhiskey, gstEtyXray, gstEtyYankee, gstEtyZulu };
