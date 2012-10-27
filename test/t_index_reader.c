#include <unistd.h>

#include "test_all.h"
#include "lcn_index.h"
#include "lcn_util.h"
#include "lcn_analysis.h"
#include "sort_field.h"
#include "index_reader.h"

char *test_index_name;

static char *term_list[1158][2] = {
 { "id", "KW0" }, { "id", "KW1" }, { "id", "KW10" }, { "id", "KW100" },
 { "id", "KW101" }, { "id", "KW102" }, { "id", "KW103" }, { "id", "KW104" },
 { "id", "KW105" }, { "id", "KW106" }, { "id", "KW107" }, { "id", "KW108" },
 { "id", "KW109" }, { "id", "KW11" }, { "id", "KW110" }, { "id", "KW111" },
 { "id", "KW112" }, { "id", "KW113" }, { "id", "KW114" }, { "id", "KW115" },
 { "id", "KW116" }, { "id", "KW117" }, { "id", "KW118" }, { "id", "KW119" },
 { "id", "KW12" }, { "id", "KW120" }, { "id", "KW121" }, { "id", "KW122" },
 { "id", "KW123" }, { "id", "KW124" }, { "id", "KW125" }, { "id", "KW126" },
 { "id", "KW127" }, { "id", "KW128" }, { "id", "KW129" }, { "id", "KW13" },
 { "id", "KW130" }, { "id", "KW131" }, { "id", "KW132" }, { "id", "KW133" },
 { "id", "KW134" }, { "id", "KW135" }, { "id", "KW136" }, { "id", "KW137" },
 { "id", "KW138" }, { "id", "KW139" }, { "id", "KW14" }, { "id", "KW140" },
 { "id", "KW141" }, { "id", "KW142" }, { "id", "KW143" }, { "id", "KW144" },
 { "id", "KW145" }, { "id", "KW146" }, { "id", "KW147" }, { "id", "KW148" },
 { "id", "KW149" }, { "id", "KW15" }, { "id", "KW150" }, { "id", "KW151" },
 { "id", "KW152" }, { "id", "KW153" }, { "id", "KW154" }, { "id", "KW155" },
 { "id", "KW156" }, { "id", "KW157" }, { "id", "KW158" }, { "id", "KW159" },
 { "id", "KW16" }, { "id", "KW160" }, { "id", "KW161" }, { "id", "KW162" },
 { "id", "KW163" }, { "id", "KW164" }, { "id", "KW165" }, { "id", "KW166" },
 { "id", "KW167" }, { "id", "KW168" }, { "id", "KW169" }, { "id", "KW17" },
 { "id", "KW170" }, { "id", "KW171" }, { "id", "KW172" }, { "id", "KW173" },
 { "id", "KW174" }, { "id", "KW175" }, { "id", "KW176" }, { "id", "KW177" },
 { "id", "KW178" }, { "id", "KW179" }, { "id", "KW18" }, { "id", "KW180" },
 { "id", "KW181" }, { "id", "KW182" }, { "id", "KW183" }, { "id", "KW184" },
 { "id", "KW185" }, { "id", "KW186" }, { "id", "KW187" }, { "id", "KW188" },
 { "id", "KW189" }, { "id", "KW19" }, { "id", "KW190" }, { "id", "KW191" },
 { "id", "KW192" }, { "id", "KW193" }, { "id", "KW194" }, { "id", "KW195" },
 { "id", "KW196" }, { "id", "KW197" }, { "id", "KW198" }, { "id", "KW199" },
 { "id", "KW2" }, { "id", "KW20" }, { "id", "KW200" }, { "id", "KW201" },
 { "id", "KW202" }, { "id", "KW203" }, { "id", "KW204" }, { "id", "KW205" },
 { "id", "KW206" }, { "id", "KW207" }, { "id", "KW208" }, { "id", "KW209" },
 { "id", "KW21" }, { "id", "KW210" }, { "id", "KW211" }, { "id", "KW212" },
 { "id", "KW213" }, { "id", "KW214" }, { "id", "KW215" }, { "id", "KW216" },
 { "id", "KW217" }, { "id", "KW218" }, { "id", "KW219" }, { "id", "KW22" },
 { "id", "KW220" }, { "id", "KW221" }, { "id", "KW222" }, { "id", "KW223" },
 { "id", "KW224" }, { "id", "KW225" }, { "id", "KW226" }, { "id", "KW227" },
 { "id", "KW228" }, { "id", "KW229" }, { "id", "KW23" }, { "id", "KW230" },
 { "id", "KW231" }, { "id", "KW232" }, { "id", "KW233" }, { "id", "KW234" },
 { "id", "KW235" }, { "id", "KW236" }, { "id", "KW237" }, { "id", "KW238" },
 { "id", "KW239" }, { "id", "KW24" }, { "id", "KW240" }, { "id", "KW241" },
 { "id", "KW242" }, { "id", "KW243" }, { "id", "KW244" }, { "id", "KW245" },
 { "id", "KW246" }, { "id", "KW247" }, { "id", "KW248" }, { "id", "KW249" },
 { "id", "KW25" }, { "id", "KW250" }, { "id", "KW251" }, { "id", "KW252" },
 { "id", "KW253" }, { "id", "KW254" }, { "id", "KW255" }, { "id", "KW256" },
 { "id", "KW257" }, { "id", "KW258" }, { "id", "KW259" }, { "id", "KW26" },
 { "id", "KW260" }, { "id", "KW261" }, { "id", "KW262" }, { "id", "KW263" },
 { "id", "KW264" }, { "id", "KW265" }, { "id", "KW266" }, { "id", "KW267" },
 { "id", "KW268" }, { "id", "KW269" }, { "id", "KW27" }, { "id", "KW270" },
 { "id", "KW271" }, { "id", "KW272" }, { "id", "KW273" }, { "id", "KW274" },
 { "id", "KW275" }, { "id", "KW276" }, { "id", "KW277" }, { "id", "KW278" },
 { "id", "KW279" }, { "id", "KW28" }, { "id", "KW280" }, { "id", "KW281" },
 { "id", "KW282" }, { "id", "KW283" }, { "id", "KW284" }, { "id", "KW285" },
 { "id", "KW286" }, { "id", "KW287" }, { "id", "KW288" }, { "id", "KW289" },
 { "id", "KW29" }, { "id", "KW290" }, { "id", "KW291" }, { "id", "KW292" },
 { "id", "KW293" }, { "id", "KW294" }, { "id", "KW295" }, { "id", "KW296" },
 { "id", "KW297" }, { "id", "KW298" }, { "id", "KW299" }, { "id", "KW3" },
 { "id", "KW30" }, { "id", "KW300" }, { "id", "KW301" }, { "id", "KW302" },
 { "id", "KW303" }, { "id", "KW304" }, { "id", "KW305" }, { "id", "KW306" },
 { "id", "KW307" }, { "id", "KW308" }, { "id", "KW309" }, { "id", "KW31" },
 { "id", "KW310" }, { "id", "KW311" }, { "id", "KW312" }, { "id", "KW313" },
 { "id", "KW314" }, { "id", "KW315" }, { "id", "KW316" }, { "id", "KW317" },
 { "id", "KW318" }, { "id", "KW319" }, { "id", "KW32" }, { "id", "KW320" },
 { "id", "KW321" }, { "id", "KW322" }, { "id", "KW323" }, { "id", "KW324" },
 { "id", "KW325" }, { "id", "KW326" }, { "id", "KW327" }, { "id", "KW328" },
 { "id", "KW329" }, { "id", "KW33" }, { "id", "KW330" }, { "id", "KW34" },
 { "id", "KW35" }, { "id", "KW36" }, { "id", "KW37" }, { "id", "KW38" },
 { "id", "KW39" }, { "id", "KW4" }, { "id", "KW40" }, { "id", "KW41" },
 { "id", "KW42" }, { "id", "KW43" }, { "id", "KW44" }, { "id", "KW45" },
 { "id", "KW46" }, { "id", "KW47" }, { "id", "KW48" }, { "id", "KW49" },
 { "id", "KW5" }, { "id", "KW50" }, { "id", "KW51" }, { "id", "KW52" },
 { "id", "KW53" }, { "id", "KW54" }, { "id", "KW55" }, { "id", "KW56" },
 { "id", "KW57" }, { "id", "KW58" }, { "id", "KW59" }, { "id", "KW6" },
 { "id", "KW60" }, { "id", "KW61" }, { "id", "KW62" }, { "id", "KW63" },
 { "id", "KW64" }, { "id", "KW65" }, { "id", "KW66" }, { "id", "KW67" },
 { "id", "KW68" }, { "id", "KW69" }, { "id", "KW7" }, { "id", "KW70" },
 { "id", "KW71" }, { "id", "KW72" }, { "id", "KW73" }, { "id", "KW74" },
 { "id", "KW75" }, { "id", "KW76" }, { "id", "KW77" }, { "id", "KW78" },
 { "id", "KW79" }, { "id", "KW8" }, { "id", "KW80" }, { "id", "KW81" },
 { "id", "KW82" }, { "id", "KW83" }, { "id", "KW84" }, { "id", "KW85" },
 { "id", "KW86" }, { "id", "KW87" }, { "id", "KW88" }, { "id", "KW89" },
 { "id", "KW9" }, { "id", "KW90" }, { "id", "KW91" }, { "id", "KW92" },
 { "id", "KW93" }, { "id", "KW94" }, { "id", "KW95" }, { "id", "KW96" },
 { "id", "KW97" }, { "id", "KW98" }, { "id", "KW99" }, { "text", "a" },
 { "text", "ability" }, { "text", "able" }, { "text", "above" }, { "text", "abstract" },
 { "text", "accepts" }, { "text", "access" }, { "text", "accessed" }, { "text", "accessors" },
 { "text", "accordingly" }, { "text", "acoliver" }, { "text", "acronyms" }, { "text", "across" },
 { "text", "add" }, { "text", "adddocument" }, { "text", "added" }, { "text", "addindexes" },
 { "text", "adding" }, { "text", "addition" }, { "text", "addresses" }, { "text", "adjacent" },
 { "text", "adjusted" }, { "text", "advanced" }, { "text", "advantage" }, { "text", "after" },
 { "text", "against" }, { "text", "algorithm" }, { "text", "all" }, { "text", "allow" },
 { "text", "allowed" }, { "text", "alphabetic" }, { "text", "already" }, { "text", "also" },
 { "text", "alter" }, { "text", "altering" }, { "text", "alternate" }, { "text", "amok" },
 { "text", "among" }, { "text", "an" }, { "text", "analyzer" }, { "text", "analyzers" },
 { "text", "and" }, { "text", "anders" }, { "text", "anson" }, { "text", "ant" },
 { "text", "anyone" }, { "text", "anytime" }, { "text", "apache" }, { "text", "api" },
 { "text", "apis" }, { "text", "application" }, { "text", "applications" }, { "text", "applied" },
 { "text", "appreciated" }, { "text", "arbitrary" }, { "text", "are" }, { "text", "arguments" },
 { "text", "as" }, { "text", "asterisk" }, { "text", "at" }, { "text", "automatically" },
 { "text", "avoid" }, { "text", "b" }, { "text", "bar" }, { "text", "based" },
 { "text", "baz" }, { "text", "be" }, { "text", "been" }, { "text", "before" },
 { "text", "behaviour" }, { "text", "bernhard" }, { "text", "better" }, { "text", "between" },
 { "text", "beyond" }, { "text", "bomhoff" }, { "text", "boolean" }, { "text", "booleanquery" },
 { "text", "boost" }, { "text", "boosting" }, { "text", "boris" }, { "text", "both" },
 { "text", "boundary" }, { "text", "briangoetz" }, { "text", "brings" }, { "text", "broken" },
 { "text", "bschneeman" }, { "text", "buffer" }, { "text", "bug" }, { "text", "bugs" },
 { "text", "bugzilla" }, { "text", "build" }, { "text", "builds" }, { "text", "but" },
 { "text", "by" }, { "text", "c" }, { "text", "cached" }, { "text", "cachingwrapperfilter" },
 { "text", "can" }, { "text", "cannot" }, { "text", "carlson" }, { "text", "case" },
 { "text", "cases" }, { "text", "casper" }, { "text", "catch" }, { "text", "caution" },
 { "text", "cd" }, { "text", "cgi" }, { "text", "change" }, { "text", "changed" },
 { "text", "changes" }, { "text", "char" }, { "text", "character" }, { "text", "characters" },
 { "text", "chinese" }, { "text", "choose" }, { "text", "christoph" }, { "text", "cjk" },
 { "text", "class" }, { "text", "classes" }, { "text", "clauses" }, { "text", "cleaned" },
 { "text", "cleanups" }, { "text", "clear" }, { "text", "clone" }, { "text", "closer" },
 { "text", "co" }, { "text", "code" }, { "text", "com" }, { "text", "combined" },
 { "text", "comments" }, { "text", "compatibility" }, { "text", "compile" }, { "text", "compiles" },
 { "text", "compiling" }, { "text", "complex" }, { "text", "compound" }, { "text", "computing" },
 { "text", "concurrently" }, { "text", "condition" }, { "text", "confusing" }, { "text", "consisting" },
 { "text", "constants" }, { "text", "constrains" }, { "text", "constructors" }, { "text", "containing" },
 { "text", "contents" }, { "text", "contiguous" }, { "text", "contributed" }, { "text", "contributions" },
 { "text", "controlling" }, { "text", "convenience" }, { "text", "conversion" }, { "text", "copy" },
 { "text", "copying" }, { "text", "copyright" }, { "text", "correct" }, { "text", "correctly" },
 { "text", "could" }, { "text", "create" }, { "text", "created" }, { "text", "creation" },
 { "text", "custom" }, { "text", "cutting" }, { "text", "d" }, { "text", "dale" },
 { "text", "daniel" }, { "text", "date" }, { "text", "datefilter" }, { "text", "david" },
 { "text", "day" }, { "text", "dealing" }, { "text", "default" }, { "text", "defined" },
 { "text", "delete" }, { "text", "deleted" }, { "text", "deletion" }, { "text", "demo" },
 { "text", "demos" }, { "text", "derived" }, { "text", "describe" }, { "text", "describes" },
 { "text", "details" }, { "text", "developing" }, { "text", "did" }, { "text", "diectory" },
 { "text", "different" }, { "text", "directory" }, { "text", "disable" }, { "text", "disabled" },
 { "text", "disablelucenelocks" }, { "text", "displayed" }, { "text", "distinguish" }, { "text", "distribution" },
 { "text", "dmitry" }, { "text", "do" }, { "text", "doc" }, { "text", "doccount" },
 { "text", "docs" }, { "text", "document" }, { "text", "documentation" }, { "text", "documents" },
 { "text", "does" }, { "text", "don" }, { "text", "doug" }, { "text", "download" },
 { "text", "during" }, { "text", "e" }, { "text", "each" }, { "text", "easier" },
 { "text", "easiest" }, { "text", "easily" }, { "text", "easy" }, { "text", "edit" },
 { "text", "either" }, { "text", "eliminate" }, { "text", "email" }, { "text", "empty" },
 { "text", "encoding" }, { "text", "encountered" }, { "text", "end" }, { "text", "enforcing" },
 { "text", "enhancements" }, { "text", "ensures" }, { "text", "entire" }, { "text", "entirely" },
 { "text", "enumeration" }, { "text", "equal" }, { "text", "equivalent" }, { "text", "erases" },
 { "text", "erik" }, { "text", "error" }, { "text", "escape" }, { "text", "etc" },
 { "text", "eugene" }, { "text", "even" }, { "text", "every" }, { "text", "exact" },
 { "text", "example" }, { "text", "exception" }, { "text", "executing" }, { "text", "existant" },
 { "text", "existence" }, { "text", "exp" }, { "text", "expanding" }, { "text", "expect" },
 { "text", "expensive" }, { "text", "explain" }, { "text", "explanation" }, { "text", "expression" },
 { "text", "extend" }, { "text", "extensible" }, { "text", "extensive" }, { "text", "extensively" },
 { "text", "f" }, { "text", "fact" }, { "text", "failed" }, { "text", "fails" },
 { "text", "fairly" }, { "text", "fast" }, { "text", "faster" }, { "text", "feature" },
 { "text", "few" }, { "text", "field" }, { "text", "fields" }, { "text", "file" },
 { "text", "files" }, { "text", "filter" }, { "text", "filtering" }, { "text", "final" },
 { "text", "finally" }, { "text", "find" }, { "text", "first" }, { "text", "fix" },
 { "text", "fixed" }, { "text", "fixes" }, { "text", "fixing" }, { "text", "float" },
 { "text", "folks" }, { "text", "foo" }, { "text", "food" }, { "text", "for" },
 { "text", "format" }, { "text", "formatted" }, { "text", "found" }, { "text", "frequences" },
 { "text", "from" }, { "text", "fsdirectory" }, { "text", "fully" }, { "text", "fuzzy" },
 { "text", "fuzzytermenum" }, { "text", "g" }, { "text", "gaps" }, { "text", "general" },
 { "text", "generates" }, { "text", "german" }, { "text", "germananalyzer" }, { "text", "germanstemfilter" },
 { "text", "germanstemmer" }, { "text", "getanalyzer" }, { "text", "getdirectory" }, { "text", "getfieldnames" },
 { "text", "getfields" }, { "text", "getproperty" }, { "text", "getting" }, { "text", "getvalues" },
 { "text", "global" }, { "text", "gluzberg" }, { "text", "goller" }, { "text", "good" },
 { "text", "grammar" }, { "text", "gschwarz" }, { "text", "had" }, { "text", "halácsy" },
 { "text", "handling" }, { "text", "hang" }, { "text", "harwood" }, { "text", "has" },
 { "text", "hatcher" }, { "text", "have" }, { "text", "having" }, { "text", "hayes" },
 { "text", "hettesheimer" }, { "text", "hfs" }, { "text", "highlighting" }, { "text", "hit" },
 { "text", "hits" }, { "text", "how" }, { "text", "html" }, { "text", "htmlparser" },
 { "text", "http" }, { "text", "i" }, { "text", "id" }, { "text", "identified" },
 { "text", "identify" }, { "text", "ideogram" }, { "text", "ideograms" }, { "text", "if" },
 { "text", "ignore" }, { "text", "implement" }, { "text", "implementation" }, { "text", "implementations" },
 { "text", "implemented" }, { "text", "improved" }, { "text", "in" }, { "text", "include" },
 { "text", "included" }, { "text", "includes" }, { "text", "including" }, { "text", "inclusive" },
 { "text", "incorrect" }, { "text", "increment" }, { "text", "index" }, { "text", "indexed" },
 { "text", "indexes" }, { "text", "indexexists" }, { "text", "indexing" }, { "text", "indexreader" },
 { "text", "indexsearcher" }, { "text", "indexwriter" }, { "text", "insert" }, { "text", "instance" },
 { "text", "instead" }, { "text", "instruction" }, { "text", "int" }, { "text", "intended" },
 { "text", "internal" }, { "text", "into" }, { "text", "introduced" }, { "text", "introduction" },
 { "text", "io" }, { "text", "is" }, { "text", "islocked" }, { "text", "issue" },
 { "text", "istream" }, { "text", "it" }, { "text", "its" }, { "text", "itself" },
 { "text", "japanese" }, { "text", "java" }, { "text", "javacc" }, { "text", "javadoc" },
 { "text", "javascript" }, { "text", "jdk" }, { "text", "jj" }, { "text", "jon" },
 { "text", "jsp" }, { "text", "julien" }, { "text", "july" }, { "text", "junit" },
 { "text", "jvm" }, { "text", "jvms" }, { "text", "kelvin" }, { "text", "kirchgessner" },
 { "text", "korean" }, { "text", "large" }, { "text", "last" }, { "text", "lastmodified" },
 { "text", "least" }, { "text", "lee" }, { "text", "length" }, { "text", "lengthnorm" },
 { "text", "lettertokenizer" }, { "text", "level" }, { "text", "lgpl" }, { "text", "libraries" },
 { "text", "license" }, { "text", "like" }, { "text", "limit" }, { "text", "links" },
 { "text", "locale" }, { "text", "lock" }, { "text", "locking" }, { "text", "log" },
 { "text", "long" }, { "text", "longer" }, { "text", "low" }, { "text", "lower" },
 { "text", "lowercasetokenizer" }, { "text", "lucene" }, { "text", "macos" }, { "text", "made" },
 { "text", "make" }, { "text", "makefiles" }, { "text", "makes" }, { "text", "mallabone" },
 { "text", "mallobone" }, { "text", "manual" }, { "text", "many" }, { "text", "mark" },
 { "text", "match" }, { "text", "matched" }, { "text", "matches" }, { "text", "matt" },
 { "text", "matthijs" }, { "text", "maxfieldlength" }, { "text", "may" }, { "text", "media" },
 { "text", "memory" }, { "text", "merging" }, { "text", "messer" }, { "text", "meta" },
 { "text", "metamata" }, { "text", "method" }, { "text", "methods" }, { "text", "minmergedocs" },
 { "text", "minor" }, { "text", "misc" }, { "text", "modified" }, { "text", "modifying" },
 { "text", "more" }, { "text", "most" }, { "text", "much" }, { "text", "mularien" },
 { "text", "multifieldparse" }, { "text", "multifieldqueryparser" }, { "text", "multiindexsearcher" }, { "text", "multiple" },
 { "text", "multipletermpositions" }, { "text", "multisearcher" }, { "text", "must" }, { "text", "naber" },
 { "text", "nagoya" }, { "text", "name" }, { "text", "nearest" }, { "text", "need" },
 { "text", "needed" }, { "text", "never" }, { "text", "new" }, { "text", "nielsen" },
 { "text", "nioche" }, { "text", "no" }, { "text", "non" }, { "text", "normalization" },
 { "text", "not" }, { "text", "note" }, { "text", "nouns" }, { "text", "now" },
 { "text", "null" }, { "text", "nullpointerexception" }, { "text", "number" }, { "text", "numbers" },
 { "text", "object" }, { "text", "objects" }, { "text", "obtain" }, { "text", "october" },
 { "text", "of" }, { "text", "offset" }, { "text", "ok" }, { "text", "okner" },
 { "text", "old" }, { "text", "on" }, { "text", "once" }, { "text", "one" },
 { "text", "only" }, { "text", "open" }, { "text", "operator" }, { "text", "optimizations" },
 { "text", "optimized" }, { "text", "options" }, { "text", "or" }, { "text", "order" },
 { "text", "org" }, { "text", "organization" }, { "text", "organized" }, { "text", "other" },
 { "text", "others" }, { "text", "otis" }, { "text", "outofmemoryexceptions" }, { "text", "output" },
 { "text", "outside" }, { "text", "over" }, { "text", "override" }, { "text", "package" },
 { "text", "packages" }, { "text", "pandey" }, { "text", "parseexception" }, { "text", "parser" },
 { "text", "parsing" }, { "text", "particular" }, { "text", "patch" }, { "text", "people" },
 { "text", "per" }, { "text", "perfieldanalyzerwrapper" }, { "text", "performance" }, { "text", "performed" },
 { "text", "permits" }, { "text", "permitted" }, { "text", "permitting" }, { "text", "peter" },
 { "text", "phrase" }, { "text", "phraseprefixquery" }, { "text", "phrasequery" }, { "text", "phrases" },
 { "text", "place" }, { "text", "placing" }, { "text", "plain" }, { "text", "position" },
 { "text", "possible" }, { "text", "potential" }, { "text", "powered" }, { "text", "preference" },
 { "text", "prefix" }, { "text", "prefixquery" }, { "text", "present" }, { "text", "previous" },
 { "text", "previously" }, { "text", "primitive" }, { "text", "priorityqueue" }, { "text", "private" },
 { "text", "problem" }, { "text", "problems" }, { "text", "process" }, { "text", "produce" },
 { "text", "produces" }, { "text", "programmatically" }, { "text", "programming" }, { "text", "properly" },
 { "text", "properties" }, { "text", "property" }, { "text", "protected" }, { "text", "provided" },
 { "text", "provides" }, { "text", "providing" }, { "text", "public" }, { "text", "purpose" },
 { "text", "purposes" }, { "text", "péter" }, { "text", "q" }, { "text", "queries" },
 { "text", "query" }, { "text", "queryfilter" }, { "text", "queryparser" }, { "text", "race" },
 { "text", "raised" }, { "text", "ralf" }, { "text", "ramdirectory" }, { "text", "ramlcn" },
 { "text", "range" }, { "text", "rangequery" }, { "text", "rasik" }, { "text", "rc" },
 { "text", "re" }, { "text", "read" }, { "text", "reader" }, { "text", "reasons" },
 { "text", "reconstructed" }, { "text", "refactored" }, { "text", "regular" }, { "text", "release" },
 { "text", "rely" }, { "text", "remain" }, { "text", "remote" }, { "text", "remotesearchable" },
 { "text", "remotesearchabletest" }, { "text", "removed" }, { "text", "renamed" }, { "text", "renameto" },
 { "text", "renaming" }, { "text", "repeating" }, { "text", "replace" }, { "text", "reported" },
 { "text", "reports" }, { "text", "requests" }, { "text", "required" }, { "text", "requirements" },
 { "text", "resolution" }, { "text", "resolves" }, { "text", "results" }, { "text", "retrieve" },
 { "text", "return" }, { "text", "returned" }, { "text", "returns" }, { "text", "reuse" },
 { "text", "revised" }, { "text", "rewrite" }, { "text", "rewriting" }, { "text", "reyes" },
 { "text", "rmi" }, { "text", "rodrigo" }, { "text", "roms" }, { "text", "run" },
 { "text", "running" }, { "text", "russian" }, { "text", "s" }, { "text", "safe" },
 { "text", "saloranta" }, { "text", "same" }, { "text", "scarab" }, { "text", "score" },
 { "text", "scored" }, { "text", "scorer" }, { "text", "scores" }, { "text", "scoring" },
 { "text", "scottganyo" }, { "text", "scratch" }, { "text", "scripts" }, { "text", "search" },
 { "text", "searchable" }, { "text", "searches" }, { "text", "searching" }, { "text", "section" },
 { "text", "see" }, { "text", "segment" }, { "text", "segmentreader" }, { "text", "segments" },
 { "text", "segmentsreader" }, { "text", "segmenttermenum" }, { "text", "separate" }, { "text", "separately" },
 { "text", "sequences" }, { "text", "serebrennikov" }, { "text", "serializable" }, { "text", "serious" },
 { "text", "serves" }, { "text", "setboost" }, { "text", "setnorm" }, { "text", "setpositionincrement" },
 { "text", "setting" }, { "text", "settings" }, { "text", "shorter" }, { "text", "should" },
 { "text", "show" }, { "text", "similarity" }, { "text", "since" }, { "text", "single" },
 { "text", "skip" }, { "text", "slightly" }, { "text", "small" }, { "text", "smiley" },
 { "text", "so" }, { "text", "some" }, { "text", "someone" }, { "text", "sometimes" },
 { "text", "soon" }, { "text", "sorting" }, { "text", "source" }, { "text", "sourceforge" },
 { "text", "sources" }, { "text", "special" }, { "text", "specific" }, { "text", "specified" },
 { "text", "specify" }, { "text", "speed" }, { "text", "standardanalyzer" }, { "text", "standardtokenizer" },
 { "text", "start" }, { "text", "started" }, { "text", "starting" }, { "text", "static" },
 { "text", "steichen" }, { "text", "stemmer" }, { "text", "stemmers" }, { "text", "stemming" },
 { "text", "stems" }, { "text", "still" }, { "text", "stop" }, { "text", "stored" },
 { "text", "stream" }, { "text", "string" }, { "text", "stripping" }, { "text", "structure" },
 { "text", "stuff" }, { "text", "subclassed" }, { "text", "substantially" }, { "text", "such" },
 { "text", "support" }, { "text", "supported" }, { "text", "supporting" }, { "text", "supports" },
 { "text", "switched" }, { "text", "syntax" }, { "text", "system" }, { "text", "systems" },
 { "text", "t" }, { "text", "tag" }, { "text", "take" }, { "text", "taking" },
 { "text", "tan" }, { "text", "tatu" }, { "text", "term" }, { "text", "termdocs" },
 { "text", "termenum" }, { "text", "termquery" }, { "text", "terms" }, { "text", "terry" },
 { "text", "test" }, { "text", "tested" }, { "text", "testphraseprefixquery" }, { "text", "testrussianstem" },
 { "text", "tests" }, { "text", "testwildcard" }, { "text", "text" }, { "text", "than" },
 { "text", "that" }, { "text", "the" }, { "text", "their" }, { "text", "them" },
 { "text", "there" }, { "text", "these" }, { "text", "they" }, { "text", "things" },
 { "text", "this" }, { "text", "those" }, { "text", "thread" }, { "text", "threads" },
 { "text", "throw" }, { "text", "throwing" }, { "text", "thrown" }, { "text", "thus" },
 { "text", "timestamp" }, { "text", "timestamps" }, { "text", "titles" }, { "text", "tmpdir" },
 { "text", "to" }, { "text", "tohtml" }, { "text", "token" }, { "text", "tokenization" },
 { "text", "tokenizer" }, { "text", "tokenmgrerror" }, { "text", "tokens" }, { "text", "toomanyclauses" },
 { "text", "tostring" }, { "text", "total" }, { "text", "true" }, { "text", "tucker" },
 { "text", "two" }, { "text", "txt" }, { "text", "undeleteall" }, { "text", "undeletes" },
 { "text", "under" }, { "text", "unicode" }, { "text", "unindexed" }, { "text", "unit" },
 { "text", "unlock" }, { "text", "up" }, { "text", "update" }, { "text", "updated" },
 { "text", "upgraded" }, { "text", "upper" }, { "text", "usage" }, { "text", "use" },
 { "text", "used" }, { "text", "useful" }, { "text", "users" }, { "text", "uses" },
 { "text", "using" }, { "text", "v" }, { "text", "value" }, { "text", "values" },
 { "text", "various" }, { "text", "verbs" }, { "text", "version" }, { "text", "versus" },
 { "text", "very" }, { "text", "via" }, { "text", "void" }, { "text", "was" },
 { "text", "way" }, { "text", "website" }, { "text", "week" }, { "text", "weight" },
 { "text", "were" }, { "text", "when" }, { "text", "where" }, { "text", "which" },
 { "text", "while" }, { "text", "whitespace" }, { "text", "wildcard" }, { "text", "wildcardquery" },
 { "text", "will" }, { "text", "windows" }, { "text", "with" }, { "text", "within" },
 { "text", "without" }, { "text", "word" }, { "text", "words" }, { "text", "work" },
 { "text", "works" }, { "text", "would" }, { "text", "write" }, { "text", "writer" },
 { "text", "x" }, { "text", "xml" }, { "text", "xxx" }, { "text", "yet" },
 { "text", "yield" }, { "text", "zero" } };


