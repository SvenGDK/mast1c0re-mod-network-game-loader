#include <mast1c0re.hpp>
#include "downloader/Downloader.hpp"

#define SERVER_PORT 9045

void main()
{
    // Set pad light to lime green
    PS::PadSetLightBar(150, 255, 0, 255);

    // Show "PS2 Network Game Loader" notification
    PS::notificationWithIcon("cxml://psnotification/tex_morpheus_trophy_platinum", "PS2 Network Game Loader");

    // Attach another notification
    PS::notificationWithIcon("cxml://psnotification/tex_default_icon_download", "v0.1.5-Mod by SvenGDK");

    // Attempt to connect to debug server
    // PS::Debug.connect(IP(192, 168, 0, 7), 9023);
    PS::Debug.printf("---------- Load PS2 Game (Network) ----------\n");

    // Set paths
    const char* gameFilepath = "/av_contents/content_tmp/disc01.iso";
    const char* configFilepath = "/av_contents/content_tmp/SCUS-97129_cli.conf";
    bool hasConfig = false;

    if (!PS::Filesystem::exists(gameFilepath))
    {
        // Download ISO
        if (!Downloader::downloadGame(gameFilepath, SERVER_PORT))
        {
            // Failed to download ISO
            PS::notification("Failed to download ISO");
            PS::Debug.printf("Failed to download ISO\n");

            // Disconnect from debug server
            PS::Debug.disconnect();
            return;
        }
    }

    // Open a new dialog to ask for the config file
    char message[512];
    PS2::sprintf(message, "Do you want to load a config file?", "Config Loader");
    if (PS::Sce::MsgDialogUserMessage::show(message, PS::Sce::MsgDialog::ButtonType::YESNO))
    {
        // Set pad light to yellow
        PS::PadSetLightBar(250, 230, 40, 255);

        hasConfig = true;

        if (!PS::Filesystem::exists(configFilepath))
        {
            // Download Config
            if (!Downloader::downloadConfig(configFilepath, SERVER_PORT))
            {
                // Failed to download config
                PS::notification("Failed to download config");
                PS::Debug.printf("Failed to download config\n");

                // Disconnect from debug server
                PS::Debug.disconnect();
                return;
            }
        }
    }

    // Mount & Load iso
    PS::Debug.printf("Mounting...\n");
    PS::MountDiscWithFilepath("./../av_contents/content_tmp/disc01.iso");

    // Get game code from mounted file
    char* gameCode = PS::GetMountedGameCode();
    if (PS2::strlen(gameCode) == 10)
    {
        // Convert name from "SCUS-97129" -> "cdrom0:\\SCUS_971.29;1"
        char* ps2Path = PS2::gameCodeToPath(gameCode);

        // Load configuration file
        if (hasConfig == true)
        {   
            PS::Debug.printf("Processing config %s\n", configFilepath);
            PS::ProcessConfigFile("./../av_contents/content_tmp/SCUS-97129_cli.conf");
        }

        // Disconnect from debug server
        PS::Debug.printf("Loading \"%s\"...\n", ps2Path);

        // Disconnect from debug server
        PS::Debug.disconnect();

        // Restore corruption
        PS::Breakout::restore();

        // Execute mounted iso
        PS2::LoadExecPS2(ps2Path, 0, NULL);
        return;
    }

    PS::notification("Unexpected game code \"%s\"!", gameCode);
    PS::Debug.printf("Unexpected game code (%s) length of %i, expecting %i\n", gameCode, PS2::strlen(gameCode), 10);

    // Disconnect from debug server
    PS::Debug.disconnect();
}