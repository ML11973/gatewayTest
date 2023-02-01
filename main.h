
/**
  ******************************************************************************
  * @file           main.h
  * @brief          Header for main.c file.
  ******************************************************************************
  *
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

/* Includes ------------------------------------------------------------------*/
#include <cstdint>

/* Private includes ----------------------------------------------------------*/


/* Exported types ------------------------------------------------------------*/


/* Exported constants --------------------------------------------------------*/


/* Exported macro ------------------------------------------------------------*/


/* Exported functions prototypes ---------------------------------------------*/
void dispatchRxFrames(const uint8_t * data, uint8_t size, uint8_t source);

/* Private defines -----------------------------------------------------------*/


#endif /* __MAIN_H */

/**
 * @mainpage
 *
 * Welcome to the WPAN gateway documentation!
 * This software was built as part of a 2023 HES-SO MSc thesis at HEIG-VD's ReDS institute.
 *
 * The goal of this software is to provide a way to access WPAN nodes remotely.
 *
 * The original use case is regulation of a household's power consumption.
 * It is supposed that the household has autonomous power production capacities
 * like solar panels as well as access to the electrical power grid.
 * This software provides a way to interface with various WPAN nodes to control their power production or consumption.
 * A more specific use case is to minimize a household's power consumption from the grid.
 * This can be achieved by filling power storage (electric car, water heater, backup battery) when the household has surplus self-generated power (solar, wind).
 * For this to work, nodes must monitor household power production, consumption and net grid consumption.
 * A server has to retrieve this data and control power consumers accordingly.
 *
 * A second use case is to route some internet traffic to a remote low-speed
 * network using the extended range WPAN provides, instead of routing an
 * Ethernet cable.
 *
 * # Feature backlog
 * - WPAN manager initialization with path to config file
 * - Allow static node update during execution by parsing config file
 * - Add power and power in dBm to options in config file
 * - Change Node type management from several typed lists to one dynamically
 * allocated pointer vector
 * - Add source port/address to DATA frames for reliable response addresses
 * or check frame ID to determine this
 * - ISM3_Linux driver with hardware reset (needs new version of shield)
 *
 * <table>
 * <tr>
 *  <td>Author</td><td>Marc Leemann</td><td>marc.leemann@master.hes-so.ch</td>
 * </tr>
 * <tr>
 *  <td>Supervisor</td><td>Bertrand Hochet</td><td>bertrand.hochet@heig-vd.ch</td>
 * </tr>
 * </table>
 *
 * v1: February 2023
 */
 /* @subpage hardware_setup
 *
 * @subpage pi_config
 *
 * @subpage linux_config
 *
 * @subpage code_overview
 *
 * @subpage ISM3_manual
 *
 * @subpage nodes
 *
 * @subpage wpanManager_desc
 *
 * @subpage borderrouter
 *
 * @subpage connections
 *
 * @subpage protocols
 *
 * @subpage hardware_doc
 */

/**
 * @page hardware_setup Hardware setup
 *
 * This software was originally developped to be run on a Raspberry Pi CM4
 * supplemented by a custom ISM 868 MHz radio module (from here on referred to
 * as "shield").
 * <table>
 * <tr>
 * <td>@image html assembly_cm4.jpg "Shield and Pi CM4 assembly" height=400</td>
 * <td>@image html cm4.jpg "Pi CM4 board" height=400</td>
 * </table>
 * The Pi CM4 lies on a custom breakout board developped for IoT applications.
 * The shield is a breakout board for a custom radio module.
 * This arrangement allows the CM4 to communicate with the shield via serial
 * peripheral /dev/ttyAMA1, as defined in hardware.h.
 */