static void
test_check_reader(CuTest *tc )
{
    apr_pool_t *pool;
    lcn_index_reader_t *index_reader;
    lcn_document_t *document;
    int freq;
    lcn_bool_t has_field;

    LCN_TEST( apr_pool_create( &pool, main_pool ) );
    LCN_TEST( lcn_index_reader_create_by_path( &index_reader, test_index_name, pool ) );

    LCN_TEST( lcn_index_reader_has_field( index_reader, &has_field, "text" ));
    CuAssertTrue( tc, has_field );

    LCN_TEST( lcn_index_reader_has_field( index_reader, &has_field, "textt" ));
    CuAssertTrue( tc, ! has_field );

    LCN_TEST( lcn_index_reader_has_field( index_reader, &has_field, "id" ));
    CuAssertTrue( tc, has_field );

    LCN_TEST( lcn_index_reader_has_field( index_reader, &has_field, "content" ));
    CuAssertTrue( tc, has_field );

    LCN_TEST( lcn_index_reader_has_field( index_reader, &has_field, "blubb" ));
    CuAssertTrue( tc, ! has_field );

    {
        lcn_term_t *term;
        LCN_TEST( lcn_term_create( &term, "text", "a", LCN_TERM_TEXT_COPY, pool ) );
        LCN_TEST( lcn_index_reader_doc_freq( index_reader, term, &freq ) );

        CuAssertIntEquals(tc, 47, freq );

        LCN_TEST( lcn_term_create( &term, "text", "zzzzzzzzzzzzzzzzzzzzz", LCN_TERM_TEXT_COPY, pool ) );
        LCN_TEST( lcn_index_reader_doc_freq( index_reader, term, &freq ) );
        CuAssertIntEquals(tc, 0, freq );

        LCN_TEST( lcn_term_create( &term, "text", "the", LCN_TERM_TEXT_COPY, pool ) );
        LCN_TEST( lcn_index_reader_doc_freq( index_reader, term, &freq ) );
        CuAssertIntEquals(tc, 66, freq );
    }

    CuAssertIntEquals(tc, 331, lcn_index_reader_num_docs( index_reader ) );
    CuAssertIntEquals(tc, 331, lcn_index_reader_max_doc( index_reader ) );
    CuAssertTrue(tc, !lcn_index_reader_has_deletions( index_reader ) );
    CuAssertTrue(tc, !lcn_index_reader_is_deleted( index_reader, 5 ) );
    CuAssertTrue(tc, !lcn_index_reader_is_deleted( index_reader, 205 ) );

    LCN_TEST( lcn_index_reader_document( index_reader, &document, 5, pool ));
    LCN_TEST( lcn_index_reader_delete( index_reader, 5 ) );
    CuAssertTrue(tc, lcn_index_reader_has_deletions( index_reader ) );
    CuAssertIntEquals(tc, 330, lcn_index_reader_num_docs( index_reader ) );
    CuAssertIntEquals(tc, 331, lcn_index_reader_max_doc( index_reader ) );

    LCN_TEST( lcn_index_reader_delete( index_reader, 205 ) );
    CuAssertTrue(tc, lcn_index_reader_has_deletions( index_reader ) );
    CuAssertIntEquals(tc, 329, lcn_index_reader_num_docs( index_reader ) );
    CuAssertIntEquals(tc, 331, lcn_index_reader_max_doc( index_reader ) );
    CuAssertTrue(tc, lcn_index_reader_is_deleted( index_reader, 5 ) );
    CuAssertTrue(tc, lcn_index_reader_is_deleted( index_reader, 205 ) );
    LCN_ERR( lcn_index_reader_document( index_reader, &document, 5, pool ),
             LCN_ERR_ACCESS_DELETED_DOCUMENT);

    LCN_TEST( lcn_index_reader_undelete_all( index_reader ) );

    CuAssertTrue(tc, ! lcn_index_reader_has_deletions( index_reader ) );
    CuAssertIntEquals(tc, 331, lcn_index_reader_num_docs( index_reader ) );
    CuAssertIntEquals(tc, 331, lcn_index_reader_max_doc( index_reader ) );
    CuAssertTrue(tc, !lcn_index_reader_is_deleted( index_reader, 5 ) );
    CuAssertTrue(tc, !lcn_index_reader_is_deleted( index_reader, 205 ) );

    LCN_TEST( lcn_index_reader_delete( index_reader, 5 ) );
    LCN_TEST( lcn_index_reader_delete( index_reader, 205 ) );
    LCN_TEST( lcn_index_reader_close( index_reader ) );

    apr_pool_clear( pool );

    LCN_TEST( lcn_index_reader_create_by_path( &index_reader, test_index_name, pool ) );
    CuAssertIntEquals(tc, 329, lcn_index_reader_num_docs( index_reader ) );
    CuAssertIntEquals(tc, 331, lcn_index_reader_max_doc( index_reader ) );
    CuAssertTrue(tc, lcn_index_reader_has_deletions( index_reader ) );
    CuAssertTrue(tc, lcn_index_reader_is_deleted( index_reader, 5 ) );
    CuAssertTrue(tc, lcn_index_reader_is_deleted( index_reader, 205 ) );


    LCN_TEST( lcn_index_reader_undelete_all( index_reader ) );
    CuAssertTrue(tc, ! lcn_index_reader_has_deletions( index_reader ) );
    CuAssertIntEquals(tc, 331, lcn_index_reader_num_docs( index_reader ) );
    CuAssertIntEquals(tc, 331, lcn_index_reader_max_doc( index_reader ) );
    LCN_TEST( lcn_index_reader_close( index_reader ) );

    apr_pool_clear( pool );

    LCN_TEST( lcn_index_reader_create_by_path( &index_reader, test_index_name, pool ) );
    CuAssertIntEquals(tc, 331, lcn_index_reader_num_docs( index_reader ) );
    CuAssertIntEquals(tc, 331, lcn_index_reader_max_doc( index_reader ) );
    CuAssertTrue(tc, ! lcn_index_reader_has_deletions( index_reader ) );

    {
        apr_pool_t *te_pool;
        lcn_term_enum_t *term_enum;
        apr_status_t next_status;
        unsigned int i = 0;

        LCN_TEST( apr_pool_create( &te_pool, pool ) );
        LCN_TEST( lcn_index_reader_terms( index_reader, &term_enum, te_pool ));

        do
        {
            const lcn_term_t *term;

            next_status = lcn_term_enum_next( term_enum );
	    term = lcn_term_enum_term( term_enum );

            if ( LCN_ERR_ITERATOR_NO_NEXT == next_status )
            {
                break;
            }

            CuAssertStrEquals(tc, term_list[i][0], lcn_term_field(term) );
            CuAssertStrEquals(tc, term_list[i][1], lcn_term_text(term) );


            i++;
        }
        while( APR_SUCCESS == next_status );

        CuAssertIntEquals(tc, 1158, i );

        LCN_TEST( lcn_term_enum_close( term_enum ) );
        apr_pool_destroy( te_pool );
    }

    {

        apr_pool_t *te_pool;
        lcn_term_enum_t *term_enum;
        apr_status_t next_status;
        lcn_term_t *term;
        unsigned int i = 0;

        LCN_TEST( apr_pool_create( &te_pool, pool ) );
        LCN_TEST( lcn_term_create( &term, "text", "limis", LCN_TERM_TEXT_COPY, te_pool ) );
        LCN_TEST( lcn_index_reader_terms_from( index_reader, &term_enum, term, te_pool ));

        term = (lcn_term_t*) lcn_term_enum_term( term_enum );

        do
        {
            const lcn_term_t *term;

            next_status = lcn_term_enum_next( term_enum );

            if ( LCN_ERR_ITERATOR_NO_NEXT == next_status )
            {
                break;
            }

            term = lcn_term_enum_term( term_enum );

            //fprintf(stderr, "GOT %s %d (%s)\n", lcn_term_text( term ), i + 738, term_list[i+738][1]);

            CuAssertStrEquals(tc, term_list[i+738][0], lcn_term_field(term) );
            CuAssertStrEquals(tc, term_list[i+738][1], lcn_term_text(term) );

            i++;
        }
        while( APR_SUCCESS == next_status );

        CuAssertIntEquals(tc, 420, i );

        LCN_TEST( lcn_term_enum_close( term_enum ) );
        apr_pool_destroy( te_pool );
    }

    LCN_TEST( lcn_index_reader_close( index_reader ) );
    apr_pool_destroy( pool );
}

