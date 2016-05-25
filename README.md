# vw_t4_tcu_temp_to_fis
Arduino project to readout Automatic Transmission Fluid Temp and write to FIS display from a VW T4 2002

Using OBD protocol KW1281 for getting the ATF temp from the Transmission Control Unit (TCU)
  Using 3LB protocol to show info on FIS-display

  Credits:

  This project would not have been possible without the help of other open source projects out there.
  Credits and thanks goes to:

  Alexander's software and OBD2-interface
  
  http://grauonline.de/wordpress/?p=74
  docs: // https://www.blafusel.de/obd/obd2_kw1281.html

  NOTE: For the level shifting, I used a 'AutoDia K409 Profi USB adapter', disassembled it,
      and connected the Arduino to the level shifter chip (LM339) - the original FTDI chip TX line
      was removed (so it does not influence the communication)

  And to these projects for information for communication with the FIS-display
    Thanks for publishing the code for the 1999 Audi
    https://github.com/tomaskovacik/arduino/tree/master/VWFIS

  Thanks for the code to understand the 3LB protocl. This is really a great project!
    https://github.com/pohlinkzei/RPi_MFD_Interface
  Thanks for the hints for checksum generation.
  I managed to figure out the 2002 VW checksum-generation after study this page and numbercrunching in LibreOffice-Calc :-)
    http://www.gti-tdi.de/board4/index.php?thread/3787-tacho-full-fis-displaing-3lb/