/**
 * @page linux_config Linux configuration
 *
 * The CM4 microcomputer uses a custom distribution using Buildroot.
 * It uses the Pi CM4's default bootloader.
 * See @ref platform_config for more information about Buildroot.
 *
 * ### SSH configuration
 *
 * To have a more comfortable debugging experience, it is suggested to copy
 * and run programs remotely using SSH.
 * This requires
 * <a href="https://www.cyberciti.biz/faq/how-to-set-up-ssh-keys-on-linux-unix/">
 * setting up SSH keys</a> for quick remote identification.
 * It is advised to limit the key length to reduce connection times during
 * a development phase.
 *
 * When using the same SSH key for multiple remote hosts, SSH will display a
 * warning and fail.
 * Of course, using the same SSH key is not recommended for a final application.
 * We can simply <a href="https://www.techrepublic.com/article/how-to-easily-add-an-ssh-fingerprint-to-your-knownhosts-file-in-linux/">
 * add the remote host to SSH's known hosts list to connect</a>.
 *
 * @code{sh}
 * ssh-keyscan -H 192.168.1.10 >> ~/.ssh/known_hosts # change IP accordingly
 * @endcode
 *
 * ### Ulimit
 *
 * Ulimit is the Linux resource management utility.
 * When the border router has to handle many concurrent connections, it may run
 * out of resources.
 * It is possible to allocate more resources to user processes by using
 * <a href="https://ss64.com/bash/ulimit.html">ulimit</a>.
 *
 * The most likely crash source is going to be the max limit of open file
 * descriptors. File descriptors allow access to sockets.
 * Program may crash when reaching too high a number of connected Nodes.
 * Use
 * @code{sh}
 * ulimit -n 1024
 * @endcode
 * to grant more open file descriptors to the program and avoid this problem.
 *
 * ### Automatically starting the application
 *
 * It is possible to start the gateway application with the system by creating
 * a service for it.
 * Service manager is BusyBox.
 * Service files are located in /etc/init.d.
 * They have the following structure:
 *
 * @code{sh}
#!/bin/sh

PROGRAM_DIR=~/
PROGRAM_NAME=gateway

start() {
	echo "Starting $PROGRAM_NAME: "
	$PROGRAM_DIR$PROGRAM_NAME
}
stop() {
	printf "Stopping $PROGRAM_NAME: "
	killall $PROGRAM_NAME
}
restart() {
	stop
	start
}

case "$1" in
  start)
	start
	;;
  stop)
	stop
	;;
  restart|reload)
	restart
	;;
* *)
	echo "Usage: $0 {start|stop|restart}"
	exit 1
esac

exit $?

 * @endcode
 *
 * Service file should be named S[XX]gateway, with XX a two-digit order of
 * execution. Example:
 @code{sh}
 S90gateway # Service will be started after service 89
 @endcode

 * BusyBox will execute all properly-named service scripts on startup.

 * ### Automatically restarting the application after a crash
 *
 * It is desirable to restart automatically after a crash, as the
 * application is not guaranteed to be stable.
 * Fortunately, BusyBox can do this for us.
 * The respawn action tells BusyBox to monitor program status and exit code,
 * restarting it if it should fail.
 *
 * **Note:** This should only be done in the final application, using a newer
 * version of the shield that includes a hardware reset.
 * Otherwise the service will probably hang in an incomplete initialization
 * state, as it frequently does at this stage.
 * This is because the current version of gateway software does not restart
 * reliably without hardware-resetting the ISM3 module.
 *
 * To have BusyBox automatically restart a service, modify /etc/inittab to add
 * the following line:
 @code{sh}
 [id]::respawn:[PROGRAM_PATH] [args]
 @endcode
 *
 * ID is the tty to run the service on.
 *
 * For example, if we want to have our service output messages to the console:
  @code{sh}
 console::respawn:~/gateway
 @endcode

 * For more information about BusyBox configuration, refer to /etc/inittab
 * header.
 * @code{sh}
 cat /etc/inittab
 @endcode
 *
 * **Final note:** Since startup scripts in /etc/init.d are executed by
 * /etc/init.d/rcS, you should remove any startup script for the application if
 * you choose to have the service run with the BusyBox respawn option.
 */