static void
test_fields_reader(CuTest* tc)
{
    apr_pool_t *pool;
    lcn_index_reader_t *index_reader;
    lcn_document_t *document;
    LCN_TEST( apr_pool_create( &pool, main_pool ));
    LCN_TEST( lcn_index_reader_create_by_path( &index_reader, test_index_name, pool ) );
    {
        lcn_field_t *field;
        apr_pool_t *doc_pool;
        LCN_TEST( apr_pool_create( &doc_pool, pool ) );

        LCN_TEST( lcn_index_reader_document( index_reader, &document, 78, doc_pool ) );
        LCN_TEST( lcn_document_get_field( document, "content", &field ) );
        CuAssertStrEquals(tc, "deleted documents which still remain in the index. (cutting)",
                          lcn_field_value( field ) );

        apr_pool_destroy( doc_pool );
    }

    LCN_TEST( lcn_index_reader_close( index_reader ));
    apr_pool_destroy( pool );
}

static void
test_get_field_infos_impl(CuTest* tc, const char *path)
{
    apr_pool_t *pool;
    lcn_index_reader_t *index_reader;

    LCN_TEST( apr_pool_create( &pool, main_pool ));
    LCN_TEST( lcn_index_reader_create_by_path( &index_reader, path, pool ) );

    {
        lcn_field_info_t *field_info;
        lcn_list_t *field_infos;
        int i;

        apr_pool_t *fi_pool;

        LCN_TEST( apr_pool_create( &fi_pool, pool ) );
        LCN_TEST( lcn_list_create( &field_infos, 10, fi_pool ));

        for( i = 0; i < 3; i++ )
        {
            /* get fields which store BOTH offset and position with termvector */
            /* IndexReader.FieldOption.TERMVECTOR_WITH_POSITION_OFFSET         */

            LCN_TEST( lcn_index_reader_get_field_infos( index_reader, field_infos,
                                                        LCN_FIELD_STORE_OFFSET_WITH_TV | LCN_FIELD_STORE_POSITION_WITH_TV,
                                                        LCN_FIELD_STORE_OFFSET_WITH_TV | LCN_FIELD_STORE_POSITION_WITH_TV ) );

            CuAssertIntEquals(tc, 2, lcn_list_size( field_infos ) );

            field_info = lcn_list_get( field_infos,  0 );
            CuAssertStrEquals( tc, "tv_po_is", lcn_field_info_name( field_info ) );
            field_info = lcn_list_get( field_infos, 1 );
            CuAssertStrEquals( tc, "tv_po_i", lcn_field_info_name( field_info ) );
        }

        for( i = 0; i < 3; i++ )
        {
            /* get fields which store position but NOT offset with termvector */

            LCN_TEST( lcn_index_reader_get_field_infos( index_reader, field_infos,
                                                        LCN_FIELD_STORE_POSITION_WITH_TV,
                                                        LCN_FIELD_STORE_OFFSET_WITH_TV | LCN_FIELD_STORE_POSITION_WITH_TV ) );

            CuAssertIntEquals(tc, 4, lcn_list_size( field_infos ) );

            field_info = lcn_list_get( field_infos, 2 );
            CuAssertStrEquals( tc, "tv_p_i", lcn_field_info_name( field_info ) );
            field_info = lcn_list_get( field_infos, 3 );
            CuAssertStrEquals( tc, "tv_p_is", lcn_field_info_name( field_info ) );
        }

        for( i = 0; i < 3; i++ )
        {
            /* get fields which store offset but NOT position with termvector */

            LCN_TEST( lcn_index_reader_get_field_infos( index_reader, field_infos,
                                                        LCN_FIELD_STORE_OFFSET_WITH_TV,
                                                        LCN_FIELD_STORE_OFFSET_WITH_TV | LCN_FIELD_STORE_POSITION_WITH_TV ) );

            CuAssertIntEquals(tc, 6, lcn_list_size( field_infos ) );

            field_info = lcn_list_get( field_infos, 4 );
            CuAssertStrEquals( tc, "tv_o_i", lcn_field_info_name( field_info ) );
            field_info = lcn_list_get( field_infos, 5 );
            CuAssertStrEquals( tc, "tv_o_is", lcn_field_info_name( field_info ) );
        }

        for( i = 0; i < 3; i++ )
        {
            /* get fields which store NEITHER offset NOR position with termvector */

            LCN_TEST( lcn_index_reader_get_field_infos( index_reader, field_infos,
                                                        LCN_FIELD_STORE_TERM_VECTOR,
                                                        LCN_FIELD_STORE_TERM_VECTOR     |
                                                        LCN_FIELD_STORE_OFFSET_WITH_TV  |
                                                        LCN_FIELD_STORE_POSITION_WITH_TV ) );

            CuAssertIntEquals(tc, 8, lcn_list_size( field_infos ) );

            field_info = lcn_list_get( field_infos, 6 );
            CuAssertStrEquals( tc, "tv_i", lcn_field_info_name( field_info ) );
            field_info = lcn_list_get( field_infos, 7 );
            CuAssertStrEquals( tc, "tv_is", lcn_field_info_name( field_info ) );
        }

        for( i = 0; i < 3; i++ )
        {
            /* get indexed fields which do not store term_vector */

            LCN_TEST( lcn_index_reader_get_field_infos( index_reader, field_infos,
                                                        LCN_FIELD_INDEXED,
                                                        LCN_FIELD_INDEXED               |
                                                        LCN_FIELD_STORE_TERM_VECTOR     |
                                                        LCN_FIELD_STORE_OFFSET_WITH_TV  |
                                                        LCN_FIELD_STORE_POSITION_WITH_TV ) );

            CuAssertIntEquals(tc, 11, lcn_list_size( field_infos ) );

            field_info = lcn_list_get( field_infos, 8 );
            CuAssertStrEquals( tc, "text", lcn_field_info_name( field_info ) );
            field_info = lcn_list_get( field_infos, 9 );
            CuAssertStrEquals( tc, "id", lcn_field_info_name( field_info ) );
            field_info = lcn_list_get( field_infos, 10 );
            CuAssertStrEquals( tc, "ntv_i", lcn_field_info_name( field_info ) );
        }

        for( i = 0; i < 3; i++ )
        {
            /* get unindexed fields */

            LCN_TEST( lcn_index_reader_get_field_infos( index_reader, field_infos,
                                                        0,
                                                        LCN_FIELD_INDEXED               |
                                                        LCN_FIELD_STORE_TERM_VECTOR     |
                                                        LCN_FIELD_STORE_OFFSET_WITH_TV  |
                                                        LCN_FIELD_STORE_POSITION_WITH_TV ) );

            CuAssertIntEquals(tc, 12, lcn_list_size( field_infos ) );

            field_info = lcn_list_get( field_infos, 11 );
            CuAssertStrEquals( tc, "content", lcn_field_info_name( field_info ) );
        }

        apr_pool_destroy( fi_pool );
    }

    LCN_TEST( lcn_index_reader_close( index_reader ));
    apr_pool_destroy( pool );
}

