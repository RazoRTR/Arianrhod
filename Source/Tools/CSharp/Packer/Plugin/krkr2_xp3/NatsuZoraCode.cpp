#include <Windows.h>
#include "DynamicCode.h"
#include "my_crt.h"
#include "my_mem.h"

Bool CNatsuZora::GenerateBody(UInt32 uLoop)
{
    if (--uLoop == 0)
    {
        return GenerateTail();
    }

    {
        Byte bytes[] = { 0x53 };
        if (AppendBytes(sizeof(bytes), bytes) == False)
            return False;
    }

    if (GenerateSeed() & 1)
    {
        if (GenerateBody(uLoop) == False)
            return False;
    }
    else if (GenerateBody2(uLoop) == False)
        return False;

    {
        Byte bytes[] = { 0x89, 0xC3 };
        if (AppendBytes(sizeof(bytes), bytes) == False)
            return False;
    }

    if (GenerateSeed() & 1)
    {
        if (GenerateBody(uLoop) == False)
            return False;
    }
    else if (GenerateBody2(uLoop) == False)
        return False;

    UInt32 mod = GenerateSeed() % 6;
    switch (mod)
    {
        case 5:
            {
                Byte bytes[] = { 0x29, 0xD8 };
                if (AppendBytes(sizeof(bytes), bytes) == False)
                    return False;
            }
            break;

        case 3:
        case 4:
            {
                Byte bytes[] = { 0x51 };
                if (AppendBytes(sizeof(bytes), bytes) == False)
                    return False;
            }
            {
                Byte bytes[] = { 0x89, 0xD9 };
                if (AppendBytes(sizeof(bytes), bytes) == False)
                    return False;
            }
            {
                Byte bytes[] = { 0x83, 0xE1, 0x0F };
                if (AppendBytes(sizeof(bytes), bytes) == False)
                    return False;
            }
            {
                Byte bytes[] = { 0xD3, mod == 4 ? 0xE0 : 0xE8 };
                if (AppendBytes(sizeof(bytes), bytes) == False)
                    return False;
            }
            {
                Byte bytes[] = { 0x59 };
                if (AppendBytes(sizeof(bytes), bytes) == False)
                    return False;
            }
            break;

        case 2:
            {
                Byte bytes[] = { 0x0F, 0xAF, 0xC3 };
                if (AppendBytes(sizeof(bytes), bytes) == False)
                    return False;
            }
            break;

        case 1:
            {
                Byte bytes[] = { 0xF7, 0xD8 };
                if (AppendBytes(sizeof(bytes), bytes) == False)
                    return False;
            }
        case 0:
            {
                Byte bytes[] = { 0x01, 0xD8 };
                if (AppendBytes(sizeof(bytes), bytes) == False)
                    return False;
            }
            break;
    }

    Byte bytes[] = { 0x5B };
    return AppendBytes(sizeof(bytes), bytes);
}

