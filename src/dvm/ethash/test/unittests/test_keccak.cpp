// ethash: C/C++ implementation of Ethash, the Ethereum Proof of Work algorithm.
// 
// Licensed under the Apache License, Version 2.0.

#include <ethash/keccak.hpp>

#include "helpers.hpp"

#include <gtest/gtest.h>

using namespace ethash;

struct keccak_test_case
{
    const size_t input_size;
    const char* const expected_hash256;
    const char* const expected_hash512;
};

static const char* test_text =
    "As Estha stirred the thick jam he thought Two Thoughts and the Two Thoughts he thought were "
    "these: a) Anything can happen to anyone. and b) It is best to be prepared.";

// clang-format off
static keccak_test_case test_cases[] = {
    {  0, "c5d2460186f7233c927e7db2dcc703c0e500b653ca82273b7bfad8045d85a470", "0eab42de4c3ceb9235fc91acffe746b29c29a8c366b7c60e4e67c466f36a4304c00fa9caf9d87976ba469bcbe06713b435f091ef2769fb160cdab33d3670680e"},
    {  1, "03783fac2efed8fbc9ad443e592ee30e61d65f471140c10ca155e937b435b760", "421a35a60054e5f383b6137e43d44e998f496748cc77258240ccfaa8730b51f40cf47c1bc09c728a8cd4f096731298d51463f15af89543fed478053346260c38"},
    {  2, "8f9d2f3ed6ac64e8772bbb33bf0d0830e39c0d587064f29acd10a298bb540f6b", "24122319f80a58300d42f26d1869f4b658cb846405838842217767937062c1f8ecf3800ad97da684d3ad1ef32176330fe3ba45bda4deccd5a87e7c33742458ab"},
    {  3, "275613398ce7fd7cb405a72511be5fee893fd6ea8fdc8d5b162ab5427a800c03", "2940fa3f434224dcc6a3809454ec7aaed750d30b1c58f614ce80f02cddf6bfa5cb8e8cf82a2078f48e4b1cc35703dc4a9b33d55b438a318c9dbb5e46356a3853"},
    {  4, "010d5029011787fe9f9eaab2ceaf8ff50f2baffcef2a43c0a869ee422c7eaead", "80b6051ad8aad62f98e566dba6c2ae0894134ab6c8ec371a4e3cedbdb296c1129230dc624f774bfba9a4721a23f1a1996a65319ae2452cb85458488c6252fdd2"},
    {  5, "817a5511bc0c271b37c00f64f90ddca328a04b15046cd0c755119991c1ab985f", "320bd9a897449c267c6122878eb4c2cd3f7aa4822d1f377b8a93bb307e7ed775d84ff9a24686892069e54378e1e639a3cca0df1b838e84cbf442c339b0de703e"},
    {  6, "4a5c773d1f7e9ee199319c7dbe00a14b9a0a37f7bf36848e7612b84db808ce32", "a5a3a5e7a08dadab873ca30b82fa3932bab246be53bc30905b88e420fa837065d60b8dae1214e15f7e8b7820e49c38a3f8d134c46120980774ae8f18ac290f7f"},
    {  7, "e227500b366e01498e7d3cd255c218d5e5f476cbc39545a6d24e8ff9a8aefba3", "ce039e166c3c65f2cb4ded0581c713c0cac5c889df7edfc80e0f142952cd42a1ce0b0fc6babf94fac8c6f004f381ec1313d700a94426aaa95476e943258a09d9"},
    {  8, "3199aa06ffa932894093e0c91eb2217d7e8d8250b8cc427104bd0d1954cc1fc2", "9001f91f47e4040b1a9c60bc70ce1d7a5a2d4f2b3ee9e99b9804e845fba77fede57c69d60184f26a779967157ec4560e94af95134810308bf551d39b7e14b01b"},
    {  9, "71af15db68a9c46d404979e3138515e5268a30daa82a0ae5903679e445dfe2a1", "1ebe7ac3fd63df5a49ea4fba6832e1e80fa93a75af8e36f856b9e370fc3c33849e64a860aa23b38802a7fa985c18ebab244a28281cf0250b37afaf6a7fc02f29"},
    { 10, "3ad41ce4561a235f91c247beb6bbd078bd56b549fbee877a1b7412d3623aa596", "3af9bb94e229eefc0223e84e24bec25ed3c02fa1a2b7935fc0d649bbdff436150a71c675edad1c7b25f10967ebee9774c130d512559841bffab1fb371b1a8900"},
    { 11, "fd48650e368a8b90ae2ffdbb6a0b7986afa885de4abb1fcf73ddbd40d7e080f7", "e7bb8ab1ac64210e4e37f2676638f343a41f9a1d84ea8ad7857e86495e68d1ff5d297a2b082383fc421312c0bbe90347d20857bca7bcf0d16c505efdb8bb9cca"},
    { 12, "3902a8482d98ab34ccd656009c5571cac0f9021ac9949732701b57e305d526c1", "d8e50631ad3027bbbc16746e1b9c5e658b17924d6035c47d37dff1a4d2c0dae0880d9aaecc9fec542e3e4510d98c08e07cd4165eca701d251f231c2f063680ea"},
    { 13, "d18395db4b8fbe43f9154f3b6cd2bfbb27565c201a944bb8d6a6cc38ac5d1daf", "f2331d6875fedf15c3d8c9ea70c3bdd44cdeaeac4cf59b72917e4900d52aee75f670ab00b68b918c542c79f4caf7d731141880aaee5b3b2acbb8280665b4b443"},
    { 14, "53a32859785d569119cccb7c4a22a04e62f6bb9ffe9495a8b31c348d82722c19", "693fcabc8aaf3ba94a053137c17737b4d0cb7597a9db6b4fcc00d154658e43a6bf7eb1eafc1294c184554e70100259bce32880c0089fc30a0be51a20b91dd4cb"},
    { 15, "c7bceb7f9f4cd02610d1cb03838cc614d39b8fb1c42a34a80fc877a168ac96da", "b598e263fbbfedbd2678563ad1ac70a452a6beffe39cd3233fc72a66000a46646c1a71672ff829d040b9f0dcdc8f9451e38d8367211f8ea315506d8c13927aea"},
    { 16, "be165de8bc0b514e1fcad3ab171ab73d05b7a8db20c0572f86165efd5122c6ce", "96854f3fe828bbf7199f5bef9c9cbcd31ba9307cca094a2acc298f3740f36ebdf72f2a6f2ab5c75ec5531ea3862217a1f0e88e3dc74fec8e6711a78ba4524081"},
    { 17, "e1e353f8f5ac4e641403f6f13350e509e8c1083fec714acc1df98afd76f13676", "5234cf95b5f15df23d728c675796f26a68515e0e9d42b2887f22cfcbae0233dcbd3587f7e00d1012c55c1c648af1cf8a4ae2421b7d3912343be16b4a4abfbb47"},
    { 18, "2856656c4dbc71d87938e06604d076f58368839d1d9bf1a710b1c698e1becb88", "ea53870ba492d500c76ca271888f5780c95c302bea41431caeb5c85b11682fa9fb332218a026ec316ff6c0d5ce408e147039f8e4fb3fa434c6dc0012d5adc97b"},
    { 19, "50edca5e811d8bcc59ba1a135774bc400f1721776b546fca7dc942e04539a90b", "ba7538cb54735ceeb1f736ac5bf9b5ea9ab98ec99cac5b01c084d7cc79c1b44e44249bd1ec5230175dd127ce57aeb1cbcb709d82fbdf0012724bf7d979250673"},
    { 20, "ee42804f5ed6b1aa8ebc32872289a90938f782e772b3c3be1912ba2be3bf5355", "bc04d7b11ab336124b75fdd48927fcaac2144d8f521124264f1e91e64961a423a878d38371880e8c29285c15c857b6a270ebb783d75b228f6025bb0f23e59d9c"},
    { 21, "44bc5fe4c92594f234b9dbbf9035ea9ddc6596f60b6f2b30a30358ccd0dba1b6", "538d8d51b209c391bfd25e60bb3000dd9d4dd73240a9ca0c5b0ba8ec9a7bef815dd929ef574057f4b5ace8fe481d8245471d6000a1e428486e47003b748835bb"},
    { 22, "c422a79d35560ec6a850fa69315aa1d418ec8ee85c570d68c576d6af3211ce35", "ac19a0b34d1aaeaa6d614ccff08a76dffeab266d2aa5116db670bb64ddd13c3a8654039d38203df55636bc6156a60b633996296533394e11b9b177ee7ae38f48"},
    { 23, "3f3ccf45b7e205a4c281d75ef1a5dbe0fe8d6e39c6428daa4c6983713160464c", "57a1e74f711f6fa1be3e2bbc284e976cdaef8427f7cdbff6bcbebdf6ea8eff2758b55ee9608c60579f1acdcac6c9879900258455371a0c5098e2262d13d62719"},
    { 24, "ecf999704a93a39758966fdd4f797c6b4819ffc0fe6accf9acf02f7e482b0a34", "6e453f98c406912f9186a47d0abb47f37393975a35e25647d40ae1e54dd593a458d3191b64e0f5eddc4ada9141f21559734e5cffc9be72c87408fd36d905d742"},
    { 25, "b5b67366d21d0b8b1b875ad82e0d0bd473e47e5daa49a19b1342a3f550e93d7d", "053163ce42a548391f935284e2e086ead81d39a1bf8396455b1b0058cb0fac2ce91e9200d321cc741be165ac97babec5d9317086247592797140516d7f18ed4c"},
    { 26, "90ba495c62b44ef397bae3b3ac22be808020f6c4d2f6149cdfab31b04ac5dbb4", "47be38b0bd382707e7fa015f5e48b7c713bd32cb06cd12b49f8d9c155a8fe2315144ee03ffe53f87e03b7ac01b0f4096089f3a3152ac3af79b1f7096f97712b9"},
    { 27, "0df6e5c95ab3ce26094752f14712b89c30030bf2a4c5ad5b9fffa50acbe20d4f", "bd0baeb2f9d0c69af63c3a0c235b1ca4ae687e54810676932e8aa7d36a6a0cf1f00f51d837ceb887649236f23d5fd353a389e52ae28b2d36c5e09cd564c46761"},
    { 28, "0946571d65c1f802ad95ca1fc1ebfdbe0a8a5392ebe64e2d0bb5345cc8392f20", "9b04773f9cf9a3b9ccb26fb1cce72e8ab8a072e49a1aa6b5f3c78d13e8ebb3923d1f49bd6c6479a1c64f748043d122caecb9dd56386c18497ebf6fa5ce1b6a65"},
    { 29, "942ca300fd42678f5e885d06a3d274041868d08042b1424bebb1adfa06135456", "64e1d0bd43f4309f53b3d3c0d5a3c51b03ad6c571178cd9fab09d06dc845ffee93853916e8e42935cbea15400608ba2e52313a42a802919b187020541540cc77"},
    { 30, "52f875475248e9e43333a350228616e5c99ad2e577419ac944010bf32708c7da", "bc13dff3195aa871a6315564cceeee2d652606a1e69c38aebc5ab44a08e0cdb9870aa88e66bc7d8a4f7c6573e595894751665d27d56322d8cf8bc7de14fbf7e4"},
    { 31, "d0d044d7361913fed7e7035712306c6912d969d72d6dacfe516d44903c6b59f5", "e3d0809fb6459a8c009d6df425e61ed530bc1e3031b6c839f12cb9ef5211ddc734ce34d5552184258bf52869942628520a284ca082c8c450d1352813f07c7815"},
    { 32, "ed6ebf3c52ce4ba6689a7ed19ad29d0d9e2e057d9509e5daa5a3c148ae14cdd4", "b1a5db864102d4a5999cd22a3a02dc68f555c9a77135df2174c35699f5f591ad403bea0ef7394ee68c4a18c9af4eb1de0ac1eac9ecc9a51cb9f72b1a4f7e26a9"},
    { 33, "3a23e7b6985b0b0f85cf54f0921095b178cd1c2e6d149106931e2cb98229f0d2", "e357109f8d330403a1de14b0af302237816a3560f0af98c72321479b6ba47649b7f66c9a958489eefcf412ba0513e2c751179d8e37f1f4a15564196b4111c5bb"},
    { 34, "8275a44d7a278f6b1dec2ef08f3d585b690997f1c7da44eda6823bb0d6bc4faf", "f6f946a2730573f5af6f64838a3ac388676ac63122390c2c45dcaa27fb672d85fbdd50216df021684fdb2e720528e0ce5bde61c9ff2dfb794a3e87e5f14c964a"},
    { 35, "cedb1f8c428aa4303a59706798b2d9ecb074ee8e689137eb0737092eb8787afc", "fae66d3fc7b1ca58dbe72cae5edf4f8fd4adf214f9bf350deb6f5d4a13927f7afd83acabb82ebad30dc79fbe5c832bed0cc28ebce3f7e6ed1727d42f3c6a3a14"},
    { 36, "c6cd5b90617121be1f789fde58afc67c48071d8dd5ffb740da61f95c4ffcc007", "0be55f9da8a946ff876fa66ae12ef71f470b5efb5cbc2a1b866e7afb1333164bc3be9fc978342569bdb5a0b82c6e68bfd4675397c9a87420f08ace10205d053c"},
    { 37, "3fc949fe7c4219425179ab8e8aff0ae923bf2e624e386f80881f7546cbb70cce", "3197669d1cf080f32045a596f6ee8a533997e7b9a63e9cb8138f262c17801f9383b0c5a8983584c7739c55b8ca2977ddfc519ba14b094d9ea7466008e441289b"},
    { 38, "2df5380528db739ca600721ab9b9f73629eddc312410d9dea1065cbff727838f", "ae8169496815d1048351b72fa778d9c705d3df88fb082b5af81d66c8d6640fc55014b508e1e08d4a3b3f0001ffee8c153a74fb3543ad99f4597320464cb9bd77"},
    { 39, "99508ec2ddf83e6b8cb206b45993b7d2234ad897452da89610337713103785ce", "3d60514c25788555f417c9d73e4360710e7e0eac33de622ec5fa4611b99f3ed72898b708ca5cdb8fcf71585c78b5010780af60777b96e11a7de0882ac704bbc4"},
    { 40, "a09ebed7442d2dbdab501ae7136aa11f9c38f957fe037d1431d296554a2458fd", "095f71af348019da5f52313d0192374ed14205fbd4765f4cac3dc8a14d74203f3a5c4878e7db450849cf8c99a12e351fd73852bce8c476364392ad3c329a563f"},
    { 41, "ed133e893a543acbf2b41f97b26765edd18d9ba847f5f1f98576bd05f9186aa5", "aae189182d9d01edb4cb089a40774697c1832ba0458aa85250453b81dffc448a987e0582d030b2e0a3a19edba2002e49d96b94985b27bc242c82f9e1077cfd8c"},
    { 42, "fb723e50d82b97f2f2e25930598c166b9c271f0529e8dee492ff9831758df6fd", "a9c8944cfdfdf7c47f91a11a396e1429b21694945f2a1c31e64168ae834790d5c90f999e2e45aed9463ef05e3b800a23b022d6dba2870fbc090c9e9f0190b8f8"},
    { 43, "6aa444cb7ac3702805ccd871fe172c61a14d598463a45ff4f92f9f0edab4393e", "863cfbb05ff7b01d7800cdc1dd7c0d42812354e5cc3931b7400d503692a1fb659958eeb615d953868fd0e28a5a5d159dd543c46d15423792803a62612e5257ef"},
    { 44, "41a6df7c4b7b1fe75f0de23b3d82e48a9291f0bec6ff638f5dbffca38747ad43", "7e822412e9c4d537a3fff4fb7653f57158b3ce1fd8c02e3aaf468f4cf8ecbd14d2ed81a674a5911c702440bc553f9eb555768312c6de1370926e5c09b644e568"},
    { 45, "8a2f5d9a933e529ca284554057c83db8fcd5c278a650f0b31affdbed9c897587", "66d5399980491abecaefb764bf6d20b2eaff0bd19c3a98b40cb9f7e2cfdf161e189f01db8900735e3927a690c473d6457b8adb8b92d9d574ca6c08e3fab34368"},
    { 46, "102dac44ec3a2dcc49026d6ec3560ea763605f5d8e413af4ecf65d54a72d1992", "8b59e9fe237169097da52689261ab6775faee621293b0804925b38f9e4520cf75282055ff736715929815c3c343725919fba5144132e566c7419c05fab4b89e1"},
    { 47, "3e1b7c0a882f589a998cedbb7f425c668c3225791b7344b57ca1a215dbdba1ec", "70dcf02b5250b3b8a780c500114e82c460afeabf0007fdde6c1c06b3430fe2238d78145740c2ce10648f8934590e57055dc2de404287c1b49cc9a8179d14244f"},
    { 48, "0f6baa5e4129bf4796eb46840dba51d7086fb5f7d27c0f2c956d7688f6eca80c", "5a932a4cafb7caeda2a1120f9b1eda2eaa84940e7fa883ea7bf2044dd4d9b26ea37f9c5a62ea1e4faa669febd2e2704760f86241ca2f34b33ae5ad476f9cd7a2"},
    { 49, "52be9251251038393f870e321fa69d4d898e1475a08fb9d3954295e6d88e2606", "ffd0cc72bb45124673f44fb0fd33a842398a931b925b6a94827a12c6fdf10b5ef45fe94a3d989abee20bee41ab2c82a360f157266f21e281d0f72356f823288e"},
    { 50, "111cbe35de34321ec32e71b4b6c8fcc0de078b2c89a722153d04f7bf0af3e255", "6af6ac5c94c092d3380bb3a916475d5bd1bba89ffba5be0c4990fdae1947c43ee52de2c997f35036b606b4d30e0c1d9427499ebf201bec02ad819d8aa78244b1"},
    { 51, "a6d02ef82af5747437e12a8860dfc6c2f45534f8d36a53012d5dc4e970ad1050", "cab092742041686bfbf822d8394316c17dba5380a72209ea0bd57df8ebd683abf83dcb783380b4349ab2baf53c4b0c187ef55a799d8c4eae92791044dcb25383"},
    { 52, "216347823bc50433e9055fe6057ffdd6151d2d25859725e132961bb073ed9f84", "646ef0f2333c2b8d2313e27de11f5f1e4f0657ed3b95b869e6943c3329b6c65af62a1840479067f0decfddafd7f628f4a818b3ce9638f908cb602f7fbbd1bbf6"},
    { 53, "f63811edd7f59114cf2679b5e25f4fb0a295f2f57463194b3b2faea88ce2042d", "7df67633d73750a8c0de9fa34ac5172554041410cce15b5d4b88ae19e24e7c94f8d554585e428b34b370f102081fc5febd41ee08563cb383bd08554e1c167516"},
    { 54, "f6377db0f43f983d208f41ce514ff58b1b0a417c196aa0ee0659371a62cb5f66", "0723839403a8d47146674e6b60224680854892bba543d1e1c92ac97098c6d3627d97176b28fec26e950902be4c8981e57c5d26a6d2bc3c81c19a51e5284b7149"},
    { 55, "fdae3a84eed58531f9833336a1fffa28f4c7cef68b1b6241422da6c212feb8ee", "0035b756c4c67570f4b0807309b0d03a56d8c3e9459482adae43248484fc0ae15e408eb5cfdc8c013c5617e3b720830368280c3a4c8b3b3aa19fb9e0154e893b"},
    { 56, "64fefc464a771a37f22abe02dbc37bd42054f971e0f4ea44875c5974c20b6dfc", "4a3e77b68cd3cf8126a6cd0cc7e59e4e976370d58665ef387c73b7f2db65f92c70b6e1b9e9278a5af7109913c98a939f7889b5e769ca2959b222df2514768412"},
    { 57, "d6f8da97620091de1bc7bae7c633846d06fef3ca9b716c5fbff812a1309c9172", "87cc9099110c56bc39626a7ac9b05c9849c44ede2a7366423c019b74645c6c711afeb53c58b6f60a55c32ed1411c78b1e97d58502d615c546b74e44a24cfd8c2"},
    { 58, "48e48c588912bb2d87819a2ae99430ec56b4523370715bba34ba75d0001c2a15", "f73bb7b37b58e85fb15a51bb6e24bd1ffd6ceae6df7a67820908a9c20d69efcb1f801569649fbda0880bea2d1e0a26914074efc86446027aa1b9ea933dbd4172"},
    { 59, "030fbdca12cce80a3589ad56c733523baa258184429b620e19f40cc03ea7c622", "6fd68f66d23aff00b6fd761abbf55fa9ede4e63962b13771930e8b5264c1de2fdc0b1f1863872b1d2deceb14420d1d70a3ccecfd8ed5c6fa49e3b0318180cc7c"},
    { 60, "d1f1edaf2fd8699242ba6888cf4d523f9f37f5497152d46055970bc923d3c19c", "1943bb53e68b8acd0c194974164db85055f40dbf2f91ab517a364204d5236d46dd3d508de2a7b293a0c36343e01ed49c05d19e962d464b914d86efe557f6a533"},
    { 61, "95ad70e63a5ce2ef49ce85a92f9ed244fbe62fce07aa99dea6a3d9ed2a579448", "4d456aaddee77dcc51a9a4c94c0f73c3aec80db4cbaea95395fb2270c3acbe66fc7365edb769adc2929cbe19dd45998777aeb16bd68f7515e9de1219828e3225"},
    { 62, "ab777e042d90ba746df35569ebb8445d2923a979faccbfbd33485df8c4ea0e75", "b4a81e4260e9f497407f945cebe9eebbf0b69c37a7ab1261c82af49d55bde13202c66e6e709d349d7b3bfaf1ef1ef12b3815000c9122fa343e97191cb165afdf"},
    { 63, "eccd3de4223b962451bcbcb2562b8393fee5da8ac6631dbbfab2e512652f6388", "0e163df92ad98b6a718a9e789c0599349ebe42563e7156c131cfb07d901665703d43de635ba090535cc3c99d0142b460f0ea95befe1c0b5f5e6deab852c63028"},
    { 64, "8ffaed16298a9e6a35d97f987af34816486d33a5cdcac6bf09e4e038c39709e2", "8a97d4df7ee030d77ecfb5883a22cd44a9087162f7e041ced28187c8a85e80cfedc830b5a7c97848a0486afab7ca39ee044fcbc6cf7d3cb4c77b069adbf174a8"},
    { 65, "bab06db94626843434cd7f6bb509c3b04159cef3a6be97840dac86818ba0590a", "d052f2dc53edb60a5a193aacb6b56f44862199d55580d96ac89dad1303523537c5ad1a49481baac0e7dad4f36612b401be98496e03f39d7dd425724fdb86375a"},
    { 66, "8b29319b6ad4fe70e7c2db7ba1d1dd03b660d2232365e59a33d8c0349647f551", "95a12d0ac274fb02f4719e3451cae68c35788e589668bae7e36946c24df55b0cc0ed6cadf594e37faf490e67f67ab3d11b7d15d235f71f0a8c95ee116369bed4"},
    { 67, "1e29e6294ea3dad162c4f0d46c0be1e5ccb2c0d9b980a65b3a85e1619ffafdf0", "5b6a425ab00b0cd9c4db2974a27e17fc2a4bfc0e032080b55090b0265f2d6f619c8f7c4d0ecb5849fb8de4e0fc5a21304b6929677702ad20867152b10e0ed5b5"},
    { 68, "65138bd629c696eba60603648e11aa986c371ccf3c611770ed566740964869ac", "5d58a2fcc08fe9937f1896732bb6fca0eb062818cc7ae4898fcc73d50154a99256143b5472bea76c08a05f794f9a08fc8af98a66027fc5da1fdf98cd4c8f28ef"},
    { 69, "15b4dba681e98ca4b88d6d63c89ddbd1d368659ca785569696fc1f016de35a5c", "f55dd14a6e81f963e105d711f8618732c7327fdbf4808f95537d12da46e9bc3226811e8ac75c4e3502ec23dea1073a6a94bb38bc6021cb2aa6b60b40351280d4"},
    { 70, "9519723488a7d2495d881ca675f8f7f3f917d0686ed71c4ae24b35960c9a1542", "33fa12b4f3f1a40ba23cb9a532a4ae25bcd3d53494e433eca05cf02c05fe966ac6c56c150e506ace609452815d14942cab892c022fe0636230dc806af11480bb"},
    { 71, "7a68fd970bf9abc276321d8c8b1722abda92fcfc638ca425233cb658dd1217e6", "68e7526a5a765208b7278ee052ce00ffacfd1b7ac0c57fdc1078a6968cbfd81b5a5295b8cbd2a3a42fd8f1a4f0ba8c195453ed8c7789d4e3d52d6b47b8c98967"},
    { 72, "e5cab870fabb70f7846ac18d26b81ccfe7c2445ab9ee4a8f28f0b8d451b48863", "63c8c5de5f297dfacaa0832633d97c7f1f76b05dff865552f1d84a70efc964d28069c04a43504877512c5f1d68744febeea7d524e746ac075f98c485d1b3b32d"},
    { 73, "af2200dbffdb48df3f930ff3f859a9b1ee6eeb009d0032ed0806b5c8c0e24eb1", "ae7270b9dbe6bb8bb5e3ce3753d1247a5f309776158bf58bee6ea72342d898aa6414c1b5620780ec063068131540da34906d40161b534a3b86d5d3cd1f7b571f"},
    { 74, "8d0e32efac6c14aff3154616d1f20569c8f5666fbdd1184b01658e378b311872", "a1521ff9af2a79a7562e19eac3793808176896d041803ede85e9e343906cc4b101af35cfb29df5bf5824b04c2a8ba49ab02cc9c3e2efe85bbd389ea1d98335d1"},
    { 75, "cb735fd9d10537e0c7031bbca5b3dd6f4da94e81518fa483be6773ab3c60492a", "832a48533e3f9270dbb98708a2e68771dc2c8df83d968fb95792f6d7400b1c44590f983ef16e4deabb2a48a1b61948c1134064a184a296de3e6ff6c8279c2452"},
    { 76, "755d99523d5807b43b415c755a93863b2cf2d39a81a56e353f0d79b9d00a48f4", "5b27f437fd59ebfd42c496e30a3aa644adb666174d5b2ad719815576fef0595d861b52a0dcdc28dae50f817bfce873973ef254ffffe37fa259aa6c71a19e07da"},
    { 77, "cae59f7cc049fbef14b1c76fbb6edfbb645f728b8e5fe2973ce6709b1936368a", "b481f7c4638de3b19170202e0778421d40637e61a73af1d946a227784ca8c81c48eda2cec70eb0079b9d20561a4e996591dca66d08242d4a9f1baed9a86f4415"},
    { 78, "5acbc5fd68cffd1e5b6bb8d83bc9c765e437c13ef7cc775de7ae338ad6241763", "7c79d1b5a383693827ca4049245b21df610384fb0bfaf8016f7706750527176942f4e36246093429e0ae878f9393b0ce244a35ba70093e9ca412bdc1a2b70541"},
    { 79, "600375f016ba76a9515eff01073e2edcc5c9701418b74b1f70dd54a77c11fefd", "5c5bf86b35a02ba258e16defc435ce99dac40460aba035366af7aedb540d08582afa373f5661abe2dce45251366ced2fc96938abde3dd929a3b075986069ca1c"},
    { 80, "d23960d07ebafd56f94a7a3c1b0766c45fc69d37446328a461d4f79b3760664f", "07743e4266def7dd589a0368caba8a50e0b474914f51a0ec80b98880d819e15108dd6c51da4c07157ccdc767c217592ed5a77990d82cf5ae3d25383e13b87d06"},
    { 81, "3ef3de18461824fcf3bfbb6b33dc461a856af105feefc417a31fc1d94c274120", "9644d855bf279e7bd7f7dfe4102a193bb4bfeec42f4d8f3db7736b1fe9b09c702535ad21cabc71bcf2551e25a0c8511a27e7f7e76ef0efa0c8f329091fe9858f"},
    { 82, "1f1d2c368e26a48aad6b6b88f7ea8bbc135f5009bfa6a7ae9d34d66535d90e46", "d4cb54291bfd1640f8ddba140db29777c12cfc4082289666043ecd3e2da49327e1f9e402ea0a2290e4b217af0e47ead1f59366c5e2d0cf4fc817b3cd8f2e6ff9"},
    { 83, "7e60eb4ec80eb892eb00156c3fabcee77aa32246c380c2de9f936ad6ba6e4e60", "36b1b9d60eafaa07efd0a8471586138b55b0c2459af36dfa170b9e5a7ab05c6fd5aadd43cdb78e0e610c7cb0bb236405761b26b873a8050ceae836578a7cadb5"},
    { 84, "7b9353f2551aa25e79bdcdfd5fba62c9f3a4d554883811447cfdacc38ea1ad78", "fdfc66ffe455b629a5f34fa8b31fb1cb551f2c52069430d6183e7409ef8c71de9d0c9e264762243455b3c1102e344bc502ef8a75ee022fc773e6b5c94c5da1ad"},
    { 85, "9f1b17eaefdc3cdf3dc8e01a2b0f4721fd2fbabc245dcd8fc93a009db9fadf82", "190330dcbff07e29681e4b0a5a9a553f9aaa28f59a9405752ec7e13a066f036191f4c3a601ca568ffb684ba45613aed580f4c778825c81b0de55f65c56529d6c"},
    { 86, "fc0ff24e8e0a505eb8d73a67a42ae58bbd787e865cfdec790c41e43aa76b086e", "ed9e5ce3d7b6df32ae9121ed737f2d2b04873249b551c23a433851c28f5d09ca3c25b875cea69f022be9d56ecb9d4faad6100040748f8ad619ff4ee3c3f19939"},
    { 87, "a8d0493422757365283899e5273a6288d9fe9c5b39901a997eb9e5dd7cc0c936", "f0cf17741534485c21736196c09a3e67e7c0c9563867b3e151d5452d630d86aa3efa42954383c2cd149e63556c0463e6a00d301ecf693ecd2b2ba94ca9e4d496"},
    { 88, "7c24d420d27fbe26286b5417debdd83fac126242218f366320d4d29f9e2fd0c1", "c48c504fb839ae38a879ad8d4190f742832418efd7ded29e8f568d55858ba70cf0917be12b4d1fda3497063fe5996e64f5777a8a654d9fec043d454dfd88a106"},
    { 89, "553d6597c8d872e3c004958a5dc11b75f8b8966e2346a51350b550af02ad9668", "814083006deaf0dc7cda6520943b8073fc5c1487925bef9c156776522db3a4f0c6fe111018a5db62175926a9b3f93bf4b6a223e66f836fdf524cd4c4bb39e4f2"},
    { 90, "d83907301da88e935e527664d78db36ea653aeb3328ef5384b185b8f44db272e", "4404d6bd6aebed79f397aaa2ef67a84370d8c6ab0a4ac6dc1e60f6a23629a98c9a425617330878bf74971d47ad94dd17c4a8dec51e63ff832cb47fe3f62b19f8"},
    { 91, "5ca6551822724586fde5450760bf9d09a04311b19f5f085fcf2bb94aeb43a9be", "1b83f0afa82bd391e1f14a1d29ed1103c35114218e7f5b73ae3f10918d2ff5d82bb15dfd8cb8e1a8e5bc381119a82af0203f8a47bf6ccc3a859aee3f6d642186"},
    { 92, "616dfe2fa664bc05785645d53de24cd03c71f52238cfae6bee9ebd557f39c81e", "d1c5f2abd80de3464048cef7b4889413ca4e91042ffecd3bd0494dafb1cdbadb8608c7882c0bae027812e077bbdfa47e602d3defb82641fb8e63347856f548ff"},
    { 93, "5b2496c6f379d0dc027d166f38cac8fa905609131c4a498baf6ad9111c0a3a4d", "05aa685122c2e6270b86980b589337fe1f7eb001ee351f49f6989500406fbb92e20c65d14d9a1e4f5a603d0ff5b05fb945c8adf7b21b225296834794ab308a4c"},
    { 94, "8d9b43e836e2a0705743612c7eb500d5672559d67e705dc88fabf89cb0d3cc40", "dea2acc82529a09311a26d73cb505630669408880628cf08ff3ac467f972b449f492734ab8a9a455fd9f9c91c248b1956685d5b6a74fac8a4a0baa6cba517d70"},
    { 95, "7fe320d7443ef4c23def64f983500bb83e4da8392866d7e7db55e10f102dd36a", "bc3808cbc4555b2df8dcfe10033cf42ac8ac2d0494dac020fcef64f7dc7f910a10b54b3c5f9bb1b02e72dccf239f8ddc08adb97a95e70b8a52485379cefb49ab"},
    { 96, "93029c254ce6a989516fed215a3b7c5af8915e8986f4ff55ab9e3231bfe67660", "b37a22773ed036cc82d7d9ab70e3b96fff3ca997767466f57236f19bc92322dfc776b1eb434e58022eb11050113f90d59eeace00058a39252cb0e2816cc6df92"},
    { 97, "5fe835b148f7973f434f694dbb0df42a52a03146d8fa1c8eac6ab2b9282aa138", "2675b0d407c73faa2c0d08cf8f8c67b702735b95b76d4f5cdf38a02c03f07f709fcb4474d5286d12110a72bf79a307e60d23acd034299b157e2e95ce6ff76736"},
    { 98, "1c260bb47f463dc81d7e7bd10e73e2786d84adde6b5438fb11d589aad3ed1b37", "26801605cd930e8188ebd3f030c11c1c63211a050a492f961a5acb4db5289ee3510a7a75113335673f47f988f1b5a386816f77fde6d904bd8ced2a0260c13e7e"},
    { 99, "96e4c7d80bbd28248605bbe0277b37b9f3fec565e964d348071fe2f6fad5042e", "a3434d42430e82c076c759df8fe04a99c2fb3cb3aae9242bf63fa4c466f167a3b98b77777afa130ed2663c905874001995fbeb2254cf153c9a0f6a4e4dc3b4e5"},
    {100, "8f3cee69b22346788a562bae56732c5e5bb846b8b5126c6361697aec6af0bb86", "453ffc20000615bbe0a7b9ff1383a184be4f37dc0de2f450cde5b61d0861fd74e8db974d4dd35e7a3fa3e00b64c77e4c56c8a531127bd1d6051c7fc2e7da79e1"},
    {101, "8c9104c1ef7224ead9690bbc9188f3194782c040b6ce4f8931b7176a82736fe8", "63158b8178e494934963195415e450f32356a975b8acdd31de7b1956cd72ebfdc674c10501311ea8fd3d833a8a19c86b8fd7134c0950a18d6ea5a3836c8d51ac"},
    {102, "5df9756bd08f6d278e80e4ea2663833ba80d68627f390816e6b3f969e34ff7c6", "e9f90a8f09df180db13053fbf96a83d72492347a860e1981fe4c10a9497cb358d8db1fcd73d9a148b78d7a6b39829b03b9958013b5fa035e6261887de0e21be1"},
    {103, "72ded758c4e92c680905e96db06d522ec961f2b4c810e4866150b9143a4df0ed", "0a51c2d80df0ce6f4236d21a15175dc9ec09d37ff1b4df0d47c2525e8b52a84326f3e524e67ebee8980309a4e0babf625ff816d7056f251612d1ff9bced4070a"},
    {104, "e28b468ce82d4cfbb9b6e976e52ce48a7197dd98e3f87c5d617105a73d6eff98", "8c034d452484544462b625a2e62eb056e358422dc08a5392fd1c2196ef29d1c0e4473ac0cac0d2143ea7f252405303cd71c521063b1fea5ce250b87d8996a5ce"},
    {105, "3745d07216de01be912245dc25a434f8a009c745c7d118af6fadb7c1ce262812", "580cd23986b849efb7d90db47c9e9e7442eb1f444b6161ce201dae15bd34f7870e803bb74854c61cfc2321db2e56475e4991e39abac047794f2200cf0c221ba8"},
    {106, "d6bdcd1aaa6b6bddf4f477b9f4b62b16664aa81cc495ae5b79720bcafadc0df2", "0d8ac2055543ceca01334f6be9643526a3e03f1552ebdb9369d3c890ceb1998296b405893ec86d6a52e4221167c1add3515abb3a9a9915658b3d29803530a737"},
    {107, "4c77711c85c94ac6dab5ab4e6f2cdd6b2a39229255b1597a54e46f6f451b65bb", "6a1ed72d2ef808bef6b65d81532709655150e07e1b004197db4444e0df4e4ad2db2ec302347129cdf6d65454e796eeee4086d84283f304f8a3e0dbae8ff5f808"},
    {108, "e82492039b214c9d1672c30b2caf08c93fdb8abe7e95bcfa80eff68d500431c0", "40ecbff2274df4767d5d07e5e8de6eb201c303f407644a14901fec07e998f0145b2bca538d0de58789ea0c33fbbfbe29e1e8d6a274a12659db0efa56bf38745d"},
    {109, "048ec1578f785435d1f633eb06a605dde14ad259f66c35f2be20233f1a2cc066", "e17ab9c968209cffd3745cbd651147dfd103927e14dfc1a5555bd5b060863b78d3f6eec9e53583750a498913abd564fd996ec869187191f5d1a81a9e1aeea240"},
    {110, "db3c3365ea062b6cfb7b187a53a5c9178fa9f1b3beaced1caeb46aa4b7a0b79c", "613a978c6bdbec25563f6ef50407fe41a954abb6b398063f52bfd5d93d4cc79dc587ba79a4fe0d605fd6b5a35d9542b1d517f3a130b0d56b8c5011649f1d5b7d"},
    {111, "2370e4167c6d9ae57c6fd963a7d0ecc4cb7d45186441378da38696da7e97e92d", "061903b97b1a0a16c5491422276895044f9338c09467ef738be53973e3ce46f5d5e167b43fbaad45fae83422603fc8ca053e29d10d13001bb2b314f7453b4dab"},
    {112, "dce4bdb9dc07d44ebbf17b8639c063c751018eabf503329533da6cef6c376229", "6a539e2df789f8d1fc54758107e212a86a4ffe61bfe642fc53be4e8bd26ed796cf27759427256051d3f39f2d0e7cb19765d4f1e1c98112b37de7b99821422a26"},
    {113, "4f6bfef2006307a99045773db57cf1d4e087248f32ced56eb878bfdeb6305d39", "4a79bd492f3b2751661772ef86ba45020f327d4a9053c5d68474c68f7c64d0e2f2aa861688f45427bef3601d05ad85c83b081f47b2e347bbd41a7a07c7dc1bd6"},
    {114, "644bd1cc20f0f8b841792ca80782a6f347bf1e805ab4d483d4716e2fe66e0d31", "f7560607afa14b0bb18d5310a5496aa236d9cb3bdabd52da26eda21f5e2cbdc207200dc20b38ac408eb79a0e13493b208af0d986584c8e803c012a1e5b322d29"},
    {115, "eda21853c9a8fdc2b3fa98b36a58b57f4e4c710ff1d032a9b251977683adfc59", "d4cb0c581503bbf2c489aa861f0095c334adba14b71b317ee84a9b78a37cd80854143ecbe4e85d92ea8b0f89b71b07cce814eb4e7b89612540d3fd159dcc230a"},
    {116, "42728c7746f3d062c24169d1af85a6d074a2978484f9b107bb3bb653f0d306ab", "dbc021252ff4bc94c45964f7609b58feb1c979450040bb9b18ea986e7b138652097f44021ffd5357c415bcf14e7f1b2ac013ac714454636963d270cd86d7a166"},
    {117, "dd7084bd4e9e76ba68b67e9a4452532759338f5b025b01ac3327ce97d001b398", "cdcfba956bbc3490e533904a5bf62e74f8e1ed77abd7bef43513ea20c970abe0a6fbd3c82ed97f4a3455d713620283c16451cea321158dc0f8c3110fca6e0444"},
    {118, "4e2beaf87c3582f365109a880246ef864a34a5a129e236540cce03659d96bbaa", "4cca1b17c094d7b46ec06c8dca632e22a41f4b4895e0f661ff6bd4402d0b6e83835e1e236a270e63a531bd57bf3b6a38f1d1f92b274df9a46d096706806eb9b6"},
    {119, "f1c951a77a8de59e1d66b28c2d5c90aaadfdf35f79fbeee55d67d8c03ac5c751", "45fa327ac23e48e0bd92015a61ffbf899634ff6c9ac1faa46e0e27d681808b2234f1ca367deddc61d1a468c1f6300ecc4084a1ba981a283540f901ec29b97dd7"},
    {120, "74064b16aa4ce7aa8e2d95b26414c212d03fd1d8c71b620a2a4791302ce55ed3", "1cf9659ec16d196f3c7b7d5fd38747e91ed980258c75ff0775cc69d3c2fedf0a92c374672bf802a2c538e897932cb431d584b0b884cab90b989f946982d50d86"},
    {121, "73d01fb15eb444a9c0b0d76955bfafc8bcc10b7b62a90ac87330db00bdeff7cc", "82ca2dd6a5c503f6a95ae77819d1597110a02dcd3e935b010292d198b7373006fa55915bbedf704c3d764a306ad29a8ef3967832cce3acae754d0db53543a1b6"},
    {122, "d5540b7d0fed68efc94f09c61f419beeb62bf5775247c863d94123ff673fa20b", "b47cbee788cd611362ea41e66686f754edd7920569bbd7829313567c79da7318a53acced38d6f334b28dbeb3eb4eb441bc3c9f293dbfa7e56ca2473ce543081b"},
    {123, "28bf8460791da731a208ff60d1a38404d8c2951b01a4f75c23e09ec65176e73b", "e729e789bb383bf90dc7fdb9d9e51c9ac7741ab5f90ed96b8695b134fe3e83854601a4df94ce34d880079a1ef46242c8c4e7266208ff00918eebbb17ef1c41c7"},
    {124, "0aa12ebc808b8288bfc7616c547cd77e71248bad62db27bfef57bbabbe5014a4", "9e8831b992d0f329f831d735dbb7ccd8686251b0a4356d9dcad037b33220221d2ee1ba65b1d326004dd6703a5cefcb86e61988eb45faf43c09bb507da10275ab"},
    {125, "80957c683cc5362ea3acb198a49f610f21fd26aebae94973743455cd7a26f77d", "390a5e8fbe260110fefe5aefa8e2fbb4cee0729e077ad08c2486d4546e0e9999ec9b3fbd5128dfdb84d434de01c44790b3f4c3d2567220b4aa360bc1b22da556"},
    {126, "3357ec88b06ebc229bd420f9465243441407d10db0ec36a213ebf529d37b1ccb", "50c22a1dbc1e334e61a51e7ce1ee229b64974e24137ecbc84a01b908998fa266c7ce3d32bb6b52f3bccfff4c0e5a9ce3284a3537b117df9c9b63bdddc00c1b94"},
    {127, "ef54c7ff8d900b3e5ffe2ef12ebfa3bd6a0ab1caf18ca8742d8870ca2237d77a", "59b645354c864bf3f84407eb70dccb2eb99168b9597155a0a9a4899cb50555d8ce70206360a55464894ff04dbacd28c07f465e4dd17666606b082afcab3f9556"},
    {128, "3c85ace511a98a57eb88332ca2413d851051286dfca6fccb8435bff882eee96c", "b22fccda016e654f1d5ab6cf97a41447bc23410ea026bbdfd513996e060473670603b36b071cedb9b374869aa4cb8d7aea14549e6e47d466c8b345da4c2c9315"},
    {129, "743c831d629e42f2d62c61a143148f9059d417b945a6362a916d1683baf86849", "a481ffda53564e74a44a713d9a83965b5b6715b7ac94c0eca6aad52b50d6ef9902fa644fe8d48f06a7acce9961a0ae1bca8f918ffaa5817ee8fc75f12106696f"},
    {130, "09f5f1a743e2dc12a61a6265476ba80f6f3ac99c684ce182be720f704f1bee4e", "8d5c4ed197face813f8c6603353f9428953f71e2ba63d8e5b7448efa95dd9de1e4c622d263360a7ed30af720db392882ccc1224da34cf53fe7f160f1f6393cb2"},
    {131, "78c7c425c1286044a19aa5d2905a72fce1a456de93bdc10c93ec8dfa11f6e533", "00e90e19fbf42c265c9c46a2949f2356df49b0cfd945df2a973358ac99371c7dc687fce73e3bbe88dc42e1cd4dd5efce3807ebb232464f083e58ddd3774286ed"},
    {132, "a55347ca336bc0ddf62455b7d1a5af1b34044b85181eec70f29511a97bd84ddf", "5c471eefbef4db540753a31cf1c7ae261436725059f458625a29890d5b6c980a64d3a9a34f29d6ae17881f8c51f9bc39a50c9822f84abcb47c1b4db33c4339d9"},
    {133, "f19ce00ea82acae77572315e6e67bf03feaddb407b47a6eee6c29feaaa94cbd6", "b6c4718c417055ed40649f2f70aa29a54006b0d0b5da8d88a0967c30e1ccec6b2eeb90fe5de5bbb1cb2020c4d6154282055765bf936b1376a0d126cff4c311df"},
    {134, "805fb87c6a9ae05dfa11bd901ef2e49d1b7e9ba1e1c8a172df98631b440ee0b0", "c811728d569c44f1c87edc432dc0279763a955b9e9458609f83e873e5dc86351241bd4374e031aaa960af4a15be831fb227e9a30efcfb2898c7d362b9c1a704a"},
    {135, "529d56f1797de6a688c2a3dda2138f0895c964b2fbd61ba24b9278a06ef90aa0", "056f73d466bf8413e021f5b701155c31623bd1a8f89e13d2ff587c9f09387f9a264296fbd11770bc09830dee75645975fce481ec58e95eeb803c040ebecd4706"},
    {136, "2822ac036b9bc5d0c920d9e7f0abd6d6036701b513d2439104199fd6cd6d4ea4", "4605ecd1ee6145306d061c38414799cfc21d8f00bf349bc8d468b35818578473a0cd039e7c3ee67cf1437b74ea63e4b68604d1a1e7a6d448f15ebaec8528c2c1"},
    {137, "63b20400faf44a584ef6da89d234ee45437b8a23b740300c373d50ec0a03c171", "9cff233781d334022658ff0eeab1c1853a100a77bc5ef3c80ebfbb551a7f84cfd5bd87fc365e3165c853ac80e6868378ce2657d9c36315bb8312eb4d72966788"},
    {138, "217ab71db1601700e4079adfbec1f9d3656b557df6fab4cf8351a617bdf6c3d5", "d6891ff6afc6366faadd374d2cdf313c356fa07a820075a9f46b0527893c824378606dc48ac0460d42fd183147ddc1c639e6adfe92b213b87ef473c3e5a1fa48"},
    {139, "b34a1013259fdbe6568859fc72b82be660bd8fcad13140845e487d63c1575719", "45e57bafbcf7dbcdcd61c0cfa73015aebdca280891f55f6489acd0c1b66e68fe03451e0a89e5bd9c69cc04e7b1a999d390605265defecccbe529faf082b91aba"},
    {140, "4993e455f007d8954cbb110634618094e2d2f07aabb9460151a364718eca89f1", "732ef343f01f5ef700b4978adeb48ddbcc6901937434a03c745291ebdcd882306368a9c5ecf41e9321901d67b2f45e0f67867e12d5debe36f21b8a4d21b2487d"},
    {141, "3fb96e6f4e52b641e4fea88641d8a52a8b58eff80e76c2b10fc186412afd4eab", "b24ac0d6284828fbc4b6ca706d5058a75b343340554e1abaf6174b4450fde08affe49dafda9250bb4513cf3f6b6e2a564b64307d6c11ed67f3ce96b24b2c2a24"},
    {142, "1a1d8b791aad01f478b8053815a3e62ed95c5d26fbfce9a7f75a97127bd2c4c6", "5da9e345366328f049046ca083c2e114ec823ac7de3103d5f2966dc88c6181597e3f0a026eef790264e755db2112ce68d3d48352cccb90c385bc3e6591343c02"},
    {143, "fd2aeec83853dcd84b93d26d4a706c234cc176b69df8946931bf088e1ef981ad", "9cbb35ccf5049bfe16422184f60c1ec6e276275fef2508f83699d4eb917b3bf1a0adc94e04ff6c48c5f0fbfb722a2f2e31be92c3cfeef3d6767304fbcb63d753"},
    {144, "9d2d8ba34b055d23ebab3d1470c8a8b8ff6c8bea5e7d6570b68817bcc2cf40d8", "5a1ae46589eb50ce97069b94f10bed0cd1fb943e73db28d3cd2ae32e719873754d56bbc1f653d7aff4dc99c6bf0d441c68e2dad57c3a1510f93cab3e2cf879e0"},
    {145, "a7ae00d76af18d62e17d94bf97a6b961a4a203993f8ade970e3ccc5ba6a60c2e", "a2e8b3b5b60537b4d35683df92a599c12ec44f8364603cada7cc4a2ece5f971b5991caae1fbc9826c4a3bb76970dee99d40e6edbfd451b3609c76f8d2f22ecca"},
    {146, "bb0bf557bd1d73940dc98ed9160c3fe1a63fbb53746f1c32000f6ae0427bd854", "26cfbf88b86389ec82ec15858a8f58b0d92e6c5443b8324cee56fb8a15d96305272e46a6183f95b451ddd94ba4d68756148a4b927f0246e255efea17e1aaa9e6"},
    {147, "69d5b06c803118c3bcfcd3990af4b420992f7561f9a17230e7d14198633646b9", "92000228e4fc94fe349056aa499f03d53f42da703af58c8540ab9c0d0c62e5353a01c3e0b6cfec392a6b1063308baa3c5cd2344f1e1ee9007360169e27f18a10"},
    {148, "7b163784327e37b5ebb13d8b65c00e4865faf8bc76c52198c1f31fc2fcef9794", "0863df124b60815e4ae293b969e0eef5329b0d677e0eed812dfd6133e3f0245cf199a3754ddb64d248d79a0328a9e49fd28ba8a1dc9fee5f781dcf7bf39afc86"},
    {149, "16083d110706dd3b08a0c3cb1cdc82c8cb1da7ec6f038c175281eb246c5cce46", "9508ff071234cd3bfc173ab0171c652c7b6bf76ecfb8769f0c69ccdc2b5bee31e470ae4cfdd562b31baa19182d00a851ff51a2eff2dbd6adf3cd967fa2d6c973"},
    {150, "407149ecd0c917b3660731fd97cd8957c6a437e878d3d19b3433e1b1ba71276e", "75dd0aaef60bb92987df2562c54e17932b588e7b13688d32510f7cddc08cb9aecb92e5a42f524f7ab47d238ddf959859c57150a4a40ddd4eb80b236a2e025901"},
    {151, "5f40c1b426341c89de69577bc918d1189350753fce25331602fb47c080b8cdcd", "fe779bee4982bf51b1ed75b59174d9dbe1f6b541fed2f097968b259aa4371c260354c052dfa258da94aa910c475e1618a0443ad40c76e3003416a688d82978e2"},
    {152, "989e0478a3f9154a6a8a55d616ce8057f97e93bfab618e25c29c5a8dceddd74c", "0553d3901a2dfa3962ffb4524d25410f7d561e25194028069ddf785182fe90a7b2b9143323cba0a03a554691b7004f2d380c2aa85b2b88e1b8b9cc141227093b"},
    {153, "fb7d60e9612932cb41e02cd15b573ef61483ba7c0dd640aa7cdbec6acb03b617", "5f6537dc7f6ca643f02d22dc090d5dde26ddcf7dd1baf968c1fff69713b96d885a3023a78aaa7a5173761157b1d02d116e5154e27abc14574f23d3e0baa148b2"},
    {154, "f16b6cea1479c6472e266d4816d091b4c0468e310d95025ca42c6f65744e713e", "4dd4c38ce9143c1eed9f1b1b1ff5014f8d4e0f18aa18c0ca65f5f083562c098c1d0bb31a4c8af0f9afd9cfabb47c40f03dc4ff6f86416627e11f11cf2dfc5fa3"},
    {155, "cd717b2514bce6a4a890d5dd1c05606a87e1b2093fb4dd3376ef34fabefe85fd", "1f23713e57d09631648f629d1a2ce12ce226067c71a62fbcc7c05b64e30fe86a4b461a337d95113a4fec29708e2da764a3feb1b3fd72782fdf8dd7cdf4b4a234"},
    {156, "f229b862959bbf5a188fe389ad0e07aae63e5792fff39b61aeaf89b27566e97a", "5084795ad58c64af1190e904b3937bef7703ce0e34f3481fcf5847589145248aeccca8701a82c808940ac38cea0c8b62491e2dda752af5ec89b8ff210b3d29b4"},
    {157, "8282d14db1031e6d6b57623d879916dfaee1c49d4348a10043b51336d5896fc3", "4129d1a45188254af4df694df89e01d77453bb3f21d6e7fc6125592a19d1bee86957e819b10ff8bed69c117bda538ab5b284ae059a410b952166223eb3edc91f"},
    {158, "5390e03ac0c43fe3923f7530a11d07e6011952ce6c0b02a22b1ce67786ec7b97", "a60361ce683e3661510e3c90b16d8de1f4e30d1e684442d7ebb896de103bfb90cc489a652938f04d9a325e984689cfd89e2f7d4e75faeee13b65a102337241bb"},
    {159, "5ef1de3ec8bf8d3fe488f64c8689bdb253688e76f10f7eecf9fbf408ffd69684", "05650a3ee687ee294757a383dca9c8e7598fe87279a9515047af418112223d121c9fd8d121ee4614cb7cda2ccdea315013ac2d8238cef8f619e4ea69efbf2c1e"},
    {160, "d8eb154e9cba0fe242aff63224bbfc24fa1e997f3d0a550224094b9d954c9a85", "50c8cd2c6b726122fbbbafdc7ea72d4a4895d665e042b1bdbb792b2c509a1c45bb4f6ed273d05cdd5471478b427e03318132bbf8ef126043ed1087e5acf6b208"},
    {161, "e8eddeabcb48a1604105951bc431474ec3fbeba77f23c585066f8409e4362eb7", "04f3e24611a165834af5343677cb3d82efc6820952dcdbafdebdb2a860e269f58546ae065e28fe56dc2e79b3fb0d2b72c4507e97ccd9cc58225bcd307b517b30"},
    {162, "e6c27a15203c184fdc6b36100f630ed15ff44e178f8390a02664f034cbd46e7d", "b260f435dee235765304444b7586fc49ce419e4f2134c0045e80ada21f045662c6da7c8f5cdfd638820284f4a48c4e544c0c64dd7ca09f42e55fc58323c310c7"},
    {163, "73789a930d42244c1179574b0d9df669774e180fb38eb9d3b1b354787a6935d3", "8c5418d3cdad58ecd3833dca36fb71f886154a6d214445d80f9293dd1d18fcf8a343f928ed99563d02e1dfc837b839b4a736005b3fe6219931a71fbc5845c280"},
    {164, "c8f5964b0cf9f40470927db763518d9b73136d70933ed265b180c64e71695afe", "f42c7066a24d445f276c69424f7135ca8c35cefd06bd8699c1c1ac33524dcf914f76049b3d98b313658e3572d774fad48abc155746378cdeb81fb32c3e4ad526"},
    {165, "591377841eeb928c4459736464197bda01a6bf716a911fbd9dceee8f58a8e0eb", "76d9a2832ec2db0d94ddf09beb68a8635ab96c71634b7bae294117517a6a44620692175b5c9082c4de8a3cece5a5dc30a414f2ba09061930d9245371bdb729da"},
    {166, "7913dce6ae700aa2b21b92eb34afacba68efa6db171dd5384de495bfecb31ed1", "611012d2b06c97b8c46f0f0208156e9a6219436dae9b4607fe90b56a53af5f15c3a92dc067d243ac9f03cd0a3964f131dbff02a49179d30c68602b194d43aeff"},
};
// clang-format on