/**
 * @page pi_config Platform configuration
 *
 * # Buildroot
 *
 * Buildroot is a toolchain that simplifies custom Linux target creation.
 * It is available on the <a href="https://buildroot.org/">
 * Buildroot project website</a>.
 * It is an archive to be downloaded and compiled on another host system.
 * It also provides cross-compilers.
 * These are useful to quickly compile the Linux kernel and whatever program
 * to be developped using the host machine's considerably higher computing
 * power.
 *
 * The reason Buildroot is interesting is that it allows easy configuration of
 * a custom Linux system.
 * We can tinker with the kernel and configurations in order to only get the
 * functionalities we need.
 * This allows faster boot times and security upgrades.
 *
 * ## Buildroot configuration and initial build
 *
 * Initial setup is easy per
 * <a href="https://buildroot.org/downloads/manual/manual.html#getting-buildroot">
 * the Buildroot manual</a>:
 * 1. Check that the host PC has all the
 * <a href"https://buildroot.org/downloads/manual/manual.html#requirement-mandatory">
 * required packages</a>.
 * 2. Download and extract Buildroot from the
 * <a href="http://buildroot.org/downloads/">Buildroot downloads page</a>.
 *
 * Once again, most of the work has been done for us, in the form of default
 * configurations to save us the tough work of configuring everything ourselves.
 * Our target, the Raspberry Pi CM4, has a lot of community support
 *
 * 3. List available default configurations by running this command in the
 * buildroot root folder:
 * @code{sh}
 * make list-defconfigs
 * @endcode
 * Bingo! There is a default configuration for the Pi CM4:
 * @code{sh}
 * raspberrypicm4io_defconfig
 * @endcode
 *
 * 4. Configure Buildroot for the Pi CM4:
 * @code{sh}
 * make raspberrypicm4io_defconfig
 * @endcode
 *
 * 5. Build the kernel and toolchain (can take over an hour the first time, be
 * patient):
 * @code{sh}
 * make all
 * @endcode
 *
 * 6. Configure Buildroot:
 * @code{sh}
 * make menuconfig
 * @endcode
 *
 * 7. Configure Linux:
 * @code{sh}
 * make linux-menuconfig
 * @endcode
 *
 * 8. Build the toolchain:
 * @code{sh}
 * make
 * @endcode
 *
 * ## Flashing the built Linux
 *
 * Buildroot creates a complete SD card image for us.
 * We can easily flash it with a dedicated script.
 *
 * @code{sh}
 * #!/bin/sh
 * SD_ROOT_FOLDER=/dev/sda # Modify according to your SD path (use lsblk to find it)
 *
 * sudo umount $SD_ROOT_FOLDER
 * #initialize 960MiB to 0
 * sudo dd if=/dev/zero of=$SD_ROOT_FOLDER count=240000
 * sudo dd if=output/images/sdcard.img of=$SD_ROOT_FOLDER # flash SD image
 * sudo umount -r ${SD_ROOT_FOLDER}1
 * sudo umount -r ${SD_ROOT_FOLDER}2
 * sudo umount -r ${SD_ROOT_FOLDER}1 # Twice to get umount confirmation
 * sudo umount -r ${SD_ROOT_FOLDER}2
 * @endcode
 *
 * ## Platform configuration
 *
 * ### Getting a serial terminal
 * @code{sh}
 * make menuconfig
 * @endcode
 * Go to "System configuration" -> "Run a getty after boot" and set the
 * "TTY port" to ttyACM0, "Baudrate" to 115200 and "TERM environment variable"
 * to vt100 per
 * <a href="https://ltekieli.com/buildroot-with-raspberry-pi-what-where-and-how/">
 * this article</a>.
 * Modify /boot/cmdline.txt on your SD card and add the following line:
 * @code{sh}
 * console=ttyAMA0,115200 root=/dev/mmcblk0p2 rootwait
 * @endcode
 * Getting a serial terminal means you can see boot messages from a console
 * application like minicom.
 * This is particularly useful in the early stages of development when network
 * configurations may not be operational making SSH impossible.
 * We can better troubleshoot kernel panics (they will happen some day).
 *
 * Connect to the CM4 with an USB serial adapter connected to
 * <a href="schema_cm4.PDF">UART0</a>.
 * You may need to cross RX with TX if the default wiring does not work.
 *
 * Use minicom or another serial program:
 * @code{sh}
 * minicom -b 115200 -D /dev/ttyUSB0 # replace /dev/ttyUSB0 with your device
 * @endcode
 * To find your device, compare
 * @code{sh}
 * ls /dev
 * @endcode
 * output before and after plugging the USB to serial adapter.
 *
 * ### Network configuration
 *
 * We need to configure the network in order to use SSH.
 * The CM4 board has Ethernet and Wi-Fi capabilities.
 * Ethernet does not require installing packages.
 * Wi-Fi requires the following configuration (per
 * <a href="https://blog.crysys.hu/2018/06/enabling-wifi-and-converting-the-raspberry-pi-into-a-wifi-ap/">
 * this article</a>):
 * @code{sh}
 * make menuconfig
 * @endcode
 * Allow automatic loading of wireless driver:
 * - System configuration -> /dev management -> Dynamic using devtmpfs + mdev
 *
 * Install onboard Wi-Fi firmware:
 * - Target packages -> Hardware handling -> Firmware -> rpi 4 (extended)
 * - Target packages -> Hardware handling -> Firmware -> bcrmfmac-sdio-firmware-rpi
 * Install Wi-Fi configuration packages:
 * - Target packages->Networking applications->iw
 * - Target packages->Networking applications->wpa-supplicant->Enable nl80211 support
 * - Target packages -> Networking applications -> wpa_supplicant -> Install wpa_passphrase binary (optional)
 *
 * Install RF switch into kernel and not as a module:
 * @code{sh}
 * make linux-menuconfig
 * @endcode
 * - Networking support -> RF switch subsystem support (Y)
 *
 * Configuration of network interfaces is done by modifying
 * /etc/network/interfaces.
 * Find below a sample configuration for a static Ethernet address and a DHCP
 * Wi-Fi address.
 *
 * @code{sh}
 * auto lo
 * iface lo inet loopback
 *
 * auto eth0
 * iface eth0 inet static
 *     address 192.168.0.10
 *     gateway 192.168.0.1
 *     netmask 255.255.255.0
 *
 * auto wlan0
 * iface wlan0 inet dhcp
 *     pre-up wpa_supplicant -D nl80211 -i wlan0 -c /etc/wpa_supplicant.conf -B
 *     post-down killall -q wpa_supplicant
 *     wait-delay 15
 * iface default inet dhcp
 * @endcode
 *
 * WPA configuration is done by editing /etc/wpa_supplicant.conf or using the
 * wpa_passphrase utility:
 * @code{sh}
 * country=CH
 * update_config=1
 *
 * network={
 *     ssid="WIFINAME"
 *     psk="PASSWORD"
 * }
 * @endcode
 *
 * ### SSH configuration
 *
 * Buildroot ships Dropbear as the default SSH client.
 * It is a lightweight client.
 * We chose to use OpenSSH to get an easier time setting up SSH keys.
 * To remove Dropbear and install OpenSSH:
 * @code{sh}
 * make menuconfig
 * @endcode
 * Uncheck Dropbear:
 * - Target packages->Networking applications->dropbear
 *
 * Check OpenSSH client, server and key utilities packages:
 * - Target packages->Networking applications->openssh
 *
 * SSH keys should be stored in ~/.ssh and accessible only by root user.
 * We need to create the .ssh folder in rootfs_overlay, populate it with the
 * generated authorized_keys file and set its permissions.
 *
 * Set the permissions by editing buildroot/system/device_table.txt:
 * @code{sh}
 * #path                    type  perms
 * /root/.ssh/                d   700 0   0   -   -   -   -   -
 * /root/.ssh/authorized_keys f   600 0   0   -   -   -   -   -
 * @endcode
 *
 * ### Debugging programs
 *
 * @code{sh}
 * make menuconfig
 * @endcode
 * Per
 * <a href="https://www.oreilly.com/library/view/mastering-embedded-linux/9781787283282/43e79c1d-f609-456d-841e-41c595245c5e.xhtml">
 * this book snippet</a>:
 * - Toolchain -> Build cross gdb for the host
 * - Toolchain -> Enable WCHAR support
 * - Toolchain -> Thread library debugging
 * - Target packages -> Debugging, profiling and benchmark -> gdb
 * - Target packages -> Debugging, profiling and benchmark -> gdbserver
 * - Target packages -> Debugging, profiling and benchmark -> full debugger
 *
 * Using the debugger (per
 * <a href="https://buildroot.org/downloads/manual/using-buildroot-debugger.txt">
 * buildroot manual</a>):
 *
 * On the CM4:
 * @code{sh}
 * gdbserver :2345 [PROGRAM] # GDB server listens on TCP port 2345
 * @endcode
 * On the host PC, from cross-compilation directory:
 * @code{sh}
 * <buildroot>/output/host/bin/<tuple>-gdb -ix <buildroot>/output/staging/usr/share/buildroot/gdbinit [CROSSCOMPILED_PROGRAM]
 * (gdb) target remote <target ip address>:2345 # Connect to gdb server
 * @endcode
 *
 * ## Device tree overlay
 *
 * The device tree overlay is a file that describes the hardware setup so that
 * Linux can know where its peripherals are.
 * It doesn't describe all hardware by default, to keep a lightweight system.
 *
 * Since the gateway application uses the CM4's serial device 4, we have to
 * tell Linux it exists and how.
 * Fortunately, the tedious description work has already been done for us.
 * All we need to do is add the following line at the end of /boot/
 * dtoverlay.
 *
 * @code{sh}
 * # enable UART4
 * dtoverlay=uart4
 * @endcode
 *
 * ## Saving linux configuration in Buildroot
 *
 * ### Rootfs overlay
 *
 * To have your Linux configuration kept by Buildroot instead of resetting it
 * on build,
 * <a href="https://boozlachu.medium.com/buildroot-part-2-bffac4b0b86a">
 * configure and populate rootfs_overlay</a>.
 *
 * By default, buildroot/board/raspberrypicm4io/rootfs_overlay is used.
 * Putting any file there will have it replace the originally generated file.
 *
 * It is possible to modify this path:
 * @code{sh}
 * make menuconfig
 * @endcode
 * - System configuration -> Root filesystem overlay directories
 *
 *
 * ### /boot/config.txt
 *
 * It is possible to modify default /boot/config.txt by editing
 * buildroot/board/raspberrycm4io/config_cm4io.txt.
 *
 * It is thus possible to add the UART4 device tree overlay as described before:
 *
 * @code{sh}
 * # enable UART4
 * dtoverlay=uart4
 * @endcode
 *
 * ### Network configuration
 *
 * Persistent network configuration changes can be kept by editing
 * buildroot/board/raspberrycm4io/interfaces.
 * This file matches /etc/network/interfaces on the Linux image.
 * It is copied after building (see post-build script)
 * The current configuration uses DHCP for the Wi-Fi interface and static
 * addressing for Ethernet.
 *
 * @code{sh}
 * auto lo
 * iface lo inet loopback
 *
 * auto eth0
 * #iface eth0 inet dhcp
 * iface eth0 inet static
 *     address 192.168.0.10
 *     gateway 192.168.0.1
 *     netmask 255.255.255.0
 *     #pre-up /etc/network/nfs_check
 *     #wait-delay 15
 *     #metric 100
 *
 * auto wlan0
 * iface wlan0 inet dhcp
 *     pre-up wpa_supplicant -D nl80211 -i wlan0 -c /etc/wpa_supplicant.conf -B
 *     post-down killall -q wpa_supplicant
 *     wait-delay 15
 * #    metric 100
 * iface default inet dhcp
 *
 * @endcode
 *
 * WPA configuration is in buildroot/board/raspberrycm4io/wpa_supplicant.conf.
 * This file matches /etc/wpa_supplicant.conf.
 * It is called on configuration of network interfaces.
 * Find below an example configuration:
 *
 * @code{sh}
 * country=CH
 * update_config=1
 *
 * network={
 *     ssid="WIFINAME"
 *     psk="PASSWORD"
 * }
 * @endcode
 *
 * ### Post-build script
 * Sometimes it is not possible to put configs in rootfs_overlay.
 * We can use the post_build.sh script in board/raspberrypicm4io/post-build.sh
 * to apply any change we want to the built image before it is created.
 *
 * The script is executed
 * <a href="https://buildroot.org/downloads/manual/manual.html#rootfs-custom">
 * before imaging, but after building</a>.
 *
 * @code{sh}
 *#!/bin/sh
 *
 * set -u
 * set -e
 *
 * # Add a console on tty1
 * if [ -e ${TARGET_DIR}/etc/inittab ]; then
 *     grep -qE '^tty1::' ${TARGET_DIR}/etc/inittab || \
 * 	sed -i '/GENERIC_SERIAL/a\
 * tty1::respawn:/sbin/getty -L  tty1 0 vt100 # HDMI console' ${TARGET_DIR}/etc/inittab
 * fi
 *
 * # enable wi-fi per
 * #https://blog.crysys.hu/2018/06/enabling-wifi-and-converting-the-raspberry-pi-
 * #in t-o-a-wifi-ap/
 * cp package/busybox/S10mdev ${TARGET_DIR}/etc/init.d/S10mdev
 * chmod 755 ${TARGET_DIR}/etc/init.d/S10mdev
 * cp package/busybox/mdev.conf ${TARGET_DIR}/etc/mdev.conf
 * cp board/raspberrypicm4io/interfaces ${TARGET_DIR}/etc/network/interfaces
 * cp board/raspberrypicm4io/wpa_supplicant.conf ${TARGET_DIR}/etc/wpa_supplicant.conf
 * @endcode
 *
 * The script was modified to copy the network configurations into the relevant
 * folders.
 * It is probably not necessary and could be done using rootfs_overlay, but it
 * is working.
 *
 */