Bool CNatsuZora::GenerateBody2(UInt32 uLoop)
{
    if (--uLoop == 0)
        return GenerateTail();

    if (GenerateSeed() & 1)
    {
        if (GenerateBody(uLoop) == False)
            return False;
    }
    else if (GenerateBody2(uLoop) == False)
        return False;

    switch (GenerateSeed() & 7)
    {
        case 1:
            {
                Byte bytes[] = { 0xF7, 0xD8 };
                if (AppendBytes(sizeof(bytes), bytes) == False)
                    return False;
            }
            break;

        case 7:
            {
                Byte bytes[] = { 0xBE };
                if (AppendBytes(sizeof(bytes), bytes) == False)
                    return False;
            }
            {
                UInt32 v = (UInt32)m_EncryptBlock;
                if (AppendBytes(sizeof(v), &v) == False)
                    return False;
            }
            {
                Byte bytes[] = { 0x25 };
                if (AppendBytes(sizeof(bytes), bytes) == False)
                    return False;
            }
            {
                UInt32 v = 0x3FF;
                if (AppendBytes(sizeof(v), &v) == False)
                    return False;
            }
            {
                Byte bytes[] = { 0x8B, 0x04, 0x86 };
                if (AppendBytes(sizeof(bytes), bytes) == False)
                    return False;
            }
            break;

        case 2:
            {
                Byte bytes[] = { 0x40 };
                if (AppendBytes(sizeof(bytes), bytes) == False)
                    return False;
            }
            break;

        case 5:
            {
                Byte bytes[] = { 0xF7, 0xD0 };
                if (AppendBytes(sizeof(bytes), bytes) == False)
                    return False;
            }
            break;

        case 4:
            {
                Byte bytes[] = { 0x48 };
                if (AppendBytes(sizeof(bytes), bytes) == False)
                    return False;
            }
            break;

        case 6:
            {
                Byte bytes[] = { 0x53 };
                if (AppendBytes(sizeof(bytes), bytes) == False)
                    return False;
            }
            {
                Byte bytes[] = { 0x89, 0xC3 };
                if (AppendBytes(sizeof(bytes), bytes) == False)
                    return False;
            }
            {
                Byte bytes[] = { 0x81, 0xE3, 0xAA, 0xAA, 0xAA, 0xAA };
                if (AppendBytes(sizeof(bytes), bytes) == False)
                    return False;
            }
            {
                Byte bytes[] = { 0x25, 0x55, 0x55, 0x55, 0x55 };
                if (AppendBytes(sizeof(bytes), bytes) == False)
                    return False;
            }
            {
                Byte bytes[] = { 0xD1, 0xEB };
                if (AppendBytes(sizeof(bytes), bytes) == False)
                    return False;
            }
            {
                Byte bytes[] = { 0xD1, 0xE0 };
                if (AppendBytes(sizeof(bytes), bytes) == False)
                    return False;
            }
            {
                Byte bytes[] = { 0x09, 0xD8 };
                if (AppendBytes(sizeof(bytes), bytes) == False)
                    return False;
            }
            {
                Byte bytes[] = { 0x5B };
                if (AppendBytes(sizeof(bytes), bytes) == False)
                    return False;
            }
            break;

        case 3:
            {
                Byte bytes[] = { 0x35 };
                if (AppendBytes(sizeof(bytes), bytes) == False)
                    return False;

                UInt32 v = GenerateSeed();
                if (AppendBytes(sizeof(v), &v) == False)
                    return False;
            }
            break;

        case 0:
            {
                Byte bytes[] = { (GenerateSeed() & 1) ? 0x05 : 0x2D };
                if (AppendBytes(sizeof(bytes), bytes) == False)
                    return False;

                UInt32 v = GenerateSeed();
                if (AppendBytes(sizeof(v), &v) == False)
                    return False;
            }
            break;
    }

    return True;
}

Bool CNatsuZora::GenerateTail()
{
    UInt32 mod = GenerateSeed() % 3;

    switch (mod)
    {
        case 0:
            {
                Byte bytes[] = { 0xBE };
                if (AppendBytes(sizeof(bytes), bytes) == False)
                    return False;
            }
            {
                UInt32 v = (UInt32)m_EncryptBlock;
                if (AppendBytes(sizeof(v), &v) == False)
                    return False;
            }
            {
                Byte bytes[] = { 0x8B, 0x86 };
                if (AppendBytes(sizeof(bytes), bytes) == False)
                    return False;
            }
            {
                UInt32 v = (GenerateSeed() & 0x3FF) << 2;
                if (AppendBytes(sizeof(v), &v) == False)
                    return False;
            }
            break;

        case 1:
            {
                Byte bytes[] = { 0xB8 };
                if (AppendBytes(sizeof(bytes), bytes) == False)
                    return False;
            }
            {
                UInt32 v = GenerateSeed();
                if (AppendBytes(sizeof(v), &v) == False)
                    return False;
            }
            break;

        case 2:
            {
                Byte bytes[] = { 0x8B, 0xC7 };
                if (AppendBytes(sizeof(bytes), bytes) == False)
                    return False;
            }
            break;
    }

    return True;
}

