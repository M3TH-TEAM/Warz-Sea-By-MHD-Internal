#pragma once

#define VALID( x ) ( x != NULL && HIWORD( x ) )

#define dwRenderAddress                        0xEFAC14  - 0x400000                    // r3dRender.h                 

#define matTmp_off			                   0x220		                           // PS_DS_SSR_VS                 
#define matView_off			                   0x1A0		                           // PS_DS_SSR_VS              

#define Disconnect			                   0x0D3FF0		                           // ***** disconnected from game server\n               

#define pGetGame_Adr                           0x133CE20  - 0x400000                    // clan               
#define pGameWorld_Adr                         0xFC903C  - 0x400000                    // WARNING: Object %s can't be loaded!\n          

#define Localplayer_Adr                        0xFE4                                   // local player dropped by server
    
#define m_pSkeleton_off			               0x0DDD8		                           		// PrimaryWeaponBone               
#define BoneCoeff_off			               0x0DFF8		                           		 // PrimaryWeaponBone             
#define CSkeleton_off			               0x1C                                        // PrimaryWeaponBone  

#define PickupIteam_off				           0x0DE38                                    // GravestoneSuicide
#define Superjumping                           0x0E8B8                                    // PLAYER_JUMPING_F
#define Stamina_offset                         0x0DF10                                   // PLAYER_DRINK
                        
#define GetHealth_off			               0x2D2C                                      // InfoMsg_MaxHealthAlready                
#define Objecttype_off                         0x0E0C                                    // Weapon owner must be obj_Player                  
#define Getposition_off                        0x2C                                    // F3 0F 7E 07 66 0F D6 86 ?? ?? ?? ?? 8B 47 08            

#define XOR_OBJLIST_MAX	                       0x0                                     
#define XOR_OBJLIST_PTR                        0x0       