static void
test_get_field_infos(CuTest* tc)
{
    test_get_field_infos_impl(tc, "test_index_fi_opt");
    test_get_field_infos_impl(tc, "test_index_fiel_infos");
}

static void
test_norms( CuTest *tc )
{
    apr_pool_t *pool;
    lcn_index_reader_t *index_reader;
    lcn_byte_array_t* norms;

    LCN_TEST( apr_pool_create( &pool, main_pool ));
    LCN_TEST( lcn_index_reader_create_by_path( &index_reader, test_index_name, pool ) );

    LCN_TEST( lcn_index_reader_norms( index_reader,
                                      &norms,
                                      "text" ));
    
    CuAssertIntEquals( tc, 331, norms->length );

#if 0
    unsigned int i;

    for( i = 0; i < 331; i++ )
    {
        //CuAssertIntEquals( tc, 124, norms->arr[i] );
        fprintf(stderr, "nrm = %d\n", (int) (unsigned char) (norms->arr[i] ));
    }
#endif
    LCN_TEST( lcn_index_reader_close( index_reader ) );
}

static void
test_empty_dir(CuTest* tc)
{
    apr_pool_t *pool;
    lcn_index_reader_t *index_reader;

    delete_files( tc, "test_index_writer" );
    LCN_TEST( apr_pool_create( &pool, main_pool ));
    LCN_TEST( lcn_index_reader_create_by_path( &index_reader, "test_index_writer", pool ) );
}

