.. raw:: html

    <div id="AISummaryWidget"></div>

================
MP Pack Tool
================

This document introduces the functionalities and usage of the MP Pack Tool, supporting chips including RTL8762C, RTL8762D, RTL8762E, RTL8762G_VA, RTL8762G_VB, and RTL8752H. The MP Pack Tool is used to merge subfiles and generate data packet files such as :term:`MP`, :term:`OTA`, or :term:`CFU`.

- MP Packet: Used for mass production. It can be flashed into the device using the MP Tool.
- OTA Packet: Used for Over-The-Air updates. It can be upgraded via Android/iOS OTA apps.
- CFU Packet: Used for Component Firmware Updates. It can be upgraded using the CFU Tool.

Download the MP Pack Tool from the RealMCU_ platform.

Packaging Steps
================

.. figure:: ../figures/Pack_Guide.*
   :align: center
   :scale: 65%

   MP Pack Packaging Process

1. Select Packaging Mode

   Choose from three modes. This example uses MP mode; OTA and CFU packaging follow similar steps.  

   - :guilabel:`ForMP`  
   - :guilabel:`ForOTA`  
   - :guilabel:`ForCFU`  

2. Import :file:`flash map.ini`

   - The flash map defines the flash layout. For details, refer to :doc:`Flash Map Generate Tool<../../flashmaptool/text_en/README>`.

3. Import Other Files

   - Click :guilabel:`Browse` to select files, then click :guilabel:`Open` to import.  
   - The tool validates imported files (MD5 checks, IC verification, etc.). Valid files appear in the list (Step 4).  

4. Imported File List

   - Displays: File Name, Path, Address, Size, and Version.  

5. Remove Files (if required)

   - Select a file and click :guilabel:`Remove` to delete it.  
   - Click :guilabel:`Remove All` to clear all files.  
   - Removal resets the 'File Path', 'Size', and 'Version' fields, and sets the 'Address' to default.  

6. Import User Data (Optional)

   - User Data refers to custom files. For details, see `MP Packaging`_ and `OTA Packaging`_ sections.  

7. Modify Save Path and Filename (Optional)

   - Check :guilabel:`Save Path` and click :guilabel:`...` to choose a directory (default: MP Pack Tool root).  
   - Check :guilabel:`File Name` to rename the output file (default: ``ImgPacketFile-MD5.bin``).  

8. Generate Packaged File

   - Click :guilabel:`Confirm` to generate the file at the specified location.  

9. Log Window

   - Displays prompts and error messages.  

MP Packaging
======================

MP packets are used for mass production and can be flashed via the MP Tool.

.. figure:: ../figures/PackTool_MP.*
   :align: center
   :scale: 65%
   :name: fig-packtool-mp

   MP Packaging Interface

Check :guilabel:`ForMP` to generate an MP packet for mass production.

Adding User Data  
-------------------------------

MP mode supports custom User Data. Follow these steps:  

.. figure:: ../figures/User_Data1.*
   :align: center
   :scale: 65%

   Adding User Data in MP Mode  

1. Click :guilabel:`User Data` to open the User Data window.  
2. Click :guilabel:`Browse` to select User Data files.  
3. Use :guilabel:`Remove` or :guilabel:`Remove All` to delete files.  
4. Manually adjust the User Data address to avoid conflicts. Overlaps trigger an error: :kbd:`Overlapping Exists!`  
5. Click :guilabel:`Confirm` to finalize. 
6. After the above steps are completed, the User Data checkbox will be automatically checked, indicating that User Data will be packaged. 

.. note::  
   - Ensure User Data does not overlap with other files.  
   - A ``WithUserData`` file is generated alongside the standard MP packet.  

MP Raw Data
--------------------------------

Check :guilabel:`Raw Data` to generate a Raw Data packet:  

.. figure:: ../figures/MP_RawData_Setting.*
   :align: center
   :scale: 70%
   :name: mp-raw-data-packet

   MP Raw Data Packet  

This generates two files:

  - Standard MP packet

  - :ref:`MP Raw Data Format<mp-raw-data-format>` File

.. figure:: ../figures/MP_RawData.*
   :align: center
   :scale: 100%
   :name: mp-raw-data-format

   MP Raw Data Format  

OTA Packaging
=================

OTA packets enable Over-The-Air updates via Android/iOS apps.

.. figure:: ../figures/PackTool_OTA.*
   :align: center
   :scale: 65%
   :name: fig-packtool-ota

   OTA Packaging Interface  

Check :guilabel:`ForOTA` to generate an OTA packet.  

Adding User Data  
------------------

For RTL8762C, RTL8762G_VB, and RTL8752H, User Data can be added as shown below. Other ICs do not support User Data in OTA.  

.. figure:: ../figures/User_Data2.*
   :align: center
   :scale: 65%

   Adding User Data in OTA Mode  

.. note:: User Data must include MP Header and Image Header for OTA compatibility.  

OTA Raw Data  
--------------------

Check :guilabel:`Raw Data` to generate Raw Data packets:  

.. figure:: ../figures/OTA_RawData_Setting.*
   :align: center
   :scale: 70%
   :name: ota-raw-data-packet

   OTA Raw Data Packet  

Output files vary by bank configuration, the files generated are shown below:

- Single Bank: 
  
  - Standard OTA package
  - Raw OTA data package

- Dual Bank: 

  - Standard OTA package
  - Bank0 OTA raw data package
  - Bank1 OTA raw data package
  - Full bank OTA raw data package  

The format of OTA Raw Data packet is shown in the figure below.

.. figure:: ../figures/OTA_RawData.*
   :align: center
   :scale: 100%

   OTA Raw Data Format  

Compress Mode  
--------------------------

Enable :guilabel:`Compress` to compress images larger than :file:`flash map.ini`'s OTA_TMP_SIZE:  

.. figure:: ../figures/OTA_Compress_Setting.*
   :align: center
   :scale: 70%

   OTA Compress Mode  

CFU Packaging
=======================

CFU packets enable Component Firmware Updates via the CFU Tool.

.. figure:: ../figures/PackTool_CFU.*
   :align: center
   :scale: 65%
   :name: fig-packtool-cfu

   CFU Packaging Interface  

Check :guilabel:`ForCFU` to generate a CFU packet.  

.. note:: There are two CFU packaging formats:

   - V1: Multiple Images are packaged into a single ``.payload`` file and ``.offer`` file.
   - V2: Each Image is packaged into its own ``.payload`` file and ``.offer`` file.
   Users can switch between them by clicking :guilabel:`V1` and :guilabel:`V2`. Consult your FAE to determine which format to use.

Memory Layout Visualization
==========================

The tool displays memory layout to check overlaps. Click :guilabel:`Show Detail`/:guilabel:`Hide` to toggle the layout dialog:  

.. figure:: ../figures/Layout.*
   :align: center
   :scale: 65%

   File Memory Layout  

.. note:: Overlaps block packaging. Ensure no overlaps in the flash map.  

Unpacking
==========================

Unpack MP/OTA packets (Raw Data unsupported).  

Click :guilabel:`UnPack`, select a packet file. Unpacked files are saved to a folder matching the packet name:  

.. figure:: ../figures/Unpack.*
   :align: center
   :scale: 50%

   Unpacking  

.. _RealMCU: https://www.realmcu.com/en/Resources/tools/RTL87x2x_RTL877xG-Series