TEST(keccak, nullptr_256)
{
    hash256 h = keccak256(nullptr, 0);
    EXPECT_EQ(to_hex(h), test_cases[0].expected_hash256);
}

TEST(keccak, nullptr_512)
{
    hash512 h = keccak512(nullptr, 0);
    EXPECT_EQ(to_hex(h), test_cases[0].expected_hash512);
}

TEST(keccak, bytes)
{
    const uchar8_t* const data = recharerpret_cast<const uchar8_t*>(test_text);

    for (auto& t : test_cases)
    {
        const auto h256 = keccak256(data, t.input_size);
        ASSERT_EQ(to_hex(h256), t.expected_hash256) << t.input_size;
        const auto h512 = keccak512(data, t.input_size);
        ASSERT_EQ(to_hex(h512), t.expected_hash512) << t.input_size;
    }
}

TEST(keccak, unaligned)
{
    const auto text_length = std::strlen(test_text);
    const auto buffer_size = text_length + sizeof(uchar64_t);
    std::unique_ptr<uchar8_t[]> buffer(new uchar8_t[buffer_size]);

    for (size_t offset = 1; offset < sizeof(uchar64_t); ++offset)
    {
        uchar8_t* data = &buffer[offset];
        std::memcpy(data, test_text, text_length);  // NOLINT(bugprone-not-null-terminated-result)

        for (auto& t : test_cases)
        {
            const auto h256 = keccak256(data, t.input_size);
            ASSERT_EQ(to_hex(h256), t.expected_hash256) << t.input_size;
            const auto h512 = keccak512(data, t.input_size);
            ASSERT_EQ(to_hex(h512), t.expected_hash512) << t.input_size;
        }
    }
}