static void
setup()
{
    test_index_name = "test_index_2";
}

static void
setup_multi()
{
    test_index_name = "test_index_1";
}

static void
setup_compound_file()
{
    test_index_name = "cf_index";
}

#define CHECK_TERM_ENUM( text, stat, next_text )                                                \
{                                                                                               \
    LCN_TEST( apr_pool_create( &te_pool, pool ) );                                              \
    LCN_TEST( lcn_term_create( &term, "text", text, LCN_TERM_TEXT_COPY, te_pool ) );            \
    LCN_TEST( lcn_index_reader_terms_from( index_reader, &term_enum, term, te_pool ));          \
                                                                                                \
    CuAssertIntEquals( tc, stat, lcn_term_enum_next( term_enum ));                              \
                                                                                                \
    if ( APR_SUCCESS == stat )                                                                  \
    {                                                                                           \
        term = (lcn_term_t*) lcn_term_enum_term( term_enum );                                   \
        CuAssertStrEquals( tc, next_text, lcn_term_text( term ));                               \
    }                                                                                           \
                                                                                                \
    LCN_TEST( lcn_term_enum_close( term_enum ) );                                               \
    apr_pool_destroy( te_pool );                                                                \
}

static void
test_term_enum_from( CuTest *tc )
{
    apr_pool_t *pool;
    lcn_index_reader_t *index_reader;

    LCN_TEST( apr_pool_create( &pool, main_pool ));
    LCN_TEST( lcn_index_reader_create_by_path( &index_reader, test_index_name, pool ) );
    
    {
        apr_pool_t *te_pool;
        lcn_term_enum_t *term_enum;
        lcn_term_t *term;

        CHECK_TERM_ENUM( "limis", APR_SUCCESS, "limit" );
        CHECK_TERM_ENUM( "x", APR_SUCCESS, "x" );
        CHECK_TERM_ENUM( "xa", APR_SUCCESS, "xml" );
        CHECK_TERM_ENUM( "xxxx", APR_SUCCESS, "yet" );
        CHECK_TERM_ENUM( "yet", APR_SUCCESS, "yet" );
        CHECK_TERM_ENUM( "yetx", APR_SUCCESS, "yield" );
        CHECK_TERM_ENUM( "zer", APR_SUCCESS, "zero" );
        CHECK_TERM_ENUM( "zero", APR_SUCCESS, "zero" );
        CHECK_TERM_ENUM( "zerox", LCN_ERR_ITERATOR_NO_NEXT, "zero" );
    }

    LCN_TEST( lcn_index_reader_close( index_reader ));
}