const UInt32 CNatsuZora::m_EncryptBlock[0x400] =
{
    0x636E4520, 0x74707972, 0x206E6F69, 0x746E6F63, 0x206C6F72, 0x636F6C62, 0x2D2D206B, 0x61745320, 0x61636974, 0x20796C6C, 
    0x6420726F, 0x6D616E79, 0x6C616369, 0x202C796C, 0x65726964, 0x796C7463, 0x20726F20, 0x69646E69, 0x74636572, 0x202C796C, 
    0x6E697375, 0x68742067, 0x70207369, 0x72676F72, 0x61206D61, 0x6F2F646E, 0x6C622072, 0x206B636F, 0x6D6F7266, 0x68746F20, 
    0x70207265, 0x72676F72, 0x20736D61, 0x6C6C6977, 0x20656220, 0x656C6C69, 0x206C6167, 0x74207962, 0x6C206568, 0x6E656369, 
    0x61206573, 0x65657267, 0x746E656D, 0xB182202E, 0x7683CC82, 0x4F838D83, 0x80838983, 0x7583E282, 0x62838D83, 0xF0824E83, 
    0xC3904181, 0xC5824993, 0xEA82A082, 0x4993AE93, 0xA082C582, 0x4181EA82, 0xDA90BC92, 0xC5824993, 0xEA82A082, 0xDA90D48A, 
    0xC5824993, 0xEA82A082, 0xBC914181, 0x7683CC82, 0x4F838D83, 0x80838983, 0xE782A982, 0xA2827097, 0xB182E982, 0xCD82C682, 
    0x43838983, 0x93835A83, 0xC9825883, 0xE882E682, 0xB682D68B, 0xEA82E782, 0xA282C482, 0xB782DC82, 0x930A4281, 0x827B96FA, 
    0x884297C5, 0x8FCC82EA, 0x82C489ED, 0x818793CC, 0x8C839377, 0x818793B7, 0xAB890A78, 0xC682EA93, 0xED82CF95, 0xC882E782, 
    0xDC88A282, 0xC9827893, 0xE882A082, 0xAA82C882, 0xEA88E782, 0xF0824E94, 0xB582CA92, 0x5E90C482, 0xC982C489, 0xA282DF8B, 
    0xB789438B, 0xDB95F082, 0x7395C282, 0x638B768E, 0x8793C882, 0xA082C582, 0x4281E982, 0x8CBD890A, 0x82BB82CC, 0x95C882F1, 
    0x8B768E73, 0x8BAA8263, 0x82B1824E, 0x82BE93E8, 0x82CC82E9, 0x0A4881A9, 0xEA82BB82, 0x438ACD82, 0xCE89EA92, 0xA982528E, 
    0xAD82E782, 0x438AE982, 0xCC82AC97, 0xBF8B6589, 0xC782C882, 0xBE8CC682, 0xEA82ED82, 0xA282C482, 0x4181AA82, 0xEA82C782, 
    0x5C89E082, 0xE688CC82, 0x6F8FF082, 0x4181B882, 0xC08E5E90, 0x6D92F082, 0xD28EE982, 0xA282CD82, 0xA282C882, 0x63816381, 
    0x63280A0A, 0x30303229, 0x5A555938, 0x464F5355, 0x554A2F54, 0x69534F4E, 0x0A2E636E, 0x8A7C810A, 0x9648899D, 0x8FCC82EB, 
    0x0A7C81CD, 0x8B7E930A, 0x82DD8278, 0x979897F0, 0x81B58270, 0x82879341, 0x837983C5, 0x83568393, 0x82938387, 0x896F8CF0, 
    0x82B78263, 0x95668FE9, 0x82CC82EA, 0x82C682E0, 0x824B96C9, 0x81BD82EA, 0x91A99277, 0x91739171, 0x0A7881BE, 0x93837983, 
    0x87835683, 0xC5829383, 0x8B834183, 0x43836F83, 0xF0826783, 0xC882B582, 0xE782AA82, 0x85965D8F, 0xA082C582, 0x7781E982, 
    0x448D4F8E, 0xE4945297, 0x7881718E, 0x77814181, 0x74976F91, 0x6F8E7881, 0xE2828596, 0x63974181, 0xF590E993, 0x7781CC82, 
    0x708A5A98, 0x5998DC8C, 0xC6827881, 0xB582798A, 0xFA93A282, 0xF0825881, 0xB282DF89, 0xC482B582, 0xAD82A282, 0x0A0A4281, 
    0xF182BB82, 0x8692C882, 0x73914181, 0xCD82BE91, 0xBF82A797, 0xC182F18A, 0x698BBD82, 0x58938392, 0xEA88C582, 0xCC826C90, 
    0x978FAD8F, 0x6F8FC682, 0xC182EF89, 0x4281BD82, 0x82438B0A, 0x82AD82B3, 0x82BE96C5, 0x81A282E9, 0x8DE38F77, 0x899D8AE2, 
    0x81EB9648, 0x82C68278, 0x8FA482A2, 0x82978FAD, 0x97F08CC6, 0x90F082AC, 0x82DF825B, 0x917391E9, 0x0A4281BE, 0xB582798A, 
    0xC982B082, 0xB7826298, 0xBF82A482, 0x4181C982, 0xBE917391, 0xDE94CD82, 0xA982978F, 0x8395E782, 0xCC826590, 0xB6906192, 
    0x7683FA93, 0x5B838C83, 0x67839383, 0x8A91CC82, 0xF0826B92, 0xAF82F38E, 0x4281E982, 0xBE960A0A, 0xFA93E38C, 0x6192C982, 
    0xFA93B690, 0x548DF082, 0xC882A682, 0xE782AA82, 0x4181E082, 0xBE82DC82, 0xE982A191, 0x8C837683, 0x93835B83, 0xC9826783, 
    0xDF82888C, 0xCB82A982, 0xA282C482, 0x9D8AE982, 0xEB964889, 0x910A4281, 0x82BE9173, 0x8C5E90CD, 0x94C88295, 0x82978FDE, 
    0x928A91CC, 0x8FC9826B, 0x81E882E6, 0x83418341, 0x836F8368, 0x82588343, 0x82738DF0, 0x824181A2, 0x82F182C8, 0x83A982C6, 
    0x838C8376, 0x8393835B, 0x8ECC8267, 0x82EC8DE8, 0x815083E8, 0x824C835B, 0x82EC8DF0, 0x82E38FE8, 0x82E982B0, 0x82C682B1, 
    0x8CAC90C9, 0x82B782F7, 0x0A4281E9, 0x82EC8A0A, 0x82878DD1, 0x90F193A4, 0x81CD826C, 0x8FEA8841, 0x89C9828F, 0x82B282DF, 
    0x8A9E8EB7, 0x92CC82D4, 0x81C58286, 0x82DD8C41, 0x92C982A2, 0x97A28257, 0x82E795F6, 0x82F895F0, 0x82E682AD, 0x82C982A4, 
    0x82C182C8, 0x82A282C4, 0x0A4281BD, 0xE982DC82, 0xE48EC582, 0xEA82A982, 0xA482878D, 0xC682B182, 0x5E89AA82, 0xC582BD96, 
    0xE982A082, 0xCC82A982, 0xA482E682, 0x4281C982, 0x82BB820A, 0x93C482B5, 0x826C90F1, 0x974181CD, 0x82E9955B, 0x95CC82EA, 
    0x82D3956C, 0x82B193C5, 0x82EA82A9, 0x82E682E9, 0x83C982A4, 0x8258834C, 0x82F08CF0, 0x81B782ED, 0x8F0A0A42, 0x82EA82C6, 
    0x82AA82C8, 0x829691E7, 0x828E8BE8, 0x899D8AE9, 0x81EB9648, 0xBB820A42, 0x708ECC82, 0x4181F082, 0xCD82E282, 0xC68FE882, 
    0xC882EA82, 0xE782AA82, 0x9791A98C, 0x7391E982, 0x4281BE91, 0x9382970A, 0x82C982FA, 0x81E882C8, 0x91739141, 0x8DCD82BE, 
    0x90FA93F0, 0x92CC8253, 0x90C98286, 0x82DC82B6, 0x8DBD82EA, 0x8A9F954B, 0x90C982B4, 0x82E8825A, 0x82AA82C8, 0x8B4181E7, 
    0x93839269, 0x82D68258, 0x828B95C6, 0x81BD82A2, 0x96930A42, 0x41815291, 0x48899D8A, 0xC982EB96, 0xA482EF89, 0xDF82BD82, 
    0x4B96C982, 0xBD82EA82, 0xBE917391, 0xC182BE82, 0xAA82BD82, 0xBB824181, 0xE690CC82, 0xCC91C582, 0xB582B18C, 0xCC82BD82, 
    0x7A91CD82, 0xF0829C91, 0xB782A295, 0xA482E682, 0x6F8FC882, 0x968E8897, 0xC182BE82, 0x4281BD82, 0x75810A0A, 0xE082BB82, 
    0xE082BB82, 0xA0824181, 0xBD82C882, 0x4E92CD82, 0xF182C882, 0xB782C582, 0x4881A982, 0x0A0A7681, 0xEF89C48D, 0xBD82B582, 
    0x48899D8A, 0xCD82EB96, 0xBD82E297, 0xE296AD82, 0x4281A482, 0x82DC820A, 0x81C582E9, 0x826D9241, 0x82C882E7, 0x8A6C90A2, 
    0x8CF082D4, 0x82E982A9, 0x82A482E6, 0x82B593C8, 0x917391F0, 0x8CC982BE, 0x82AF82FC, 0x816381C4, 0x0A428163, 0x91B19C0A, 
    0x82C68252, 0x8EE982B7, 0x8C6C90E5, 0x8ACD82F6, 0x9648899D, 0x94CC82EB, 0x82A796E9, 0x826D92F0, 0x82B182E9, 0x82C982C6, 
    0x81E982C8, 0x968E0A42, 0xC582CC8C, 0xFA8A5A92, 0xAF894C8B, 0x518AE18F, 0xD78AC982, 0xBD82C182, 0x978FAD8F, 0x63816381, 
    0xAF894C8B, 0x4F8EF082, 0xB582FA93, 0xDB88A982, 0xC5829D8E, 0xC882AB82, 0xC682A282, 0xA482A282, 0xE28DE38F, 0x48899D8A, 
    0x4281EB96, 0x6F8F0A0A, 0xA282EF89, 0x4181E082, 0x6C90F193, 0x5994C582, 0xBE82F182, 0xC682B182, 0x4181E082, 0x58834C83, 
    0xB582F082, 0xB182BD82, 0xE082C682, 0x63816381, 0xC4825391, 0xED82B88E, 0xC482EA82, 0xDC82B582, 0xBD82C182, 0x48899D8A, 
    0xCC82EB96, 0xAF894C8B, 0x0A0A4281, 0xCC82BB82, 0xEE8F968E, 0x6D92F082, 0xBD82C182, 0xBE917391, 0x4181CD82, 0x978FDE94, 
    0x4C8BCC82, 0xC682AF89, 0xB3905E90, 0xA982CA96, 0xA797E782, 0xFC8CBF82, 0xA482A982, 0xC682B182, 0x888CF082, 0xB782D388, 
    0x4281E982, 0x4C8B0A0A, 0xF082AF89, 0xFA934F8E, 0xA982B582, 0x9D8EDB88, 0xAB82C582, 0xA282C882, 0x978FAD8F, 0xE38F4181, 
    0x9D8AE28D, 0xEB964889, 0xCC82C682, 0xEF896F8F, 0xAA82A282, 0x7191A992, 0xBE917391, 0xA895F082, 0xCC82EA8C, 0xD6828692, 
    0xC1825597, 0xA282C482, 0x4281AD82, 0x82BB820A, 0x90C482B5, 0x82EA8247, 0x828793E9, 0x96E994CC, 0x81C682A7, 0x8FAD8F41, 
    0x94CC8297, 0x81A796E9, 0x53910A42, 0xF082C482, 0xC1826D92, 0xF193BD82, 0xAA826C90, 0xAB82FC8C, 0xA482878D, 0xC08E5E90, 
    0xCD82C682, 0x5C815C81, 0x0A0A4881, 0x52977C81, 0x718EE494, 0xCD8FCC82, 0x0A0A7C81, 0x788B7E93, 0xF082DD82, 0x70979897, 
    0x4181B582, 0xC5828793, 0x93837983, 0x87835683, 0xF0829383, 0x63896F8C, 0xE982B782, 0xEA95668F, 0xE082CC82, 0xC982C682, 
    0xEA824B96, 0x7781BD82, 0x7191A992, 0xBE917391, 0x830A7881, 0x83938379, 0x83878356, 0x83C58293, 0x838B8341, 0x8343836F, 
    0x82F08267, 0x82C882B5, 0x8FE782AA, 0x8285965D, 0x82A082C5, 0x8E7781E9, 0x97448D4F, 0x8EE49452, 0x81788171, 0x91778141, 
    0x8174976F, 0x966F8E78, 0x81E28285, 0x93639741, 0x82F590E9, 0x987781CC, 0x8C708A5A, 0x815998DC, 0x8AC68278, 0x82B58279, 
    0x81FA93A2, 0x89F08258, 0x82B282DF, 0x82C482B5, 0x81AD82A2, 0x820A0A42, 0x82C282A2, 0x95C682E0, 0x82ED82CF, 0x93CA82E7, 
    0x82C58287, 0x82E995CC, 0x81B582E7, 0xDD820A42, 0xC882F182, 0xA882C582, 0xE882D58D, 0xD489E282, 0xF082CE89, 0xB582798A, 
    0x4281DE82, 0x82BE820A, 0x82A082AA, 0x81FA93E9, 0x91739141, 0x97C682BE, 0x8EE49452, 0x8FCC8271, 0x8285965D, 0x82A282C6, 
    0x8CD68AA4, 0x81CD8257, 0x82768E41, 0x82CA82ED, 0x82B182C6, 0x95C582EB, 0x8BEA82F6, 0x81E9828E, 0x820A0A42, 0x82C682D3, 
    0x82BD82B5, 0x82C682B1, 0x82AB82F0, 0x82A982C1, 0x81C982AF, 0x94529741, 0x82718EE4, 0x917391CD, 0x91C982BE, 0x82A2827A, 
    0x826093F0, 0x82C482A6, 0x82BD82AB, 0x81BE82CC, 0xBE820A42, 0x4181AA82, 0x9F8B718E, 0xA08DCC82, 0xE782A982, 0x8F8FEA88, 
    0xC182BE82, 0xB182BD82, 0xE082C682, 0xE882A082, 0x52974181, 0x718EE494, 0x438BCC82, 0xBF829D8E, 0xA28DC982, 0xB7826698, 
    0x7391E982, 0x4281BE91, 0x82FC890A, 0x8EC482DF, 0x82AA95A9, 0x8E438BCC, 0x82BF829D, 0x82C282C9, 0x8DC482A2, 0x82A6826C, 
    0x0A4281E9, 0xB582BB82, 0x4181C482, 0xE8824892, 0xA2828592, 0x9A93BD82, 0x6381A682, 0xA98E6381, 0xCD82AA95, 0xE4945297, 
    0xCC82718E, 0xC682B182, 0x448DAA82, 0xBE82AB82, 0xA282C682, 0x438BA482, 0xBF829D8E, 0x0A0A4281, 0xCC82BB82, 0x9D8E438B, 
    0xF082BF82, 0xBC92B390, 0x6093C982, 0x4181A682, 0xC182E282, 0xCC82C682, 0xC682B182, 0x4181C582, 0xAB827495, 0xA482878D, 
    0xC682B182, 0xC882C682, 0xBD82C182, 0x6C90F193, 0x900A4281, 0x82EA82B0, 0x974181C4, 0x90E99363, 0x82A982F5, 0x90F697E7, 
    0x82D6826C, 0x8CD68AC6, 0x90F08257, 0x82DF825B, 0x82A282C4, 0x0A4281AD, 0x82B5820A, 0x81B582A9, 0x824B8D41, 0x93C882B9, 
    0x825881FA, 0x82B191AA, 0x82C482A2, 0x82AD82A2, 0x82768EC6, 0x82EA82ED, 0x92C58DBD, 0x93418186, 0x826C90F1, 0x824F91CC, 
    0x825996C9, 0x828E8BEA, 0x82BD82C1, 0x82B882CD, 0x8BDF89CC, 0x97AA828E, 0x82BF82A7, 0x82B382D3, 0x82C182AA, 0x815C81BD, 
    0x917D8B5C, 0x95C982AC, 0x8BEA82F6, 0x82C1828E, 0x82A282C4, 0x8FFA93AD, 0x81C982ED, 0x90F19341, 0x82CD826C, 0x97A482C7, 
    0x8CBF82A7, 0x82A982FC, 0x82C482C1, 0x82AD82A2, 0x81A982CC, 0x0A488149, 0xB582BB82, 0x5297C482, 0x718EE494, 0x5996AA82, 
    0x8E8BEA82, 0xBD82C182, 0x8E8BDF89, 0xCD82C682, 0x5C815C81, 0x0A0A4881, 0xB98D7C81, 0x85975881, 0x50788898, 0x0DE6227B, 
    0x844DC440, 0xDE4E4175, 0xD421201C, 0x6B82D4D2, 0x8BB28F3E, 0x6CD1AB40, 0x54849DF3, 0xD07EBD05, 0x305A1DBA, 0xB126F2D3, 
    0x4CBEC0FB, 0x2923A6F0, 0x94A6AE7B, 0xDF56EFD1, 0x16EAED49, 0x7BAA60A1, 0x182C0C58, 0x6B6FB9D2, 0x314C08BD, 0xA10CCC71, 
    0xD4D4AACE, 0xA249B578, 0x4D78971F, 0xC2D3A50C, 0x225F8589, 0xFD14C567, 0x55AA43F6, 0x85E847A7, 0x1E9D6379, 0xA466566E, 
    0x8D5D94CE, 0xB0656668, 0x4AA45510, 0x1ED1548E, 0x33090443, 0x89E55A34, 0x19628010, 0x446C3A01, 0x26205C84, 0xE3A063EC, 
    0xF182C8F1, 0x28693F54, 0x99AF59D0, 0x33053250, 0xFE5A276A, 0x7620E300, 0x0045F9B0, 0x4D5B314F, 0x7D7F4007, 0x9DF19B5D, 
    0xD7A87DE4, 0x312F4CC6, 0xDD202464, 0x9617924F, 0xE82B2198, 0xF3AC2660, 0xD47BC600, 0x3FE3D5CD, 0xD5108943, 0x721E6866, 
    0xD8A6286C, 0xFFCF8EE7, 0xAD428755, 0xB090B1FF, 0x74E7639C, 0x940CC0E0, 0x4A5B5FC8, 0xAB915F12, 0x16DCA8D7, 0x6217C194, 
    0xA23EA443, 0xF2EACD93, 0x2EF6E4B4, 0xF74070E6, 0x857DDFE3, 0xE49A4944, 0xC10B1BBE, 0x95427B79, 0x2F178713, 0xFB8D9214, 
    0x6524AC1F, 0x58D27DFD, 0xD7424456, 0xB7E01275, 0x988DCFFA, 0x312A4FF7, 0x0F574F1B, 0x52D15691, 0x04FBB24E, 0xFA6BD5A0, 
    0x96081539, 0x96CE8FB4, 0xAC03E24D, 0x1376D2C8, 0x9332B283, 0x3F74449A, 0x75AE0040, 0x644D3706, 0xAB40B566, 0x85625428, 
    0xA61D4A3C, 0x6B3A7077, 0x5C69FB38, 0xE286EE54, 0x8903B4FD, 0xE075B93F, 0x5431550C, 0x5BEF7B44, 0xFEE57F0C, 0x7E581A57, 
    0xA406852C, 0x6B1CA649, 0xF501001C, 0x8EFBC4AE, 0xE2A57DFE, 0x364BF257, 0x44658CEC, 0x6738FE8A, 0xA655F3E2, 0x619DC418, 
    0xF2C6DC9E, 0xA0C845C8, 0x7ABAB9E3, 0x73217AED, 0xDAFDA399, 0x0246B55A, 0x2135E844, 0x5F08F461, 0x3CC54044, 0x1F996B18, 
    0xD7A3D857, 0x18812AED, 0xB81F7859, 0xEFC0FB93, 0xE8AB6100, 0x529D3609, 0x24F07C13, 0xA4C24C2D, 0x06553CB6, 0x7ECBEB0C, 
    0x57D18233, 0x257D8E84, 0xA300A785, 0xBBB0DE7A, 0xCD1230F6, 0x1F56D159, 0xA8DADEC5, 0x7D687C15, 0xB6724F82, 0xAA10F023, 
    0x0EFD5F28, 0xF3EAC4B1, 0xF288559B, 0x93812641, 0x64E1C8DB, 0x8B436CEA, 0xE7DEFEC8, 0xAD28B9E9, 0xB8577B84, 0x12470290, 
    0xCEF433F2, 0x563F8D32, 0x61DD1AA2, 0x1749D56D, 0x51C9C357, 0x3193124F, 0xC43E4773, 0xF5D9998C, 0xA108EAD4, 0x86E15979, 
    0x5FF3061A, 0xEBBC8C45, 0x6F7B19C9, 0x6837BDFA, 0xABACDE38, 0xD1F4DE1F, 0x41211C59, 0x7ECFF567, 0x6EEB8706, 0xF3C4D26D, 
    0x3F7EF938, 0x5B39BA0C, 0xA9BD34A7, 0xE980E6D5, 0x55AAFEDE, 0xF5300CF5, 0xB538932C, 0xDFDCB986, 0x927104D4, 0x3956DFAC, 
    0xBAA0BC9A, 0xDA3BE0D1, 0x1BD56C26, 0xAEC4D0EC, 0xAB484C65, 0xA382C5B2, 0x62C97032, 0x729DE9A9, 0x05CB30AC, 0x80E588B7, 
    0xA216C308, 0xC79C94DC, 0x60E3E774, 0x07085099, 0xE7FB73AA, 0xBB23D7A2, 0x81F65ED0, 0x7D12B641, 0x90A826DC, 0x6E4E93D5, 
    0x656398DA, 0x66D79906, 0xA0541CD6, 0x6CFF2AFD, 0xA76BA6CA, 0x97EC7F3E, 0x5D57CBC2, 0xA6C1C8FE, 0xCFE6ECDD, 0x7216A199, 
    0xE031C838, 0xB51B3A21, 0x8A1C7E71, 0x39DE7F7D, 0x15937C29, 0x3777B562, 0xC8CFC2A9, 0xEE07BC36, 0x0D86AE93, 0xC4C8B10A, 
    0xBD0D6F84, 0x91EA75E4, 0x4AE44929, 0x2BAEDFA0, 0xCC924E2E, 0x72BC28C3, 0x61EBD46F, 0x60AA8E98, 0x55064885, 0x1034883C, 
    0x8E9142AC, 0xB0BD8DD9, 0xBE4BC61B, 0xDA4ADFBB, 0x2DBC19B8, 0x51E955C2, 0xCE94D7E8, 0xFD759DE5, 0x8DA81E5F, 0xCC9DEDF6, 
    0x94D9260B, 0xC69DE5B0, 0x4105DB6D, 0xB3B04B7E
};