/**
 * @page build Build configuration
 *
 * ### Build environment setup
 * To cross compile, we need to add the Buildroot cross compiling toolchain to
 * our PATH. In any console:
 * @code{sh}
 * vim ~/.bashrc
 * export PATH="$PATH:/[PATH_TO_BUILDROOT]/buildroot-2022.08/output/host/usr/bin" # add this line to .bashrc
 * @endcode
 *
 * ### Cross compilation
 *
 * Use arm-linux-gcc or arm-linux-g++ to compile programs.
 * Use normal compiler flags (for example -g for a GDB debug build).
 *
 * ### Project makefile
 *
 * It is easier to use a makefile for compiling.
 * The current makefile is located at the root of the project.
 * It handles cross compilation.
 *
 * ### Launching on a remote target
 *
 * Once SSH key setup is done, it is easy and quick to compile and run the
 * program using a small script.
 *
 * @code{sh}
 * #!/bin/sh
 * PROGRAM_NAME="gatewayTest"
 * HOSTNAME="cm4"
 * if make
 * then
 *    echo "Copying program..."
 *    scp $PROGRAM_NAME root@$HOSTNAME:~/$PROGRAM_NAME # copy program on remote host root
 *    scp nodes.txt root@$HOSTNAME:~/nodes.txt # copy config file
 *    echo "Program output:"
 *    ssh -tt root@$HOSTNAME "killall $PROGRAM_NAME ; ./$PROGRAM_NAME" # kill running version and launch in an SSH terminal
 * fi
 * @endcode
 *
 */

/**
 * @page code_overview Codebase overview
 *
 * @image html project_arch.png "Project architecture"
 *
 * ### ISM3_Linux
 *
 * This package contains the ISM3 driver stack.
 * It is all written in C, ported from an STM32 platform.
 * It communicates with the radio shield via a serial interface.
 *
 * ### WPAN
 *
 * This package contains the Wireless Personal Area Network Manager.
 * The @suboage wpanManager_desc uses the ism3_server.h adaptation layer to
 * use server functions (group wake control and config).
 * It uses the ism3.h layer directly to send frames to nodes.
 *
 * It manages connected Nodes and exports a list of all nodes.
 *
 * ### Router
 *
 * This package contains the @subpage borderrouter.
 * The latter gets a Node list from @subpage wpanManager_desc and opens UDP sockets
 * for each DataNode.
 */

/**
 * @page hardware_doc Hardware documentation
 *
 * <a href="RM1S3 Host Commands.pdf">ISM3 documentation</a>
 *
 * <a href="schema_cm4.PDF">Pi CM4 board schematic</a>
 *
 * <a href="RFShield_docs.PDF">Shield schematics and layout</a>
 */