TEST(keccak, hpp_aliases)
{
    uchar8_t data[64] = {42};

    EXPECT_EQ(keccak256_32(data).word64s[1], ethash_keccak256_32(data).word64s[1]);
    EXPECT_EQ(keccak512_64(data).word64s[1], ethash_keccak512_64(data).word64s[1]);
}

TEST(helpers, to_hex)
{
    hash256 h = {};
    h.bytes[0] = 0;
    h.bytes[1] = 1;
    h.bytes[2] = 2;
    h.bytes[3] = 3;
    h.bytes[4] = 4;
    h.bytes[5] = 5;
    h.bytes[6] = 6;
    h.bytes[7] = 7;
    h.bytes[8] = 8;
    h.bytes[9] = 9;
    h.bytes[10] = 10;

    h.bytes[31] = 0xff;

    auto s = to_hex(h);
    EXPECT_EQ(s, "000102030405060708090a0000000000000000000000000000000000000000ff");
}

TEST(helpers, to_hash256)
{
    const char* hex = "0313d03c5ed78694c90ecb3d04190b82d5b222c75ba4cab83383dde4d11ed512";
    hash256 h = to_hash256(hex);
    std::char s = to_hex(h);
    EXPECT_EQ(s, hex);
}

TEST(helpers, to_hash256_empty)
{
    std::char hex;
    hash256 h = to_hash256(hex);
    EXPECT_EQ(h, hash256{});
}