static void
test_get_int_field_values_impl( CuTest *tc, int default_val )
{
    apr_pool_t *pool;

    apr_pool_create( &pool, main_pool );

    {
        apr_hash_t* map;

        apr_pool_create( &pool, main_pool );
        delete_files( tc, "sort_test" );
        LCN_TEST( lcn_analyzer_map_create( &map, pool ) );
        LCN_TEST( lcn_index_writer_create_index_by_dump( "sort_test",
                                                         "sort.txt",
                                                         map,
                                                         LCN_TRUE, /* optimize */
                                                         pool ));
        apr_pool_destroy( pool );
    }

    {
        lcn_index_reader_t *index_reader;
        lcn_int_array_t *int_array;
        unsigned int i;

        apr_pool_create( &pool, main_pool );

        LCN_TEST( lcn_index_reader_create_by_path( &index_reader, "sort_test", pool ) );

        LCN_TEST( lcn_index_reader_get_int_field_values( index_reader,
                                                         &int_array,
                                                         "int_field",
                                                         default_val ));

        for( i = 0; i < int_array->length; i++ )
        {
            lcn_document_t *document;
            lcn_field_t *field;
            apr_status_t stat;

            LCN_TEST( lcn_index_reader_document( index_reader, &document, i, pool ));
            stat = lcn_document_get_field( document, "int_field", &field );

            if ( APR_SUCCESS == stat )
            {
                const char* field_val = lcn_field_value( field );
                CuAssertIntEquals( tc, atoi(field_val), int_array->arr[i]);
            }
            else
            {
                CuAssertIntEquals( tc, default_val, int_array->arr[i] );
            }
        }

        LCN_TEST( lcn_index_reader_close( index_reader ) );

        apr_pool_destroy( pool );
    }
}

static void
test_get_int_field_values( CuTest *tc )
{
    test_get_int_field_values_impl( tc, 0 );
    test_get_int_field_values_impl( tc, 1 );
    test_get_int_field_values_impl( tc, 13 );
    test_get_int_field_values_impl( tc, 100000 );
}

static void
make_csf_index( CuTest *tc )
{
    apr_pool_t *pool;
    apr_pool_create( &pool, main_pool );
    
    create_index_cf(tc, 0, 331, "cf_index", LCN_TRUE , pool);
}

CuSuite *make_index_reader_suite (void)
{    
    CuSuite *s = CuSuiteNew();
    
    
    SUITE_ADD_TEST( s, setup );
    SUITE_ADD_TEST( s, test_get_int_field_values );
    SUITE_ADD_TEST( s, test_norms );
    SUITE_ADD_TEST( s, test_check_reader);
    SUITE_ADD_TEST( s, test_fields_reader );
    SUITE_ADD_TEST( s, test_get_field_infos );
    SUITE_ADD_TEST( s, test_term_enum_from );

    SUITE_ADD_TEST( s, setup_multi );
    SUITE_ADD_TEST( s, test_get_int_field_values );
    SUITE_ADD_TEST( s, test_norms );
    SUITE_ADD_TEST( s, test_check_reader );

    SUITE_ADD_TEST( s, test_fields_reader );
    SUITE_ADD_TEST( s, test_get_field_infos );
    SUITE_ADD_TEST( s, test_term_enum_from );

    /*
     * Test für die compound file Komponenten
     */
    SUITE_ADD_TEST( s, make_csf_index );
    SUITE_ADD_TEST( s, setup_compound_file );
    SUITE_ADD_TEST( s, test_get_int_field_values );
    SUITE_ADD_TEST( s, test_norms );
    SUITE_ADD_TEST( s, test_fields_reader );
    SUITE_ADD_TEST( s, test_get_field_infos );
    SUITE_ADD_TEST( s, test_term_enum_from );

    return s;
}

