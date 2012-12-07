/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                            DDDD    CCCC  M   M                              %
%                            D   D  C      MM MM                              %
%                            D   D  C      M M M                              %
%                            D   D  C      M   M                              %
%                            DDDD    CCCC  M   M                              %
%                                                                             %
%                                                                             %
%                          Read DICOM Image Format                            %
%                                                                             %
%                              Software Design                                %
%                                John Cristy                                  %
%                                 July 1992                                   %
%                                                                             %
%                                                                             %
%  Copyright 1999-2013 ImageMagick Studio LLC, a non-profit organization      %
%  dedicated to making software imaging solutions freely available.           %
%                                                                             %
%  You may not use this file except in compliance with the License.  You may  %
%  obtain a copy of the License at                                            %
%                                                                             %
%    http://www.imagemagick.org/script/license.php                            %
%                                                                             %
%  Unless required by applicable law or agreed to in writing, software        %
%  distributed under the License is distributed on an "AS IS" BASIS,          %
%  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   %
%  See the License for the specific language governing permissions and        %
%  limitations under the License.                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%
*/

/*
  Include declarations.
*/
#include "magick/studio.h"
#include "magick/property.h"
#include "magick/blob.h"
#include "magick/blob-private.h"
#include "magick/cache.h"
#include "magick/color.h"
#include "magick/color-private.h"
#include "magick/colormap.h"
#include "magick/colormap-private.h"
#include "magick/constitute.h"
#include "magick/enhance.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/image.h"
#include "magick/image-private.h"
#include "magick/list.h"
#include "magick/magick.h"
#include "magick/memory_.h"
#include "magick/monitor.h"
#include "magick/monitor-private.h"
#include "magick/option.h"
#include "magick/pixel-accessor.h"
#include "magick/quantum-private.h"
#include "magick/resource_.h"
#include "magick/static.h"
#include "magick/string_.h"
#include "magick/string-private.h"
#include "magick/module.h"

/*
  Dicom medical image declarations.
*/

typedef struct _DicomInfo
{
  unsigned short
    group,
    element;

  const char
    *vr,
    *description;
} DicomInfo;

static const DicomInfo
  dicom_info[] =
  {
    { 0x0000, 0x0000, "UL", "Group Length" },
    { 0x0000, 0x0001, "UL", "Command Length to End" },
    { 0x0000, 0x0002, "UI", "Affected SOP Class UID" },
    { 0x0000, 0x0003, "UI", "Requested SOP Class UID" },
    { 0x0000, 0x0010, "LO", "Command Recognition Code" },
    { 0x0000, 0x0100, "US", "Command Field" },
    { 0x0000, 0x0110, "US", "Message ID" },
    { 0x0000, 0x0120, "US", "Message ID Being Responded To" },
    { 0x0000, 0x0200, "AE", "Initiator" },
    { 0x0000, 0x0300, "AE", "Receiver" },
    { 0x0000, 0x0400, "AE", "Find Location" },
    { 0x0000, 0x0600, "AE", "Move Destination" },
    { 0x0000, 0x0700, "US", "Priority" },
    { 0x0000, 0x0800, "US", "Data Set Type" },
    { 0x0000, 0x0850, "US", "Number of Matches" },
    { 0x0000, 0x0860, "US", "Response Sequence Number" },
    { 0x0000, 0x0900, "US", "Status" },
    { 0x0000, 0x0901, "AT", "Offending Element" },
    { 0x0000, 0x0902, "LO", "Exception Comment" },
    { 0x0000, 0x0903, "US", "Exception ID" },
    { 0x0000, 0x1000, "UI", "Affected SOP Instance UID" },
    { 0x0000, 0x1001, "UI", "Requested SOP Instance UID" },
    { 0x0000, 0x1002, "US", "Event Type ID" },
    { 0x0000, 0x1005, "AT", "Attribute Identifier List" },
    { 0x0000, 0x1008, "US", "Action Type ID" },
    { 0x0000, 0x1020, "US", "Number of Remaining Suboperations" },
    { 0x0000, 0x1021, "US", "Number of Completed Suboperations" },
    { 0x0000, 0x1022, "US", "Number of Failed Suboperations" },
    { 0x0000, 0x1023, "US", "Number of Warning Suboperations" },
    { 0x0000, 0x1030, "AE", "Move Originator Application Entity Title" },
    { 0x0000, 0x1031, "US", "Move Originator Message ID" },
    { 0x0000, 0x4000, "LO", "Dialog Receiver" },
    { 0x0000, 0x4010, "LO", "Terminal Type" },
    { 0x0000, 0x5010, "SH", "Message Set ID" },
    { 0x0000, 0x5020, "SH", "End Message Set" },
    { 0x0000, 0x5110, "LO", "Display Format" },
    { 0x0000, 0x5120, "LO", "Page Position ID" },
    { 0x0000, 0x5130, "LO", "Text Format ID" },
    { 0x0000, 0x5140, "LO", "Normal Reverse" },
    { 0x0000, 0x5150, "LO", "Add Gray Scale" },
    { 0x0000, 0x5160, "LO", "Borders" },
    { 0x0000, 0x5170, "IS", "Copies" },
    { 0x0000, 0x5180, "LO", "OldMagnificationType" },
    { 0x0000, 0x5190, "LO", "Erase" },
    { 0x0000, 0x51a0, "LO", "Print" },
    { 0x0000, 0x51b0, "US", "Overlays" },
    { 0x0002, 0x0000, "UL", "Meta Element Group Length" },
    { 0x0002, 0x0001, "OB", "File Meta Information Version" },
    { 0x0002, 0x0002, "UI", "Media Storage SOP Class UID" },
    { 0x0002, 0x0003, "UI", "Media Storage SOP Instance UID" },
    { 0x0002, 0x0010, "UI", "Transfer Syntax UID" },
    { 0x0002, 0x0012, "UI", "Implementation Class UID" },
    { 0x0002, 0x0013, "SH", "Implementation Version Name" },
    { 0x0002, 0x0016, "AE", "Source Application Entity Title" },
    { 0x0002, 0x0100, "UI", "Private Information Creator UID" },
    { 0x0002, 0x0102, "OB", "Private Information" },
    { 0x0003, 0x0000, "US", "?" },
    { 0x0003, 0x0008, "US", "ISI Command Field" },
    { 0x0003, 0x0011, "US", "Attach ID Application Code" },
    { 0x0003, 0x0012, "UL", "Attach ID Message Count" },
    { 0x0003, 0x0013, "DA", "Attach ID Date" },
    { 0x0003, 0x0014, "TM", "Attach ID Time" },
    { 0x0003, 0x0020, "US", "Message Type" },
    { 0x0003, 0x0030, "DA", "Max Waiting Date" },
    { 0x0003, 0x0031, "TM", "Max Waiting Time" },
    { 0x0004, 0x0000, "UL", "File Set Group Length" },
    { 0x0004, 0x1130, "CS", "File Set ID" },
    { 0x0004, 0x1141, "CS", "File Set Descriptor File ID" },
    { 0x0004, 0x1142, "CS", "File Set Descriptor File Specific Character Set" },
    { 0x0004, 0x1200, "UL", "Root Directory Entity First Directory Record Offset" },
    { 0x0004, 0x1202, "UL", "Root Directory Entity Last Directory Record Offset" },
    { 0x0004, 0x1212, "US", "File Set Consistency Flag" },
    { 0x0004, 0x1220, "SQ", "Directory Record Sequence" },
    { 0x0004, 0x1400, "UL", "Next Directory Record Offset" },
    { 0x0004, 0x1410, "US", "Record In Use Flag" },
    { 0x0004, 0x1420, "UL", "Referenced Lower Level Directory Entity Offset" },
    { 0x0004, 0x1430, "CS", "Directory Record Type" },
    { 0x0004, 0x1432, "UI", "Private Record UID" },
    { 0x0004, 0x1500, "CS", "Referenced File ID" },
    { 0x0004, 0x1504, "UL", "MRDR Directory Record Offset" },
    { 0x0004, 0x1510, "UI", "Referenced SOP Class UID In File" },
    { 0x0004, 0x1511, "UI", "Referenced SOP Instance UID In File" },
    { 0x0004, 0x1512, "UI", "Referenced Transfer Syntax UID In File" },
    { 0x0004, 0x1600, "UL", "Number of References" },
    { 0x0005, 0x0000, "US", "?" },
    { 0x0006, 0x0000, "US", "?" },
    { 0x0008, 0x0000, "UL", "Identifying Group Length" },
    { 0x0008, 0x0001, "UL", "Length to End" },
    { 0x0008, 0x0005, "CS", "Specific Character Set" },
    { 0x0008, 0x0008, "CS", "Image Type" },
    { 0x0008, 0x0010, "LO", "Recognition Code" },
    { 0x0008, 0x0012, "DA", "Instance Creation Date" },
    { 0x0008, 0x0013, "TM", "Instance Creation Time" },
    { 0x0008, 0x0014, "UI", "Instance Creator UID" },
    { 0x0008, 0x0016, "UI", "SOP Class UID" },
    { 0x0008, 0x0018, "UI", "SOP Instance UID" },
    { 0x0008, 0x0020, "DA", "Study Date" },
    { 0x0008, 0x0021, "DA", "Series Date" },
    { 0x0008, 0x0022, "DA", "Acquisition Date" },
    { 0x0008, 0x0023, "DA", "Image Date" },
    { 0x0008, 0x0024, "DA", "Overlay Date" },
    { 0x0008, 0x0025, "DA", "Curve Date" },
    { 0x0008, 0x0030, "TM", "Study Time" },
    { 0x0008, 0x0031, "TM", "Series Time" },
    { 0x0008, 0x0032, "TM", "Acquisition Time" },
    { 0x0008, 0x0033, "TM", "Image Time" },
    { 0x0008, 0x0034, "TM", "Overlay Time" },
    { 0x0008, 0x0035, "TM", "Curve Time" },
    { 0x0008, 0x0040, "xs", "Old Data Set Type" },
    { 0x0008, 0x0041, "xs", "Old Data Set Subtype" },
    { 0x0008, 0x0042, "CS", "Nuclear Medicine Series Type" },
    { 0x0008, 0x0050, "SH", "Accession Number" },
    { 0x0008, 0x0052, "CS", "Query/Retrieve Level" },
    { 0x0008, 0x0054, "AE", "Retrieve AE Title" },
    { 0x0008, 0x0058, "UI", "Failed SOP Instance UID List" },
    { 0x0008, 0x0060, "CS", "Modality" },
    { 0x0008, 0x0062, "SQ", "Modality Subtype" },
    { 0x0008, 0x0064, "CS", "Conversion Type" },
    { 0x0008, 0x0068, "CS", "Presentation Intent Type" },
    { 0x0008, 0x0070, "LO", "Manufacturer" },
    { 0x0008, 0x0080, "LO", "Institution Name" },
    { 0x0008, 0x0081, "ST", "Institution Address" },
    { 0x0008, 0x0082, "SQ", "Institution Code Sequence" },
    { 0x0008, 0x0090, "PN", "Referring Physician's Name" },
    { 0x0008, 0x0092, "ST", "Referring Physician's Address" },
    { 0x0008, 0x0094, "SH", "Referring Physician's Telephone Numbers" },
    { 0x0008, 0x0100, "SH", "Code Value" },
    { 0x0008, 0x0102, "SH", "Coding Scheme Designator" },
    { 0x0008, 0x0103, "SH", "Coding Scheme Version" },
    { 0x0008, 0x0104, "LO", "Code Meaning" },
    { 0x0008, 0x0105, "CS", "Mapping Resource" },
    { 0x0008, 0x0106, "DT", "Context Group Version" },
    { 0x0008, 0x010b, "CS", "Code Set Extension Flag" },
    { 0x0008, 0x010c, "UI", "Private Coding Scheme Creator UID" },
    { 0x0008, 0x010d, "UI", "Code Set Extension Creator UID" },
    { 0x0008, 0x010f, "CS", "Context Identifier" },
    { 0x0008, 0x1000, "LT", "Network ID" },
    { 0x0008, 0x1010, "SH", "Station Name" },
    { 0x0008, 0x1030, "LO", "Study Description" },
    { 0x0008, 0x1032, "SQ", "Procedure Code Sequence" },
    { 0x0008, 0x103e, "LO", "Series Description" },
    { 0x0008, 0x1040, "LO", "Institutional Department Name" },
    { 0x0008, 0x1048, "PN", "Physician of Record" },
    { 0x0008, 0x1050, "PN", "Performing Physician's Name" },
    { 0x0008, 0x1060, "PN", "Name of Physician(s) Reading Study" },
    { 0x0008, 0x1070, "PN", "Operator's Name" },
    { 0x0008, 0x1080, "LO", "Admitting Diagnosis Description" },
    { 0x0008, 0x1084, "SQ", "Admitting Diagnosis Code Sequence" },
    { 0x0008, 0x1090, "LO", "Manufacturer's Model Name" },
    { 0x0008, 0x1100, "SQ", "Referenced Results Sequence" },
    { 0x0008, 0x1110, "SQ", "Referenced Study Sequence" },
    { 0x0008, 0x1111, "SQ", "Referenced Study Component Sequence" },
    { 0x0008, 0x1115, "SQ", "Referenced Series Sequence" },
    { 0x0008, 0x1120, "SQ", "Referenced Patient Sequence" },
    { 0x0008, 0x1125, "SQ", "Referenced Visit Sequence" },
    { 0x0008, 0x1130, "SQ", "Referenced Overlay Sequence" },
    { 0x0008, 0x1140, "SQ", "Referenced Image Sequence" },
    { 0x0008, 0x1145, "SQ", "Referenced Curve Sequence" },
    { 0x0008, 0x1148, "SQ", "Referenced Previous Waveform" },
    { 0x0008, 0x114a, "SQ", "Referenced Simultaneous Waveforms" },
    { 0x0008, 0x114c, "SQ", "Referenced Subsequent Waveform" },
    { 0x0008, 0x1150, "UI", "Referenced SOP Class UID" },
    { 0x0008, 0x1155, "UI", "Referenced SOP Instance UID" },
    { 0x0008, 0x1160, "IS", "Referenced Frame Number" },
    { 0x0008, 0x1195, "UI", "Transaction UID" },
    { 0x0008, 0x1197, "US", "Failure Reason" },
    { 0x0008, 0x1198, "SQ", "Failed SOP Sequence" },
    { 0x0008, 0x1199, "SQ", "Referenced SOP Sequence" },
    { 0x0008, 0x2110, "CS", "Old Lossy Image Compression" },
    { 0x0008, 0x2111, "ST", "Derivation Description" },
    { 0x0008, 0x2112, "SQ", "Source Image Sequence" },
    { 0x0008, 0x2120, "SH", "Stage Name" },
    { 0x0008, 0x2122, "IS", "Stage Number" },
    { 0x0008, 0x2124, "IS", "Number of Stages" },
    { 0x0008, 0x2128, "IS", "View Number" },
    { 0x0008, 0x2129, "IS", "Number of Event Timers" },
    { 0x0008, 0x212a, "IS", "Number of Views in Stage" },
    { 0x0008, 0x2130, "DS", "Event Elapsed Time(s)" },
    { 0x0008, 0x2132, "LO", "Event Timer Name(s)" },
    { 0x0008, 0x2142, "IS", "Start Trim" },
    { 0x0008, 0x2143, "IS", "Stop Trim" },
    { 0x0008, 0x2144, "IS", "Recommended Display Frame Rate" },
    { 0x0008, 0x2200, "CS", "Transducer Position" },
    { 0x0008, 0x2204, "CS", "Transducer Orientation" },
    { 0x0008, 0x2208, "CS", "Anatomic Structure" },
    { 0x0008, 0x2218, "SQ", "Anatomic Region Sequence" },
    { 0x0008, 0x2220, "SQ", "Anatomic Region Modifier Sequence" },
    { 0x0008, 0x2228, "SQ", "Primary Anatomic Structure Sequence" },
    { 0x0008, 0x2230, "SQ", "Primary Anatomic Structure Modifier Sequence" },
    { 0x0008, 0x2240, "SQ", "Transducer Position Sequence" },
    { 0x0008, 0x2242, "SQ", "Transducer Position Modifier Sequence" },
    { 0x0008, 0x2244, "SQ", "Transducer Orientation Sequence" },
    { 0x0008, 0x2246, "SQ", "Transducer Orientation Modifier Sequence" },
    { 0x0008, 0x2251, "SQ", "Anatomic Structure Space Or Region Code Sequence" },
    { 0x0008, 0x2253, "SQ", "Anatomic Portal Of Entrance Code Sequence" },
    { 0x0008, 0x2255, "SQ", "Anatomic Approach Direction Code Sequence" },
    { 0x0008, 0x2256, "ST", "Anatomic Perspective Description" },
    { 0x0008, 0x2257, "SQ", "Anatomic Perspective Code Sequence" },
    { 0x0008, 0x2258, "ST", "Anatomic Location Of Examining Instrument Description" },
    { 0x0008, 0x2259, "SQ", "Anatomic Location Of Examining Instrument Code Sequence" },
    { 0x0008, 0x225a, "SQ", "Anatomic Structure Space Or Region Modifier Code Sequence" },
    { 0x0008, 0x225c, "SQ", "OnAxis Background Anatomic Structure Code Sequence" },
    { 0x0008, 0x4000, "LT", "Identifying Comments" },
    { 0x0009, 0x0000, "xs", "?" },
    { 0x0009, 0x0001, "xs", "?" },
    { 0x0009, 0x0002, "xs", "?" },
    { 0x0009, 0x0003, "xs", "?" },
    { 0x0009, 0x0004, "xs", "?" },
    { 0x0009, 0x0005, "UN", "?" },
    { 0x0009, 0x0006, "UN", "?" },
    { 0x0009, 0x0007, "UN", "?" },
    { 0x0009, 0x0008, "xs", "?" },
    { 0x0009, 0x0009, "LT", "?" },
    { 0x0009, 0x000a, "IS", "?" },
    { 0x0009, 0x000b, "IS", "?" },
    { 0x0009, 0x000c, "IS", "?" },
    { 0x0009, 0x000d, "IS", "?" },
    { 0x0009, 0x000e, "IS", "?" },
    { 0x0009, 0x000f, "UN", "?" },
    { 0x0009, 0x0010, "xs", "?" },
    { 0x0009, 0x0011, "xs", "?" },
    { 0x0009, 0x0012, "xs", "?" },
    { 0x0009, 0x0013, "xs", "?" },
    { 0x0009, 0x0014, "xs", "?" },
    { 0x0009, 0x0015, "xs", "?" },
    { 0x0009, 0x0016, "xs", "?" },
    { 0x0009, 0x0017, "LT", "?" },
    { 0x0009, 0x0018, "LT", "Data Set Identifier" },
    { 0x0009, 0x001a, "US", "?" },
    { 0x0009, 0x001e, "UI", "?" },
    { 0x0009, 0x0020, "xs", "?" },
    { 0x0009, 0x0021, "xs", "?" },
    { 0x0009, 0x0022, "SH", "User Orientation" },
    { 0x0009, 0x0023, "SL", "Initiation Type" },
    { 0x0009, 0x0024, "xs", "?" },
    { 0x0009, 0x0025, "xs", "?" },
    { 0x0009, 0x0026, "xs", "?" },
    { 0x0009, 0x0027, "xs", "?" },
    { 0x0009, 0x0029, "xs", "?" },
    { 0x0009, 0x002a, "SL", "?" },
    { 0x0009, 0x002c, "LO", "Series Comments" },
    { 0x0009, 0x002d, "SL", "Track Beat Average" },
    { 0x0009, 0x002e, "FD", "Distance Prescribed" },
    { 0x0009, 0x002f, "LT", "?" },
    { 0x0009, 0x0030, "xs", "?" },
    { 0x0009, 0x0031, "xs", "?" },
    { 0x0009, 0x0032, "LT", "?" },
    { 0x0009, 0x0034, "xs", "?" },
    { 0x0009, 0x0035, "SL", "Gantry Locus Type" },
    { 0x0009, 0x0037, "SL", "Starting Heart Rate" },
    { 0x0009, 0x0038, "xs", "?" },
    { 0x0009, 0x0039, "SL", "RR Window Offset" },
    { 0x0009, 0x003a, "SL", "Percent Cycle Imaged" },
    { 0x0009, 0x003e, "US", "?" },
    { 0x0009, 0x003f, "US", "?" },
    { 0x0009, 0x0040, "xs", "?" },
    { 0x0009, 0x0041, "xs", "?" },
    { 0x0009, 0x0042, "xs", "?" },
    { 0x0009, 0x0043, "xs", "?" },
    { 0x0009, 0x0050, "LT", "?" },
    { 0x0009, 0x0051, "xs", "?" },
    { 0x0009, 0x0060, "LT", "?" },
    { 0x0009, 0x0061, "LT", "Series Unique Identifier" },
    { 0x0009, 0x0070, "LT", "?" },
    { 0x0009, 0x0080, "LT", "?" },
    { 0x0009, 0x0091, "LT", "?" },
    { 0x0009, 0x00e2, "LT", "?" },
    { 0x0009, 0x00e3, "UI", "Equipment UID" },
    { 0x0009, 0x00e6, "SH", "Genesis Version Now" },
    { 0x0009, 0x00e7, "UL", "Exam Record Checksum" },
    { 0x0009, 0x00e8, "UL", "?" },
    { 0x0009, 0x00e9, "SL", "Actual Series Data Time Stamp" },
    { 0x0009, 0x00f2, "UN", "?" },
    { 0x0009, 0x00f3, "UN", "?" },
    { 0x0009, 0x00f4, "LT", "?" },
    { 0x0009, 0x00f5, "xs", "?" },
    { 0x0009, 0x00f6, "LT", "PDM Data Object Type Extension" },
    { 0x0009, 0x00f8, "US", "?" },
    { 0x0009, 0x00fb, "IS", "?" },
    { 0x0009, 0x1002, "OB", "?" },
    { 0x0009, 0x1003, "OB", "?" },
    { 0x0009, 0x1010, "UN", "?" },
    { 0x0010, 0x0000, "UL", "Patient Group Length" },
    { 0x0010, 0x0010, "PN", "Patient's Name" },
    { 0x0010, 0x0020, "LO", "Patient's ID" },
    { 0x0010, 0x0021, "LO", "Issuer of Patient's ID" },
    { 0x0010, 0x0030, "DA", "Patient's Birth Date" },
    { 0x0010, 0x0032, "TM", "Patient's Birth Time" },
    { 0x0010, 0x0040, "CS", "Patient's Sex" },
    { 0x0010, 0x0050, "SQ", "Patient's Insurance Plan Code Sequence" },
    { 0x0010, 0x1000, "LO", "Other Patient's ID's" },
    { 0x0010, 0x1001, "PN", "Other Patient's Names" },
    { 0x0010, 0x1005, "PN", "Patient's Birth Name" },
    { 0x0010, 0x1010, "AS", "Patient's Age" },
    { 0x0010, 0x1020, "DS", "Patient's Size" },
    { 0x0010, 0x1030, "DS", "Patient's Weight" },
    { 0x0010, 0x1040, "LO", "Patient's Address" },
    { 0x0010, 0x1050, "LT", "Insurance Plan Identification" },
    { 0x0010, 0x1060, "PN", "Patient's Mother's Birth Name" },
    { 0x0010, 0x1080, "LO", "Military Rank" },
    { 0x0010, 0x1081, "LO", "Branch of Service" },
    { 0x0010, 0x1090, "LO", "Medical Record Locator" },
    { 0x0010, 0x2000, "LO", "Medical Alerts" },
    { 0x0010, 0x2110, "LO", "Contrast Allergies" },
    { 0x0010, 0x2150, "LO", "Country of Residence" },
    { 0x0010, 0x2152, "LO", "Region of Residence" },
    { 0x0010, 0x2154, "SH", "Patients Telephone Numbers" },
    { 0x0010, 0x2160, "SH", "Ethnic Group" },
    { 0x0010, 0x2180, "SH", "Occupation" },
    { 0x0010, 0x21a0, "CS", "Smoking Status" },
    { 0x0010, 0x21b0, "LT", "Additional Patient History" },
    { 0x0010, 0x21c0, "US", "Pregnancy Status" },
    { 0x0010, 0x21d0, "DA", "Last Menstrual Date" },
    { 0x0010, 0x21f0, "LO", "Patients Religious Preference" },
    { 0x0010, 0x4000, "LT", "Patient Comments" },
    { 0x0011, 0x0001, "xs", "?" },
    { 0x0011, 0x0002, "US", "?" },
    { 0x0011, 0x0003, "LT", "Patient UID" },
    { 0x0011, 0x0004, "LT", "Patient ID" },
    { 0x0011, 0x000a, "xs", "?" },
    { 0x0011, 0x000b, "SL", "Effective Series Duration" },
    { 0x0011, 0x000c, "SL", "Num Beats" },
    { 0x0011, 0x000d, "LO", "Radio Nuclide Name" },
    { 0x0011, 0x0010, "xs", "?" },
    { 0x0011, 0x0011, "xs", "?" },
    { 0x0011, 0x0012, "LO", "Dataset Name" },
    { 0x0011, 0x0013, "LO", "Dataset Type" },
    { 0x0011, 0x0015, "xs", "?" },
    { 0x0011, 0x0016, "SL", "Energy Number" },
    { 0x0011, 0x0017, "SL", "RR Interval Window Number" },
    { 0x0011, 0x0018, "SL", "MG Bin Number" },
    { 0x0011, 0x0019, "FD", "Radius Of Rotation" },
    { 0x0011, 0x001a, "SL", "Detector Count Zone" },
    { 0x0011, 0x001b, "SL", "Num Energy Windows" },
    { 0x0011, 0x001c, "SL", "Energy Offset" },
    { 0x0011, 0x001d, "SL", "Energy Range" },
    { 0x0011, 0x001f, "SL", "Image Orientation" },
    { 0x0011, 0x0020, "xs", "?" },
    { 0x0011, 0x0021, "xs", "?" },
    { 0x0011, 0x0022, "xs", "?" },
    { 0x0011, 0x0023, "xs", "?" },
    { 0x0011, 0x0024, "SL", "FOV Mask Y Cutoff Angle" },
    { 0x0011, 0x0025, "xs", "?" },
    { 0x0011, 0x0026, "SL", "Table Orientation" },
    { 0x0011, 0x0027, "SL", "ROI Top Left" },
    { 0x0011, 0x0028, "SL", "ROI Bottom Right" },
    { 0x0011, 0x0030, "xs", "?" },
    { 0x0011, 0x0031, "xs", "?" },
    { 0x0011, 0x0032, "UN", "?" },
    { 0x0011, 0x0033, "LO", "Energy Correct Name" },
    { 0x0011, 0x0034, "LO", "Spatial Correct Name" },
    { 0x0011, 0x0035, "xs", "?" },
    { 0x0011, 0x0036, "LO", "Uniformity Correct Name" },
    { 0x0011, 0x0037, "LO", "Acquisition Specific Correct Name" },
    { 0x0011, 0x0038, "SL", "Byte Order" },
    { 0x0011, 0x003a, "SL", "Picture Format" },
    { 0x0011, 0x003b, "FD", "Pixel Scale" },
    { 0x0011, 0x003c, "FD", "Pixel Offset" },
    { 0x0011, 0x003e, "SL", "FOV Shape" },
    { 0x0011, 0x003f, "SL", "Dataset Flags" },
    { 0x0011, 0x0040, "xs", "?" },
    { 0x0011, 0x0041, "LT", "Medical Alerts" },
    { 0x0011, 0x0042, "LT", "Contrast Allergies" },
    { 0x0011, 0x0044, "FD", "Threshold Center" },
    { 0x0011, 0x0045, "FD", "Threshold Width" },
    { 0x0011, 0x0046, "SL", "Interpolation Type" },
    { 0x0011, 0x0055, "FD", "Period" },
    { 0x0011, 0x0056, "FD", "ElapsedTime" },
    { 0x0011, 0x00a1, "DA", "Patient Registration Date" },
    { 0x0011, 0x00a2, "TM", "Patient Registration Time" },
    { 0x0011, 0x00b0, "LT", "Patient Last Name" },
    { 0x0011, 0x00b2, "LT", "Patient First Name" },
    { 0x0011, 0x00b4, "LT", "Patient Hospital Status" },
    { 0x0011, 0x00bc, "TM", "Current Location Time" },
    { 0x0011, 0x00c0, "LT", "Patient Insurance Status" },
    { 0x0011, 0x00d0, "LT", "Patient Billing Type" },
    { 0x0011, 0x00d2, "LT", "Patient Billing Address" },
    { 0x0013, 0x0000, "LT", "Modifying Physician" },
    { 0x0013, 0x0010, "xs", "?" },
    { 0x0013, 0x0011, "SL", "?" },
    { 0x0013, 0x0012, "xs", "?" },
    { 0x0013, 0x0016, "SL", "AutoTrack Peak" },
    { 0x0013, 0x0017, "SL", "AutoTrack Width" },
    { 0x0013, 0x0018, "FD", "Transmission Scan Time" },
    { 0x0013, 0x0019, "FD", "Transmission Mask Width" },
    { 0x0013, 0x001a, "FD", "Copper Attenuator Thickness" },
    { 0x0013, 0x001c, "FD", "?" },
    { 0x0013, 0x001d, "FD", "?" },
    { 0x0013, 0x001e, "FD", "Tomo View Offset" },
    { 0x0013, 0x0020, "LT", "Patient Name" },
    { 0x0013, 0x0022, "LT", "Patient Id" },
    { 0x0013, 0x0026, "LT", "Study Comments" },
    { 0x0013, 0x0030, "DA", "Patient Birthdate" },
    { 0x0013, 0x0031, "DS", "Patient Weight" },
    { 0x0013, 0x0032, "LT", "Patients Maiden Name" },
    { 0x0013, 0x0033, "LT", "Referring Physician" },
    { 0x0013, 0x0034, "LT", "Admitting Diagnosis" },
    { 0x0013, 0x0035, "LT", "Patient Sex" },
    { 0x0013, 0x0040, "LT", "Procedure Description" },
    { 0x0013, 0x0042, "LT", "Patient Rest Direction" },
    { 0x0013, 0x0044, "LT", "Patient Position" },
    { 0x0013, 0x0046, "LT", "View Direction" },
    { 0x0015, 0x0001, "DS", "Stenosis Calibration Ratio" },
    { 0x0015, 0x0002, "DS", "Stenosis Magnification" },
    { 0x0015, 0x0003, "DS", "Cardiac Calibration Ratio" },
    { 0x0018, 0x0000, "UL", "Acquisition Group Length" },
    { 0x0018, 0x0010, "LO", "Contrast/Bolus Agent" },
    { 0x0018, 0x0012, "SQ", "Contrast/Bolus Agent Sequence" },
    { 0x0018, 0x0014, "SQ", "Contrast/Bolus Administration Route Sequence" },
    { 0x0018, 0x0015, "CS", "Body Part Examined" },
    { 0x0018, 0x0020, "CS", "Scanning Sequence" },
    { 0x0018, 0x0021, "CS", "Sequence Variant" },
    { 0x0018, 0x0022, "CS", "Scan Options" },
    { 0x0018, 0x0023, "CS", "MR Acquisition Type" },
    { 0x0018, 0x0024, "SH", "Sequence Name" },
    { 0x0018, 0x0025, "CS", "Angio Flag" },
    { 0x0018, 0x0026, "SQ", "Intervention Drug Information Sequence" },
    { 0x0018, 0x0027, "TM", "Intervention Drug Stop Time" },
    { 0x0018, 0x0028, "DS", "Intervention Drug Dose" },
    { 0x0018, 0x0029, "SQ", "Intervention Drug Code Sequence" },
    { 0x0018, 0x002a, "SQ", "Additional Drug Sequence" },
    { 0x0018, 0x0030, "LO", "Radionuclide" },
    { 0x0018, 0x0031, "LO", "Radiopharmaceutical" },
    { 0x0018, 0x0032, "DS", "Energy Window Centerline" },
    { 0x0018, 0x0033, "DS", "Energy Window Total Width" },
    { 0x0018, 0x0034, "LO", "Intervention Drug Name" },
    { 0x0018, 0x0035, "TM", "Intervention Drug Start Time" },
    { 0x0018, 0x0036, "SQ", "Intervention Therapy Sequence" },
    { 0x0018, 0x0037, "CS", "Therapy Type" },
    { 0x0018, 0x0038, "CS", "Intervention Status" },
    { 0x0018, 0x0039, "CS", "Therapy Description" },
    { 0x0018, 0x0040, "IS", "Cine Rate" },
    { 0x0018, 0x0050, "DS", "Slice Thickness" },
    { 0x0018, 0x0060, "DS", "KVP" },
    { 0x0018, 0x0070, "IS", "Counts Accumulated" },
    { 0x0018, 0x0071, "CS", "Acquisition Termination Condition" },
    { 0x0018, 0x0072, "DS", "Effective Series Duration" },
    { 0x0018, 0x0073, "CS", "Acquisition Start Condition" },
    { 0x0018, 0x0074, "IS", "Acquisition Start Condition Data" },
    { 0x0018, 0x0075, "IS", "Acquisition Termination Condition Data" },
    { 0x0018, 0x0080, "DS", "Repetition Time" },
    { 0x0018, 0x0081, "DS", "Echo Time" },
    { 0x0018, 0x0082, "DS", "Inversion Time" },
    { 0x0018, 0x0083, "DS", "Number of Averages" },
    { 0x0018, 0x0084, "DS", "Imaging Frequency" },
    { 0x0018, 0x0085, "SH", "Imaged Nucleus" },
    { 0x0018, 0x0086, "IS", "Echo Number(s)" },
    { 0x0018, 0x0087, "DS", "Magnetic Field Strength" },
    { 0x0018, 0x0088, "DS", "Spacing Between Slices" },
    { 0x0018, 0x0089, "IS", "Number of Phase Encoding Steps" },
    { 0x0018, 0x0090, "DS", "Data Collection Diameter" },
    { 0x0018, 0x0091, "IS", "Echo Train Length" },
    { 0x0018, 0x0093, "DS", "Percent Sampling" },
    { 0x0018, 0x0094, "DS", "Percent Phase Field of View" },
    { 0x0018, 0x0095, "DS", "Pixel Bandwidth" },
    { 0x0018, 0x1000, "LO", "Device Serial Number" },
    { 0x0018, 0x1004, "LO", "Plate ID" },
    { 0x0018, 0x1010, "LO", "Secondary Capture Device ID" },
    { 0x0018, 0x1012, "DA", "Date of Secondary Capture" },
    { 0x0018, 0x1014, "TM", "Time of Secondary Capture" },
    { 0x0018, 0x1016, "LO", "Secondary Capture Device Manufacturer" },
    { 0x0018, 0x1018, "LO", "Secondary Capture Device Manufacturer Model Name" },
    { 0x0018, 0x1019, "LO", "Secondary Capture Device Software Version(s)" },
    { 0x0018, 0x1020, "LO", "Software Version(s)" },
    { 0x0018, 0x1022, "SH", "Video Image Format Acquired" },
    { 0x0018, 0x1023, "LO", "Digital Image Format Acquired" },
    { 0x0018, 0x1030, "LO", "Protocol Name" },
    { 0x0018, 0x1040, "LO", "Contrast/Bolus Route" },
    { 0x0018, 0x1041, "DS", "Contrast/Bolus Volume" },
    { 0x0018, 0x1042, "TM", "Contrast/Bolus Start Time" },
    { 0x0018, 0x1043, "TM", "Contrast/Bolus Stop Time" },
    { 0x0018, 0x1044, "DS", "Contrast/Bolus Total Dose" },
    { 0x0018, 0x1045, "IS", "Syringe Counts" },
    { 0x0018, 0x1046, "DS", "Contrast Flow Rate" },
    { 0x0018, 0x1047, "DS", "Contrast Flow Duration" },
    { 0x0018, 0x1048, "CS", "Contrast/Bolus Ingredient" },
    { 0x0018, 0x1049, "DS", "Contrast/Bolus Ingredient Concentration" },
    { 0x0018, 0x1050, "DS", "Spatial Resolution" },
    { 0x0018, 0x1060, "DS", "Trigger Time" },
    { 0x0018, 0x1061, "LO", "Trigger Source or Type" },
    { 0x0018, 0x1062, "IS", "Nominal Interval" },
    { 0x0018, 0x1063, "DS", "Frame Time" },
    { 0x0018, 0x1064, "LO", "Framing Type" },
    { 0x0018, 0x1065, "DS", "Frame Time Vector" },
    { 0x0018, 0x1066, "DS", "Frame Delay" },
    { 0x0018, 0x1067, "DS", "Image Trigger Delay" },
    { 0x0018, 0x1068, "DS", "Group Time Offset" },
    { 0x0018, 0x1069, "DS", "Trigger Time Offset" },
    { 0x0018, 0x106a, "CS", "Synchronization Trigger" },
    { 0x0018, 0x106b, "UI", "Synchronization Frame of Reference" },
    { 0x0018, 0x106e, "UL", "Trigger Sample Position" },
    { 0x0018, 0x1070, "LO", "Radiopharmaceutical Route" },
    { 0x0018, 0x1071, "DS", "Radiopharmaceutical Volume" },
    { 0x0018, 0x1072, "TM", "Radiopharmaceutical Start Time" },
    { 0x0018, 0x1073, "TM", "Radiopharmaceutical Stop Time" },
    { 0x0018, 0x1074, "DS", "Radionuclide Total Dose" },
    { 0x0018, 0x1075, "DS", "Radionuclide Half Life" },
    { 0x0018, 0x1076, "DS", "Radionuclide Positron Fraction" },
    { 0x0018, 0x1077, "DS", "Radiopharmaceutical Specific Activity" },
    { 0x0018, 0x1080, "CS", "Beat Rejection Flag" },
    { 0x0018, 0x1081, "IS", "Low R-R Value" },
    { 0x0018, 0x1082, "IS", "High R-R Value" },
    { 0x0018, 0x1083, "IS", "Intervals Acquired" },
    { 0x0018, 0x1084, "IS", "Intervals Rejected" },
    { 0x0018, 0x1085, "LO", "PVC Rejection" },
    { 0x0018, 0x1086, "IS", "Skip Beats" },
    { 0x0018, 0x1088, "IS", "Heart Rate" },
    { 0x0018, 0x1090, "IS", "Cardiac Number of Images" },
    { 0x0018, 0x1094, "IS", "Trigger Window" },
    { 0x0018, 0x1100, "DS", "Reconstruction Diameter" },
    { 0x0018, 0x1110, "DS", "Distance Source to Detector" },
    { 0x0018, 0x1111, "DS", "Distance Source to Patient" },
    { 0x0018, 0x1114, "DS", "Estimated Radiographic Magnification Factor" },
    { 0x0018, 0x1120, "DS", "Gantry/Detector Tilt" },
    { 0x0018, 0x1121, "DS", "Gantry/Detector Slew" },
    { 0x0018, 0x1130, "DS", "Table Height" },
    { 0x0018, 0x1131, "DS", "Table Traverse" },
    { 0x0018, 0x1134, "CS", "Table Motion" },
    { 0x0018, 0x1135, "DS", "Table Vertical Increment" },
    { 0x0018, 0x1136, "DS", "Table Lateral Increment" },
    { 0x0018, 0x1137, "DS", "Table Longitudinal Increment" },
    { 0x0018, 0x1138, "DS", "Table Angle" },
    { 0x0018, 0x113a, "CS", "Table Type" },
    { 0x0018, 0x1140, "CS", "Rotation Direction" },
    { 0x0018, 0x1141, "DS", "Angular Position" },
    { 0x0018, 0x1142, "DS", "Radial Position" },
    { 0x0018, 0x1143, "DS", "Scan Arc" },
    { 0x0018, 0x1144, "DS", "Angular Step" },
    { 0x0018, 0x1145, "DS", "Center of Rotation Offset" },
    { 0x0018, 0x1146, "DS", "Rotation Offset" },
    { 0x0018, 0x1147, "CS", "Field of View Shape" },
    { 0x0018, 0x1149, "IS", "Field of View Dimension(s)" },
    { 0x0018, 0x1150, "IS", "Exposure Time" },
    { 0x0018, 0x1151, "IS", "X-ray Tube Current" },
    { 0x0018, 0x1152, "IS", "Exposure" },
    { 0x0018, 0x1153, "IS", "Exposure in uAs" },
    { 0x0018, 0x1154, "DS", "AveragePulseWidth" },
    { 0x0018, 0x1155, "CS", "RadiationSetting" },
    { 0x0018, 0x1156, "CS", "Rectification Type" },
    { 0x0018, 0x115a, "CS", "RadiationMode" },
    { 0x0018, 0x115e, "DS", "ImageAreaDoseProduct" },
    { 0x0018, 0x1160, "SH", "Filter Type" },
    { 0x0018, 0x1161, "LO", "TypeOfFilters" },
    { 0x0018, 0x1162, "DS", "IntensifierSize" },
    { 0x0018, 0x1164, "DS", "ImagerPixelSpacing" },
    { 0x0018, 0x1166, "CS", "Grid" },
    { 0x0018, 0x1170, "IS", "Generator Power" },
    { 0x0018, 0x1180, "SH", "Collimator/Grid Name" },
    { 0x0018, 0x1181, "CS", "Collimator Type" },
    { 0x0018, 0x1182, "IS", "Focal Distance" },
    { 0x0018, 0x1183, "DS", "X Focus Center" },
    { 0x0018, 0x1184, "DS", "Y Focus Center" },
    { 0x0018, 0x1190, "DS", "Focal Spot(s)" },
    { 0x0018, 0x1191, "CS", "Anode Target Material" },
    { 0x0018, 0x11a0, "DS", "Body Part Thickness" },
    { 0x0018, 0x11a2, "DS", "Compression Force" },
    { 0x0018, 0x1200, "DA", "Date of Last Calibration" },
    { 0x0018, 0x1201, "TM", "Time of Last Calibration" },
    { 0x0018, 0x1210, "SH", "Convolution Kernel" },
    { 0x0018, 0x1240, "IS", "Upper/Lower Pixel Values" },
    { 0x0018, 0x1242, "IS", "Actual Frame Duration" },
    { 0x0018, 0x1243, "IS", "Count Rate" },
    { 0x0018, 0x1244, "US", "Preferred Playback Sequencing" },
    { 0x0018, 0x1250, "SH", "Receiving Coil" },
    { 0x0018, 0x1251, "SH", "Transmitting Coil" },
    { 0x0018, 0x1260, "SH", "Plate Type" },
    { 0x0018, 0x1261, "LO", "Phosphor Type" },
    { 0x0018, 0x1300, "DS", "Scan Velocity" },
    { 0x0018, 0x1301, "CS", "Whole Body Technique" },
    { 0x0018, 0x1302, "IS", "Scan Length" },
    { 0x0018, 0x1310, "US", "Acquisition Matrix" },
    { 0x0018, 0x1312, "CS", "Phase Encoding Direction" },
    { 0x0018, 0x1314, "DS", "Flip Angle" },
    { 0x0018, 0x1315, "CS", "Variable Flip Angle Flag" },
    { 0x0018, 0x1316, "DS", "SAR" },
    { 0x0018, 0x1318, "DS", "dB/dt" },
    { 0x0018, 0x1400, "LO", "Acquisition Device Processing Description" },
    { 0x0018, 0x1401, "LO", "Acquisition Device Processing Code" },
    { 0x0018, 0x1402, "CS", "Cassette Orientation" },
    { 0x0018, 0x1403, "CS", "Cassette Size" },
    { 0x0018, 0x1404, "US", "Exposures on Plate" },
    { 0x0018, 0x1405, "IS", "Relative X-ray Exposure" },
    { 0x0018, 0x1450, "DS", "Column Angulation" },
    { 0x0018, 0x1460, "DS", "Tomo Layer Height" },
    { 0x0018, 0x1470, "DS", "Tomo Angle" },
    { 0x0018, 0x1480, "DS", "Tomo Time" },
    { 0x0018, 0x1490, "CS", "Tomo Type" },
    { 0x0018, 0x1491, "CS", "Tomo Class" },
    { 0x0018, 0x1495, "IS", "Number of Tomosynthesis Source Images" },
    { 0x0018, 0x1500, "CS", "PositionerMotion" },
    { 0x0018, 0x1508, "CS", "Positioner Type" },
    { 0x0018, 0x1510, "DS", "PositionerPrimaryAngle" },
    { 0x0018, 0x1511, "DS", "PositionerSecondaryAngle" },
    { 0x0018, 0x1520, "DS", "PositionerPrimaryAngleIncrement" },
    { 0x0018, 0x1521, "DS", "PositionerSecondaryAngleIncrement" },
    { 0x0018, 0x1530, "DS", "DetectorPrimaryAngle" },
    { 0x0018, 0x1531, "DS", "DetectorSecondaryAngle" },
    { 0x0018, 0x1600, "CS", "Shutter Shape" },
    { 0x0018, 0x1602, "IS", "Shutter Left Vertical Edge" },
    { 0x0018, 0x1604, "IS", "Shutter Right Vertical Edge" },
    { 0x0018, 0x1606, "IS", "Shutter Upper Horizontal Edge" },
    { 0x0018, 0x1608, "IS", "Shutter Lower Horizonta lEdge" },
    { 0x0018, 0x1610, "IS", "Center of Circular Shutter" },
    { 0x0018, 0x1612, "IS", "Radius of Circular Shutter" },
    { 0x0018, 0x1620, "IS", "Vertices of Polygonal Shutter" },
    { 0x0018, 0x1622, "US", "Shutter Presentation Value" },
    { 0x0018, 0x1623, "US", "Shutter Overlay Group" },
    { 0x0018, 0x1700, "CS", "Collimator Shape" },
    { 0x0018, 0x1702, "IS", "Collimator Left Vertical Edge" },
    { 0x0018, 0x1704, "IS", "Collimator Right Vertical Edge" },
    { 0x0018, 0x1706, "IS", "Collimator Upper Horizontal Edge" },
    { 0x0018, 0x1708, "IS", "Collimator Lower Horizontal Edge" },
    { 0x0018, 0x1710, "IS", "Center of Circular Collimator" },
    { 0x0018, 0x1712, "IS", "Radius of Circular Collimator" },
    { 0x0018, 0x1720, "IS", "Vertices of Polygonal Collimator" },
    { 0x0018, 0x1800, "CS", "Acquisition Time Synchronized" },
    { 0x0018, 0x1801, "SH", "Time Source" },
    { 0x0018, 0x1802, "CS", "Time Distribution Protocol" },
    { 0x0018, 0x4000, "LT", "Acquisition Comments" },
    { 0x0018, 0x5000, "SH", "Output Power" },
    { 0x0018, 0x5010, "LO", "Transducer Data" },
    { 0x0018, 0x5012, "DS", "Focus Depth" },
    { 0x0018, 0x5020, "LO", "Processing Function" },
    { 0x0018, 0x5021, "LO", "Postprocessing Function" },
    { 0x0018, 0x5022, "DS", "Mechanical Index" },
    { 0x0018, 0x5024, "DS", "Thermal Index" },
    { 0x0018, 0x5026, "DS", "Cranial Thermal Index" },
    { 0x0018, 0x5027, "DS", "Soft Tissue Thermal Index" },
    { 0x0018, 0x5028, "DS", "Soft Tissue-Focus Thermal Index" },
    { 0x0018, 0x5029, "DS", "Soft Tissue-Surface Thermal Index" },
    { 0x0018, 0x5030, "DS", "Dynamic Range" },
    { 0x0018, 0x5040, "DS", "Total Gain" },
    { 0x0018, 0x5050, "IS", "Depth of Scan Field" },
    { 0x0018, 0x5100, "CS", "Patient Position" },
    { 0x0018, 0x5101, "CS", "View Position" },
    { 0x0018, 0x5104, "SQ", "Projection Eponymous Name Code Sequence" },
    { 0x0018, 0x5210, "DS", "Image Transformation Matrix" },
    { 0x0018, 0x5212, "DS", "Image Translation Vector" },
    { 0x0018, 0x6000, "DS", "Sensitivity" },
    { 0x0018, 0x6011, "IS", "Sequence of Ultrasound Regions" },
    { 0x0018, 0x6012, "US", "Region Spatial Format" },
    { 0x0018, 0x6014, "US", "Region Data Type" },
    { 0x0018, 0x6016, "UL", "Region Flags" },
    { 0x0018, 0x6018, "UL", "Region Location Min X0" },
    { 0x0018, 0x601a, "UL", "Region Location Min Y0" },
    { 0x0018, 0x601c, "UL", "Region Location Max X1" },
    { 0x0018, 0x601e, "UL", "Region Location Max Y1" },
    { 0x0018, 0x6020, "SL", "Reference Pixel X0" },
    { 0x0018, 0x6022, "SL", "Reference Pixel Y0" },
    { 0x0018, 0x6024, "US", "Physical Units X Direction" },
    { 0x0018, 0x6026, "US", "Physical Units Y Direction" },
    { 0x0018, 0x6028, "FD", "Reference Pixel Physical Value X" },
    { 0x0018, 0x602a, "US", "Reference Pixel Physical Value Y" },
    { 0x0018, 0x602c, "US", "Physical Delta X" },
    { 0x0018, 0x602e, "US", "Physical Delta Y" },
    { 0x0018, 0x6030, "UL", "Transducer Frequency" },
    { 0x0018, 0x6031, "CS", "Transducer Type" },
    { 0x0018, 0x6032, "UL", "Pulse Repetition Frequency" },
    { 0x0018, 0x6034, "FD", "Doppler Correction Angle" },
    { 0x0018, 0x6036, "FD", "Steering Angle" },
    { 0x0018, 0x6038, "UL", "Doppler Sample Volume X Position" },
    { 0x0018, 0x603a, "UL", "Doppler Sample Volume Y Position" },
    { 0x0018, 0x603c, "UL", "TM-Line Position X0" },
    { 0x0018, 0x603e, "UL", "TM-Line Position Y0" },
    { 0x0018, 0x6040, "UL", "TM-Line Position X1" },
    { 0x0018, 0x6042, "UL", "TM-Line Position Y1" },
    { 0x0018, 0x6044, "US", "Pixel Component Organization" },
    { 0x0018, 0x6046, "UL", "Pixel Component Mask" },
    { 0x0018, 0x6048, "UL", "Pixel Component Range Start" },
    { 0x0018, 0x604a, "UL", "Pixel Component Range Stop" },
    { 0x0018, 0x604c, "US", "Pixel Component Physical Units" },
    { 0x0018, 0x604e, "US", "Pixel Component Data Type" },
    { 0x0018, 0x6050, "UL", "Number of Table Break Points" },
    { 0x0018, 0x6052, "UL", "Table of X Break Points" },
    { 0x0018, 0x6054, "FD", "Table of Y Break Points" },
    { 0x0018, 0x6056, "UL", "Number of Table Entries" },
    { 0x0018, 0x6058, "UL", "Table of Pixel Values" },
    { 0x0018, 0x605a, "FL", "Table of Parameter Values" },
    { 0x0018, 0x7000, "CS", "Detector Conditions Nominal Flag" },
    { 0x0018, 0x7001, "DS", "Detector Temperature" },
    { 0x0018, 0x7004, "CS", "Detector Type" },
    { 0x0018, 0x7005, "CS", "Detector Configuration" },
    { 0x0018, 0x7006, "LT", "Detector Description" },
    { 0x0018, 0x7008, "LT", "Detector Mode" },
    { 0x0018, 0x700a, "SH", "Detector ID" },
    { 0x0018, 0x700c, "DA", "Date of Last Detector Calibration " },
    { 0x0018, 0x700e, "TM", "Time of Last Detector Calibration" },
    { 0x0018, 0x7010, "IS", "Exposures on Detector Since Last Calibration" },
    { 0x0018, 0x7011, "IS", "Exposures on Detector Since Manufactured" },
    { 0x0018, 0x7012, "DS", "Detector Time Since Last Exposure" },
    { 0x0018, 0x7014, "DS", "Detector Active Time" },
    { 0x0018, 0x7016, "DS", "Detector Activation Offset From Exposure" },
    { 0x0018, 0x701a, "DS", "Detector Binning" },
    { 0x0018, 0x7020, "DS", "Detector Element Physical Size" },
    { 0x0018, 0x7022, "DS", "Detector Element Spacing" },
    { 0x0018, 0x7024, "CS", "Detector Active Shape" },
    { 0x0018, 0x7026, "DS", "Detector Active Dimensions" },
    { 0x0018, 0x7028, "DS", "Detector Active Origin" },
    { 0x0018, 0x7030, "DS", "Field of View Origin" },
    { 0x0018, 0x7032, "DS", "Field of View Rotation" },
    { 0x0018, 0x7034, "CS", "Field of View Horizontal Flip" },
    { 0x0018, 0x7040, "LT", "Grid Absorbing Material" },
    { 0x0018, 0x7041, "LT", "Grid Spacing Material" },
    { 0x0018, 0x7042, "DS", "Grid Thickness" },
    { 0x0018, 0x7044, "DS", "Grid Pitch" },
    { 0x0018, 0x7046, "IS", "Grid Aspect Ratio" },
    { 0x0018, 0x7048, "DS", "Grid Period" },
    { 0x0018, 0x704c, "DS", "Grid Focal Distance" },
    { 0x0018, 0x7050, "LT", "Filter Material" },
    { 0x0018, 0x7052, "DS", "Filter Thickness Minimum" },
    { 0x0018, 0x7054, "DS", "Filter Thickness Maximum" },
    { 0x0018, 0x7060, "CS", "Exposure Control Mode" },
    { 0x0018, 0x7062, "LT", "Exposure Control Mode Description" },
    { 0x0018, 0x7064, "CS", "Exposure Status" },
    { 0x0018, 0x7065, "DS", "Phototimer Setting" },
    { 0x0019, 0x0000, "xs", "?" },
    { 0x0019, 0x0001, "xs", "?" },
    { 0x0019, 0x0002, "xs", "?" },
    { 0x0019, 0x0003, "xs", "?" },
    { 0x0019, 0x0004, "xs", "?" },
    { 0x0019, 0x0005, "xs", "?" },
    { 0x0019, 0x0006, "xs", "?" },
    { 0x0019, 0x0007, "xs", "?" },
    { 0x0019, 0x0008, "xs", "?" },
    { 0x0019, 0x0009, "xs", "?" },
    { 0x0019, 0x000a, "xs", "?" },
    { 0x0019, 0x000b, "DS", "?" },
    { 0x0019, 0x000c, "US", "?" },
    { 0x0019, 0x000d, "TM", "Time" },
    { 0x0019, 0x000e, "xs", "?" },
    { 0x0019, 0x000f, "DS", "Horizontal Frame Of Reference" },
    { 0x0019, 0x0010, "xs", "?" },
    { 0x0019, 0x0011, "xs", "?" },
    { 0x0019, 0x0012, "xs", "?" },
    { 0x0019, 0x0013, "xs", "?" },
    { 0x0019, 0x0014, "xs", "?" },
    { 0x0019, 0x0015, "xs", "?" },
    { 0x0019, 0x0016, "xs", "?" },
    { 0x0019, 0x0017, "xs", "?" },
    { 0x0019, 0x0018, "xs", "?" },
    { 0x0019, 0x0019, "xs", "?" },
    { 0x0019, 0x001a, "xs", "?" },
    { 0x0019, 0x001b, "xs", "?" },
    { 0x0019, 0x001c, "CS", "Dose" },
    { 0x0019, 0x001d, "IS", "Side Mark" },
    { 0x0019, 0x001e, "xs", "?" },
    { 0x0019, 0x001f, "DS", "Exposure Duration" },
    { 0x0019, 0x0020, "xs", "?" },
    { 0x0019, 0x0021, "xs", "?" },
    { 0x0019, 0x0022, "xs", "?" },
    { 0x0019, 0x0023, "xs", "?" },
    { 0x0019, 0x0024, "xs", "?" },
    { 0x0019, 0x0025, "xs", "?" },
    { 0x0019, 0x0026, "xs", "?" },
    { 0x0019, 0x0027, "xs", "?" },
    { 0x0019, 0x0028, "xs", "?" },
    { 0x0019, 0x0029, "IS", "?" },
    { 0x0019, 0x002a, "xs", "?" },
    { 0x0019, 0x002b, "DS", "Xray Off Position" },
    { 0x0019, 0x002c, "xs", "?" },
    { 0x0019, 0x002d, "US", "?" },
    { 0x0019, 0x002e, "xs", "?" },
    { 0x0019, 0x002f, "DS", "Trigger Frequency" },
    { 0x0019, 0x0030, "xs", "?" },
    { 0x0019, 0x0031, "xs", "?" },
    { 0x0019, 0x0032, "xs", "?" },
    { 0x0019, 0x0033, "UN", "ECG 2 Offset 2" },
    { 0x0019, 0x0034, "US", "?" },
    { 0x0019, 0x0036, "US", "?" },
    { 0x0019, 0x0038, "US", "?" },
    { 0x0019, 0x0039, "xs", "?" },
    { 0x0019, 0x003a, "xs", "?" },
    { 0x0019, 0x003b, "LT", "?" },
    { 0x0019, 0x003c, "xs", "?" },
    { 0x0019, 0x003e, "xs", "?" },
    { 0x0019, 0x003f, "UN", "?" },
    { 0x0019, 0x0040, "xs", "?" },
    { 0x0019, 0x0041, "xs", "?" },
    { 0x0019, 0x0042, "xs", "?" },
    { 0x0019, 0x0043, "xs", "?" },
    { 0x0019, 0x0044, "xs", "?" },
    { 0x0019, 0x0045, "xs", "?" },
    { 0x0019, 0x0046, "xs", "?" },
    { 0x0019, 0x0047, "xs", "?" },
    { 0x0019, 0x0048, "xs", "?" },
    { 0x0019, 0x0049, "US", "?" },
    { 0x0019, 0x004a, "xs", "?" },
    { 0x0019, 0x004b, "SL", "Data Size For Scan Data" },
    { 0x0019, 0x004c, "US", "?" },
    { 0x0019, 0x004e, "US", "?" },
    { 0x0019, 0x0050, "xs", "?" },
    { 0x0019, 0x0051, "xs", "?" },
    { 0x0019, 0x0052, "xs", "?" },
    { 0x0019, 0x0053, "LT", "Barcode" },
    { 0x0019, 0x0054, "xs", "?" },
    { 0x0019, 0x0055, "DS", "Receiver Reference Gain" },
    { 0x0019, 0x0056, "xs", "?" },
    { 0x0019, 0x0057, "SS", "CT Water Number" },
    { 0x0019, 0x0058, "xs", "?" },
    { 0x0019, 0x005a, "xs", "?" },
    { 0x0019, 0x005c, "xs", "?" },
    { 0x0019, 0x005d, "US", "?" },
    { 0x0019, 0x005e, "xs", "?" },
    { 0x0019, 0x005f, "SL", "Increment Between Channels" },
    { 0x0019, 0x0060, "xs", "?" },
    { 0x0019, 0x0061, "xs", "?" },
    { 0x0019, 0x0062, "xs", "?" },
    { 0x0019, 0x0063, "xs", "?" },
    { 0x0019, 0x0064, "xs", "?" },
    { 0x0019, 0x0065, "xs", "?" },
    { 0x0019, 0x0066, "xs", "?" },
    { 0x0019, 0x0067, "xs", "?" },
    { 0x0019, 0x0068, "xs", "?" },
    { 0x0019, 0x0069, "UL", "Convolution Mode" },
    { 0x0019, 0x006a, "xs", "?" },
    { 0x0019, 0x006b, "SS", "Field Of View In Detector Cells" },
    { 0x0019, 0x006c, "US", "?" },
    { 0x0019, 0x006e, "US", "?" },
    { 0x0019, 0x0070, "xs", "?" },
    { 0x0019, 0x0071, "xs", "?" },
    { 0x0019, 0x0072, "xs", "?" },
    { 0x0019, 0x0073, "xs", "?" },
    { 0x0019, 0x0074, "xs", "?" },
    { 0x0019, 0x0075, "xs", "?" },
    { 0x0019, 0x0076, "xs", "?" },
    { 0x0019, 0x0077, "US", "?" },
    { 0x0019, 0x0078, "US", "?" },
    { 0x0019, 0x007a, "US", "?" },
    { 0x0019, 0x007c, "US", "?" },
    { 0x0019, 0x007d, "DS", "Second Echo" },
    { 0x0019, 0x007e, "xs", "?" },
    { 0x0019, 0x007f, "DS", "Table Delta" },
    { 0x0019, 0x0080, "xs", "?" },
    { 0x0019, 0x0081, "xs", "?" },
    { 0x0019, 0x0082, "xs", "?" },
    { 0x0019, 0x0083, "xs", "?" },
    { 0x0019, 0x0084, "xs", "?" },
    { 0x0019, 0x0085, "xs", "?" },
    { 0x0019, 0x0086, "xs", "?" },
    { 0x0019, 0x0087, "xs", "?" },
    { 0x0019, 0x0088, "xs", "?" },
    { 0x0019, 0x008a, "xs", "?" },
    { 0x0019, 0x008b, "SS", "Actual Receive Gain Digital" },
    { 0x0019, 0x008c, "US", "?" },
    { 0x0019, 0x008d, "DS", "Delay After Trigger" },
    { 0x0019, 0x008e, "US", "?" },
    { 0x0019, 0x008f, "SS", "Swap Phase Frequency" },
    { 0x0019, 0x0090, "xs", "?" },
    { 0x0019, 0x0091, "xs", "?" },
    { 0x0019, 0x0092, "xs", "?" },
    { 0x0019, 0x0093, "xs", "?" },
    { 0x0019, 0x0094, "xs", "?" },
    { 0x0019, 0x0095, "SS", "Analog Receiver Gain" },
    { 0x0019, 0x0096, "xs", "?" },
    { 0x0019, 0x0097, "xs", "?" },
    { 0x0019, 0x0098, "xs", "?" },
    { 0x0019, 0x0099, "US", "?" },
    { 0x0019, 0x009a, "US", "?" },
    { 0x0019, 0x009b, "SS", "Pulse Sequence Mode" },
    { 0x0019, 0x009c, "xs", "?" },
    { 0x0019, 0x009d, "DT", "Pulse Sequence Date" },
    { 0x0019, 0x009e, "xs", "?" },
    { 0x0019, 0x009f, "xs", "?" },
    { 0x0019, 0x00a0, "xs", "?" },
    { 0x0019, 0x00a1, "xs", "?" },
    { 0x0019, 0x00a2, "xs", "?" },
    { 0x0019, 0x00a3, "xs", "?" },
    { 0x0019, 0x00a4, "xs", "?" },
    { 0x0019, 0x00a5, "xs", "?" },
    { 0x0019, 0x00a6, "xs", "?" },
    { 0x0019, 0x00a7, "xs", "?" },
    { 0x0019, 0x00a8, "xs", "?" },
    { 0x0019, 0x00a9, "xs", "?" },
    { 0x0019, 0x00aa, "xs", "?" },
    { 0x0019, 0x00ab, "xs", "?" },
    { 0x0019, 0x00ac, "xs", "?" },
    { 0x0019, 0x00ad, "xs", "?" },
    { 0x0019, 0x00ae, "xs", "?" },
    { 0x0019, 0x00af, "xs", "?" },
    { 0x0019, 0x00b0, "xs", "?" },
    { 0x0019, 0x00b1, "xs", "?" },
    { 0x0019, 0x00b2, "xs", "?" },
    { 0x0019, 0x00b3, "xs", "?" },
    { 0x0019, 0x00b4, "xs", "?" },
    { 0x0019, 0x00b5, "xs", "?" },
    { 0x0019, 0x00b6, "DS", "User Data" },
    { 0x0019, 0x00b7, "DS", "User Data" },
    { 0x0019, 0x00b8, "DS", "User Data" },
    { 0x0019, 0x00b9, "DS", "User Data" },
    { 0x0019, 0x00ba, "DS", "User Data" },
    { 0x0019, 0x00bb, "DS", "User Data" },
    { 0x0019, 0x00bc, "DS", "User Data" },
    { 0x0019, 0x00bd, "DS", "User Data" },
    { 0x0019, 0x00be, "DS", "Projection Angle" },
    { 0x0019, 0x00c0, "xs", "?" },
    { 0x0019, 0x00c1, "xs", "?" },
    { 0x0019, 0x00c2, "xs", "?" },
    { 0x0019, 0x00c3, "xs", "?" },
    { 0x0019, 0x00c4, "xs", "?" },
    { 0x0019, 0x00c5, "xs", "?" },
    { 0x0019, 0x00c6, "SS", "SAT Location H" },
    { 0x0019, 0x00c7, "SS", "SAT Location F" },
    { 0x0019, 0x00c8, "SS", "SAT Thickness R L" },
    { 0x0019, 0x00c9, "SS", "SAT Thickness A P" },
    { 0x0019, 0x00ca, "SS", "SAT Thickness H F" },
    { 0x0019, 0x00cb, "xs", "?" },
    { 0x0019, 0x00cc, "xs", "?" },
    { 0x0019, 0x00cd, "SS", "Thickness Disclaimer" },
    { 0x0019, 0x00ce, "SS", "Prescan Type" },
    { 0x0019, 0x00cf, "SS", "Prescan Status" },
    { 0x0019, 0x00d0, "SH", "Raw Data Type" },
    { 0x0019, 0x00d1, "DS", "Flow Sensitivity" },
    { 0x0019, 0x00d2, "xs", "?" },
    { 0x0019, 0x00d3, "xs", "?" },
    { 0x0019, 0x00d4, "xs", "?" },
    { 0x0019, 0x00d5, "xs", "?" },
    { 0x0019, 0x00d6, "xs", "?" },
    { 0x0019, 0x00d7, "xs", "?" },
    { 0x0019, 0x00d8, "xs", "?" },
    { 0x0019, 0x00d9, "xs", "?" },
    { 0x0019, 0x00da, "xs", "?" },
    { 0x0019, 0x00db, "DS", "Back Projector Coefficient" },
    { 0x0019, 0x00dc, "SS", "Primary Speed Correction Used" },
    { 0x0019, 0x00dd, "SS", "Overrange Correction Used" },
    { 0x0019, 0x00de, "DS", "Dynamic Z Alpha Value" },
    { 0x0019, 0x00df, "DS", "User Data" },
    { 0x0019, 0x00e0, "DS", "User Data" },
    { 0x0019, 0x00e1, "xs", "?" },
    { 0x0019, 0x00e2, "xs", "?" },
    { 0x0019, 0x00e3, "xs", "?" },
    { 0x0019, 0x00e4, "LT", "?" },
    { 0x0019, 0x00e5, "IS", "?" },
    { 0x0019, 0x00e6, "US", "?" },
    { 0x0019, 0x00e8, "DS", "?" },
    { 0x0019, 0x00e9, "DS", "?" },
    { 0x0019, 0x00eb, "DS", "?" },
    { 0x0019, 0x00ec, "US", "?" },
    { 0x0019, 0x00f0, "xs", "?" },
    { 0x0019, 0x00f1, "xs", "?" },
    { 0x0019, 0x00f2, "xs", "?" },
    { 0x0019, 0x00f3, "xs", "?" },
    { 0x0019, 0x00f4, "LT", "?" },
    { 0x0019, 0x00f9, "DS", "Transmission Gain" },
    { 0x0019, 0x1015, "UN", "?" },
    { 0x0020, 0x0000, "UL", "Relationship Group Length" },
    { 0x0020, 0x000d, "UI", "Study Instance UID" },
    { 0x0020, 0x000e, "UI", "Series Instance UID" },
    { 0x0020, 0x0010, "SH", "Study ID" },
    { 0x0020, 0x0011, "IS", "Series Number" },
    { 0x0020, 0x0012, "IS", "Acquisition Number" },
    { 0x0020, 0x0013, "IS", "Instance (formerly Image) Number" },
    { 0x0020, 0x0014, "IS", "Isotope Number" },
    { 0x0020, 0x0015, "IS", "Phase Number" },
    { 0x0020, 0x0016, "IS", "Interval Number" },
    { 0x0020, 0x0017, "IS", "Time Slot Number" },
    { 0x0020, 0x0018, "IS", "Angle Number" },
    { 0x0020, 0x0020, "CS", "Patient Orientation" },
    { 0x0020, 0x0022, "IS", "Overlay Number" },
    { 0x0020, 0x0024, "IS", "Curve Number" },
    { 0x0020, 0x0026, "IS", "LUT Number" },
    { 0x0020, 0x0030, "DS", "Image Position" },
    { 0x0020, 0x0032, "DS", "Image Position (Patient)" },
    { 0x0020, 0x0035, "DS", "Image Orientation" },
    { 0x0020, 0x0037, "DS", "Image Orientation (Patient)" },
    { 0x0020, 0x0050, "DS", "Location" },
    { 0x0020, 0x0052, "UI", "Frame of Reference UID" },
    { 0x0020, 0x0060, "CS", "Laterality" },
    { 0x0020, 0x0062, "CS", "Image Laterality" },
    { 0x0020, 0x0070, "LT", "Image Geometry Type" },
    { 0x0020, 0x0080, "LO", "Masking Image" },
    { 0x0020, 0x0100, "IS", "Temporal Position Identifier" },
    { 0x0020, 0x0105, "IS", "Number of Temporal Positions" },
    { 0x0020, 0x0110, "DS", "Temporal Resolution" },
    { 0x0020, 0x1000, "IS", "Series in Study" },
    { 0x0020, 0x1001, "DS", "Acquisitions in Series" },
    { 0x0020, 0x1002, "IS", "Images in Acquisition" },
    { 0x0020, 0x1003, "IS", "Images in Series" },
    { 0x0020, 0x1004, "IS", "Acquisitions in Study" },
    { 0x0020, 0x1005, "IS", "Images in Study" },
    { 0x0020, 0x1020, "LO", "Reference" },
    { 0x0020, 0x1040, "LO", "Position Reference Indicator" },
    { 0x0020, 0x1041, "DS", "Slice Location" },
    { 0x0020, 0x1070, "IS", "Other Study Numbers" },
    { 0x0020, 0x1200, "IS", "Number of Patient Related Studies" },
    { 0x0020, 0x1202, "IS", "Number of Patient Related Series" },
    { 0x0020, 0x1204, "IS", "Number of Patient Related Images" },
    { 0x0020, 0x1206, "IS", "Number of Study Related Series" },
    { 0x0020, 0x1208, "IS", "Number of Study Related Series" },
    { 0x0020, 0x3100, "LO", "Source Image IDs" },
    { 0x0020, 0x3401, "LO", "Modifying Device ID" },
    { 0x0020, 0x3402, "LO", "Modified Image ID" },
    { 0x0020, 0x3403, "xs", "Modified Image Date" },
    { 0x0020, 0x3404, "LO", "Modifying Device Manufacturer" },
    { 0x0020, 0x3405, "xs", "Modified Image Time" },
    { 0x0020, 0x3406, "xs", "Modified Image Description" },
    { 0x0020, 0x4000, "LT", "Image Comments" },
    { 0x0020, 0x5000, "AT", "Original Image Identification" },
    { 0x0020, 0x5002, "LO", "Original Image Identification Nomenclature" },
    { 0x0021, 0x0000, "xs", "?" },
    { 0x0021, 0x0001, "xs", "?" },
    { 0x0021, 0x0002, "xs", "?" },
    { 0x0021, 0x0003, "xs", "?" },
    { 0x0021, 0x0004, "DS", "VOI Position" },
    { 0x0021, 0x0005, "xs", "?" },
    { 0x0021, 0x0006, "IS", "CSI Matrix Size Original" },
    { 0x0021, 0x0007, "xs", "?" },
    { 0x0021, 0x0008, "DS", "Spatial Grid Shift" },
    { 0x0021, 0x0009, "DS", "Signal Limits Minimum" },
    { 0x0021, 0x0010, "xs", "?" },
    { 0x0021, 0x0011, "xs", "?" },
    { 0x0021, 0x0012, "xs", "?" },
    { 0x0021, 0x0013, "xs", "?" },
    { 0x0021, 0x0014, "xs", "?" },
    { 0x0021, 0x0015, "xs", "?" },
    { 0x0021, 0x0016, "xs", "?" },
    { 0x0021, 0x0017, "DS", "EPI Operation Mode Flag" },
    { 0x0021, 0x0018, "xs", "?" },
    { 0x0021, 0x0019, "xs", "?" },
    { 0x0021, 0x0020, "xs", "?" },
    { 0x0021, 0x0021, "xs", "?" },
    { 0x0021, 0x0022, "xs", "?" },
    { 0x0021, 0x0024, "xs", "?" },
    { 0x0021, 0x0025, "US", "?" },
    { 0x0021, 0x0026, "IS", "Image Pixel Offset" },
    { 0x0021, 0x0030, "xs", "?" },
    { 0x0021, 0x0031, "xs", "?" },
    { 0x0021, 0x0032, "xs", "?" },
    { 0x0021, 0x0034, "xs", "?" },
    { 0x0021, 0x0035, "SS", "Series From Which Prescribed" },
    { 0x0021, 0x0036, "xs", "?" },
    { 0x0021, 0x0037, "SS", "Screen Format" },
    { 0x0021, 0x0039, "DS", "Slab Thickness" },
    { 0x0021, 0x0040, "xs", "?" },
    { 0x0021, 0x0041, "xs", "?" },
    { 0x0021, 0x0042, "xs", "?" },
    { 0x0021, 0x0043, "xs", "?" },
    { 0x0021, 0x0044, "xs", "?" },
    { 0x0021, 0x0045, "xs", "?" },
    { 0x0021, 0x0046, "xs", "?" },
    { 0x0021, 0x0047, "xs", "?" },
    { 0x0021, 0x0048, "xs", "?" },
    { 0x0021, 0x0049, "xs", "?" },
    { 0x0021, 0x004a, "xs", "?" },
    { 0x0021, 0x004e, "US", "?" },
    { 0x0021, 0x004f, "xs", "?" },
    { 0x0021, 0x0050, "xs", "?" },
    { 0x0021, 0x0051, "xs", "?" },
    { 0x0021, 0x0052, "xs", "?" },
    { 0x0021, 0x0053, "xs", "?" },
    { 0x0021, 0x0054, "xs", "?" },
    { 0x0021, 0x0055, "xs", "?" },
    { 0x0021, 0x0056, "xs", "?" },
    { 0x0021, 0x0057, "xs", "?" },
    { 0x0021, 0x0058, "xs", "?" },
    { 0x0021, 0x0059, "xs", "?" },
    { 0x0021, 0x005a, "SL", "Integer Slop" },
    { 0x0021, 0x005b, "DS", "Float Slop" },
    { 0x0021, 0x005c, "DS", "Float Slop" },
    { 0x0021, 0x005d, "DS", "Float Slop" },
    { 0x0021, 0x005e, "DS", "Float Slop" },
    { 0x0021, 0x005f, "DS", "Float Slop" },
    { 0x0021, 0x0060, "xs", "?" },
    { 0x0021, 0x0061, "DS", "Image Normal" },
    { 0x0021, 0x0062, "IS", "Reference Type Code" },
    { 0x0021, 0x0063, "DS", "Image Distance" },
    { 0x0021, 0x0065, "US", "Image Positioning History Mask" },
    { 0x0021, 0x006a, "DS", "Image Row" },
    { 0x0021, 0x006b, "DS", "Image Column" },
    { 0x0021, 0x0070, "xs", "?" },
    { 0x0021, 0x0071, "xs", "?" },
    { 0x0021, 0x0072, "xs", "?" },
    { 0x0021, 0x0073, "DS", "Second Repetition Time" },
    { 0x0021, 0x0075, "DS", "Light Brightness" },
    { 0x0021, 0x0076, "DS", "Light Contrast" },
    { 0x0021, 0x007a, "IS", "Overlay Threshold" },
    { 0x0021, 0x007b, "IS", "Surface Threshold" },
    { 0x0021, 0x007c, "IS", "Grey Scale Threshold" },
    { 0x0021, 0x0080, "xs", "?" },
    { 0x0021, 0x0081, "DS", "Auto Window Level Alpha" },
    { 0x0021, 0x0082, "xs", "?" },
    { 0x0021, 0x0083, "DS", "Auto Window Level Window" },
    { 0x0021, 0x0084, "DS", "Auto Window Level Level" },
    { 0x0021, 0x0090, "xs", "?" },
    { 0x0021, 0x0091, "xs", "?" },
    { 0x0021, 0x0092, "xs", "?" },
    { 0x0021, 0x0093, "xs", "?" },
    { 0x0021, 0x0094, "DS", "EPI Change Value of X Component" },
    { 0x0021, 0x0095, "DS", "EPI Change Value of Y Component" },
    { 0x0021, 0x0096, "DS", "EPI Change Value of Z Component" },
    { 0x0021, 0x00a0, "xs", "?" },
    { 0x0021, 0x00a1, "DS", "?" },
    { 0x0021, 0x00a2, "xs", "?" },
    { 0x0021, 0x00a3, "LT", "?" },
    { 0x0021, 0x00a4, "LT", "?" },
    { 0x0021, 0x00a7, "LT", "?" },
    { 0x0021, 0x00b0, "IS", "?" },
    { 0x0021, 0x00c0, "IS", "?" },
    { 0x0023, 0x0000, "xs", "?" },
    { 0x0023, 0x0001, "SL", "Number Of Series In Study" },
    { 0x0023, 0x0002, "SL", "Number Of Unarchived Series" },
    { 0x0023, 0x0010, "xs", "?" },
    { 0x0023, 0x0020, "xs", "?" },
    { 0x0023, 0x0030, "xs", "?" },
    { 0x0023, 0x0040, "xs", "?" },
    { 0x0023, 0x0050, "xs", "?" },
    { 0x0023, 0x0060, "xs", "?" },
    { 0x0023, 0x0070, "xs", "?" },
    { 0x0023, 0x0074, "SL", "Number Of Updates To Info" },
    { 0x0023, 0x007d, "SS", "Indicates If Study Has Complete Info" },
    { 0x0023, 0x0080, "xs", "?" },
    { 0x0023, 0x0090, "xs", "?" },
    { 0x0023, 0x00ff, "US", "?" },
    { 0x0025, 0x0000, "UL", "Group Length" },
    { 0x0025, 0x0006, "SS", "Last Pulse Sequence Used" },
    { 0x0025, 0x0007, "SL", "Images In Series" },
    { 0x0025, 0x0010, "SS", "Landmark Counter" },
    { 0x0025, 0x0011, "SS", "Number Of Acquisitions" },
    { 0x0025, 0x0014, "SL", "Indicates Number Of Updates To Info" },
    { 0x0025, 0x0017, "SL", "Series Complete Flag" },
    { 0x0025, 0x0018, "SL", "Number Of Images Archived" },
    { 0x0025, 0x0019, "SL", "Last Image Number Used" },
    { 0x0025, 0x001a, "SH", "Primary Receiver Suite And Host" },
    { 0x0027, 0x0000, "US", "?" },
    { 0x0027, 0x0006, "SL", "Image Archive Flag" },
    { 0x0027, 0x0010, "SS", "Scout Type" },
    { 0x0027, 0x0011, "UN", "?" },
    { 0x0027, 0x0012, "IS", "?" },
    { 0x0027, 0x0013, "IS", "?" },
    { 0x0027, 0x0014, "IS", "?" },
    { 0x0027, 0x0015, "IS", "?" },
    { 0x0027, 0x0016, "LT", "?" },
    { 0x0027, 0x001c, "SL", "Vma Mamp" },
    { 0x0027, 0x001d, "SS", "Vma Phase" },
    { 0x0027, 0x001e, "SL", "Vma Mod" },
    { 0x0027, 0x001f, "SL", "Vma Clip" },
    { 0x0027, 0x0020, "SS", "Smart Scan On Off Flag" },
    { 0x0027, 0x0030, "SH", "Foreign Image Revision" },
    { 0x0027, 0x0031, "SS", "Imaging Mode" },
    { 0x0027, 0x0032, "SS", "Pulse Sequence" },
    { 0x0027, 0x0033, "SL", "Imaging Options" },
    { 0x0027, 0x0035, "SS", "Plane Type" },
    { 0x0027, 0x0036, "SL", "Oblique Plane" },
    { 0x0027, 0x0040, "SH", "RAS Letter Of Image Location" },
    { 0x0027, 0x0041, "FL", "Image Location" },
    { 0x0027, 0x0042, "FL", "Center R Coord Of Plane Image" },
    { 0x0027, 0x0043, "FL", "Center A Coord Of Plane Image" },
    { 0x0027, 0x0044, "FL", "Center S Coord Of Plane Image" },
    { 0x0027, 0x0045, "FL", "Normal R Coord" },
    { 0x0027, 0x0046, "FL", "Normal A Coord" },
    { 0x0027, 0x0047, "FL", "Normal S Coord" },
    { 0x0027, 0x0048, "FL", "R Coord Of Top Right Corner" },
    { 0x0027, 0x0049, "FL", "A Coord Of Top Right Corner" },
    { 0x0027, 0x004a, "FL", "S Coord Of Top Right Corner" },
    { 0x0027, 0x004b, "FL", "R Coord Of Bottom Right Corner" },
    { 0x0027, 0x004c, "FL", "A Coord Of Bottom Right Corner" },
    { 0x0027, 0x004d, "FL", "S Coord Of Bottom Right Corner" },
    { 0x0027, 0x0050, "FL", "Table Start Location" },
    { 0x0027, 0x0051, "FL", "Table End Location" },
    { 0x0027, 0x0052, "SH", "RAS Letter For Side Of Image" },
    { 0x0027, 0x0053, "SH", "RAS Letter For Anterior Posterior" },
    { 0x0027, 0x0054, "SH", "RAS Letter For Scout Start Loc" },
    { 0x0027, 0x0055, "SH", "RAS Letter For Scout End Loc" },
    { 0x0027, 0x0060, "FL", "Image Dimension X" },
    { 0x0027, 0x0061, "FL", "Image Dimension Y" },
    { 0x0027, 0x0062, "FL", "Number Of Excitations" },
    { 0x0028, 0x0000, "UL", "Image Presentation Group Length" },
    { 0x0028, 0x0002, "US", "Samples per Pixel" },
    { 0x0028, 0x0004, "CS", "Photometric Interpretation" },
    { 0x0028, 0x0005, "US", "Image Dimensions" },
    { 0x0028, 0x0006, "US", "Planar Configuration" },
    { 0x0028, 0x0008, "IS", "Number of Frames" },
    { 0x0028, 0x0009, "AT", "Frame Increment Pointer" },
    { 0x0028, 0x0010, "US", "Rows" },
    { 0x0028, 0x0011, "US", "Columns" },
    { 0x0028, 0x0012, "US", "Planes" },
    { 0x0028, 0x0014, "US", "Ultrasound Color Data Present" },
    { 0x0028, 0x0030, "DS", "Pixel Spacing" },
    { 0x0028, 0x0031, "DS", "Zoom Factor" },
    { 0x0028, 0x0032, "DS", "Zoom Center" },
    { 0x0028, 0x0034, "IS", "Pixel Aspect Ratio" },
    { 0x0028, 0x0040, "LO", "Image Format" },
    { 0x0028, 0x0050, "LT", "Manipulated Image" },
    { 0x0028, 0x0051, "CS", "Corrected Image" },
    { 0x0028, 0x005f, "LO", "Compression Recognition Code" },
    { 0x0028, 0x0060, "LO", "Compression Code" },
    { 0x0028, 0x0061, "SH", "Compression Originator" },
    { 0x0028, 0x0062, "SH", "Compression Label" },
    { 0x0028, 0x0063, "SH", "Compression Description" },
    { 0x0028, 0x0065, "LO", "Compression Sequence" },
    { 0x0028, 0x0066, "AT", "Compression Step Pointers" },
    { 0x0028, 0x0068, "US", "Repeat Interval" },
    { 0x0028, 0x0069, "US", "Bits Grouped" },
    { 0x0028, 0x0070, "US", "Perimeter Table" },
    { 0x0028, 0x0071, "xs", "Perimeter Value" },
    { 0x0028, 0x0080, "US", "Predictor Rows" },
    { 0x0028, 0x0081, "US", "Predictor Columns" },
    { 0x0028, 0x0082, "US", "Predictor Constants" },
    { 0x0028, 0x0090, "LO", "Blocked Pixels" },
    { 0x0028, 0x0091, "US", "Block Rows" },
    { 0x0028, 0x0092, "US", "Block Columns" },
    { 0x0028, 0x0093, "US", "Row Overlap" },
    { 0x0028, 0x0094, "US", "Column Overlap" },
    { 0x0028, 0x0100, "US", "Bits Allocated" },
    { 0x0028, 0x0101, "US", "Bits Stored" },
    { 0x0028, 0x0102, "US", "High Bit" },
    { 0x0028, 0x0103, "US", "Pixel Representation" },
    { 0x0028, 0x0104, "xs", "Smallest Valid Pixel Value" },
    { 0x0028, 0x0105, "xs", "Largest Valid Pixel Value" },
    { 0x0028, 0x0106, "xs", "Smallest Image Pixel Value" },
    { 0x0028, 0x0107, "xs", "Largest Image Pixel Value" },
    { 0x0028, 0x0108, "xs", "Smallest Pixel Value in Series" },
    { 0x0028, 0x0109, "xs", "Largest Pixel Value in Series" },
    { 0x0028, 0x0110, "xs", "Smallest Pixel Value in Plane" },
    { 0x0028, 0x0111, "xs", "Largest Pixel Value in Plane" },
    { 0x0028, 0x0120, "xs", "Pixel Padding Value" },
    { 0x0028, 0x0200, "xs", "Image Location" },
    { 0x0028, 0x0300, "CS", "Quality Control Image" },
    { 0x0028, 0x0301, "CS", "Burned In Annotation" },
    { 0x0028, 0x0400, "xs", "?" },
    { 0x0028, 0x0401, "xs", "?" },
    { 0x0028, 0x0402, "xs", "?" },
    { 0x0028, 0x0403, "xs", "?" },
    { 0x0028, 0x0404, "AT", "Details of Coefficients" },
    { 0x0028, 0x0700, "LO", "DCT Label" },
    { 0x0028, 0x0701, "LO", "Data Block Description" },
    { 0x0028, 0x0702, "AT", "Data Block" },
    { 0x0028, 0x0710, "US", "Normalization Factor Format" },
    { 0x0028, 0x0720, "US", "Zonal Map Number Format" },
    { 0x0028, 0x0721, "AT", "Zonal Map Location" },
    { 0x0028, 0x0722, "US", "Zonal Map Format" },
    { 0x0028, 0x0730, "US", "Adaptive Map Format" },
    { 0x0028, 0x0740, "US", "Code Number Format" },
    { 0x0028, 0x0800, "LO", "Code Label" },
    { 0x0028, 0x0802, "US", "Number of Tables" },
    { 0x0028, 0x0803, "AT", "Code Table Location" },
    { 0x0028, 0x0804, "US", "Bits For Code Word" },
    { 0x0028, 0x0808, "AT", "Image Data Location" },
    { 0x0028, 0x1040, "CS", "Pixel Intensity Relationship" },
    { 0x0028, 0x1041, "SS", "Pixel Intensity Relationship Sign" },
    { 0x0028, 0x1050, "DS", "Window Center" },
    { 0x0028, 0x1051, "DS", "Window Width" },
    { 0x0028, 0x1052, "DS", "Rescale Intercept" },
    { 0x0028, 0x1053, "DS", "Rescale Slope" },
    { 0x0028, 0x1054, "LO", "Rescale Type" },
    { 0x0028, 0x1055, "LO", "Window Center & Width Explanation" },
    { 0x0028, 0x1080, "LO", "Gray Scale" },
    { 0x0028, 0x1090, "CS", "Recommended Viewing Mode" },
    { 0x0028, 0x1100, "xs", "Gray Lookup Table Descriptor" },
    { 0x0028, 0x1101, "xs", "Red Palette Color Lookup Table Descriptor" },
    { 0x0028, 0x1102, "xs", "Green Palette Color Lookup Table Descriptor" },
    { 0x0028, 0x1103, "xs", "Blue Palette Color Lookup Table Descriptor" },
    { 0x0028, 0x1111, "OW", "Large Red Palette Color Lookup Table Descriptor" },
    { 0x0028, 0x1112, "OW", "Large Green Palette Color Lookup Table Descriptor" },
    { 0x0028, 0x1113, "OW", "Large Blue Palette Color Lookup Table Descriptor" },
    { 0x0028, 0x1199, "UI", "Palette Color Lookup Table UID" },
    { 0x0028, 0x1200, "xs", "Gray Lookup Table Data" },
    { 0x0028, 0x1201, "OW", "Red Palette Color Lookup Table Data" },
    { 0x0028, 0x1202, "OW", "Green Palette Color Lookup Table Data" },
    { 0x0028, 0x1203, "OW", "Blue Palette Color Lookup Table Data" },
    { 0x0028, 0x1211, "OW", "Large Red Palette Color Lookup Table Data" },
    { 0x0028, 0x1212, "OW", "Large Green Palette Color Lookup Table Data" },
    { 0x0028, 0x1213, "OW", "Large Blue Palette Color Lookup Table Data" },
    { 0x0028, 0x1214, "UI", "Large Palette Color Lookup Table UID" },
    { 0x0028, 0x1221, "OW", "Segmented Red Palette Color Lookup Table Data" },
    { 0x0028, 0x1222, "OW", "Segmented Green Palette Color Lookup Table Data" },
    { 0x0028, 0x1223, "OW", "Segmented Blue Palette Color Lookup Table Data" },
    { 0x0028, 0x1300, "CS", "Implant Present" },
    { 0x0028, 0x2110, "CS", "Lossy Image Compression" },
    { 0x0028, 0x2112, "DS", "Lossy Image Compression Ratio" },
    { 0x0028, 0x3000, "SQ", "Modality LUT Sequence" },
    { 0x0028, 0x3002, "US", "LUT Descriptor" },
    { 0x0028, 0x3003, "LO", "LUT Explanation" },
    { 0x0028, 0x3004, "LO", "Modality LUT Type" },
    { 0x0028, 0x3006, "US", "LUT Data" },
    { 0x0028, 0x3010, "xs", "VOI LUT Sequence" },
    { 0x0028, 0x4000, "LT", "Image Presentation Comments" },
    { 0x0028, 0x5000, "SQ", "Biplane Acquisition Sequence" },
    { 0x0028, 0x6010, "US", "Representative Frame Number" },
    { 0x0028, 0x6020, "US", "Frame Numbers of Interest" },
    { 0x0028, 0x6022, "LO", "Frame of Interest Description" },
    { 0x0028, 0x6030, "US", "Mask Pointer" },
    { 0x0028, 0x6040, "US", "R Wave Pointer" },
    { 0x0028, 0x6100, "SQ", "Mask Subtraction Sequence" },
    { 0x0028, 0x6101, "CS", "Mask Operation" },
    { 0x0028, 0x6102, "US", "Applicable Frame Range" },
    { 0x0028, 0x6110, "US", "Mask Frame Numbers" },
    { 0x0028, 0x6112, "US", "Contrast Frame Averaging" },
    { 0x0028, 0x6114, "FL", "Mask Sub-Pixel Shift" },
    { 0x0028, 0x6120, "SS", "TID Offset" },
    { 0x0028, 0x6190, "ST", "Mask Operation Explanation" },
    { 0x0029, 0x0000, "xs", "?" },
    { 0x0029, 0x0001, "xs", "?" },
    { 0x0029, 0x0002, "xs", "?" },
    { 0x0029, 0x0003, "xs", "?" },
    { 0x0029, 0x0004, "xs", "?" },
    { 0x0029, 0x0005, "xs", "?" },
    { 0x0029, 0x0006, "xs", "?" },
    { 0x0029, 0x0007, "SL", "Lower Range Of Pixels" },
    { 0x0029, 0x0008, "SH", "Lower Range Of Pixels" },
    { 0x0029, 0x0009, "SH", "Lower Range Of Pixels" },
    { 0x0029, 0x000a, "SS", "Lower Range Of Pixels" },
    { 0x0029, 0x000c, "xs", "?" },
    { 0x0029, 0x000e, "CS", "Zoom Enable Status" },
    { 0x0029, 0x000f, "CS", "Zoom Select Status" },
    { 0x0029, 0x0010, "xs", "?" },
    { 0x0029, 0x0011, "xs", "?" },
    { 0x0029, 0x0013, "LT", "?" },
    { 0x0029, 0x0015, "xs", "?" },
    { 0x0029, 0x0016, "SL", "Lower Range Of Pixels" },
    { 0x0029, 0x0017, "SL", "Lower Range Of Pixels" },
    { 0x0029, 0x0018, "SL", "Upper Range Of Pixels" },
    { 0x0029, 0x001a, "SL", "Length Of Total Info In Bytes" },
    { 0x0029, 0x001e, "xs", "?" },
    { 0x0029, 0x001f, "xs", "?" },
    { 0x0029, 0x0020, "xs", "?" },
    { 0x0029, 0x0022, "IS", "Pixel Quality Value" },
    { 0x0029, 0x0025, "LT", "Processed Pixel Data Quality" },
    { 0x0029, 0x0026, "SS", "Version Of Info Structure" },
    { 0x0029, 0x0030, "xs", "?" },
    { 0x0029, 0x0031, "xs", "?" },
    { 0x0029, 0x0032, "xs", "?" },
    { 0x0029, 0x0033, "xs", "?" },
    { 0x0029, 0x0034, "xs", "?" },
    { 0x0029, 0x0035, "SL", "Advantage Comp Underflow" },
    { 0x0029, 0x0038, "US", "?" },
    { 0x0029, 0x0040, "xs", "?" },
    { 0x0029, 0x0041, "DS", "Magnifying Glass Rectangle" },
    { 0x0029, 0x0043, "DS", "Magnifying Glass Factor" },
    { 0x0029, 0x0044, "US", "Magnifying Glass Function" },
    { 0x0029, 0x004e, "CS", "Magnifying Glass Enable Status" },
    { 0x0029, 0x004f, "CS", "Magnifying Glass Select Status" },
    { 0x0029, 0x0050, "xs", "?" },
    { 0x0029, 0x0051, "LT", "Exposure Code" },
    { 0x0029, 0x0052, "LT", "Sort Code" },
    { 0x0029, 0x0053, "LT", "?" },
    { 0x0029, 0x0060, "xs", "?" },
    { 0x0029, 0x0061, "xs", "?" },
    { 0x0029, 0x0067, "LT", "?" },
    { 0x0029, 0x0070, "xs", "?" },
    { 0x0029, 0x0071, "xs", "?" },
    { 0x0029, 0x0072, "xs", "?" },
    { 0x0029, 0x0077, "CS", "Window Select Status" },
    { 0x0029, 0x0078, "LT", "ECG Display Printing ID" },
    { 0x0029, 0x0079, "CS", "ECG Display Printing" },
    { 0x0029, 0x007e, "CS", "ECG Display Printing Enable Status" },
    { 0x0029, 0x007f, "CS", "ECG Display Printing Select Status" },
    { 0x0029, 0x0080, "xs", "?" },
    { 0x0029, 0x0081, "xs", "?" },
    { 0x0029, 0x0082, "IS", "View Zoom" },
    { 0x0029, 0x0083, "IS", "View Transform" },
    { 0x0029, 0x008e, "CS", "Physiological Display Enable Status" },
    { 0x0029, 0x008f, "CS", "Physiological Display Select Status" },
    { 0x0029, 0x0090, "IS", "?" },
    { 0x0029, 0x0099, "LT", "Shutter Type" },
    { 0x0029, 0x00a0, "US", "Rows of Rectangular Shutter" },
    { 0x0029, 0x00a1, "US", "Columns of Rectangular Shutter" },
    { 0x0029, 0x00a2, "US", "Origin of Rectangular Shutter" },
    { 0x0029, 0x00b0, "US", "Radius of Circular Shutter" },
    { 0x0029, 0x00b2, "US", "Origin of Circular Shutter" },
    { 0x0029, 0x00c0, "LT", "Functional Shutter ID" },
    { 0x0029, 0x00c1, "xs", "?" },
    { 0x0029, 0x00c3, "IS", "Scan Resolution" },
    { 0x0029, 0x00c4, "IS", "Field of View" },
    { 0x0029, 0x00c5, "LT", "Field Of Shutter Rectangle" },
    { 0x0029, 0x00ce, "CS", "Shutter Enable Status" },
    { 0x0029, 0x00cf, "CS", "Shutter Select Status" },
    { 0x0029, 0x00d0, "IS", "?" },
    { 0x0029, 0x00d1, "IS", "?" },
    { 0x0029, 0x00d5, "LT", "Slice Thickness" },
    { 0x0031, 0x0010, "LT", "Request UID" },
    { 0x0031, 0x0012, "LT", "Examination Reason" },
    { 0x0031, 0x0030, "DA", "Requested Date" },
    { 0x0031, 0x0032, "TM", "Worklist Request Start Time" },
    { 0x0031, 0x0033, "TM", "Worklist Request End Time" },
    { 0x0031, 0x0045, "LT", "Requesting Physician" },
    { 0x0031, 0x004a, "TM", "Requested Time" },
    { 0x0031, 0x0050, "LT", "Requested Physician" },
    { 0x0031, 0x0080, "LT", "Requested Location" },
    { 0x0032, 0x0000, "UL", "Study Group Length" },
    { 0x0032, 0x000a, "CS", "Study Status ID" },
    { 0x0032, 0x000c, "CS", "Study Priority ID" },
    { 0x0032, 0x0012, "LO", "Study ID Issuer" },
    { 0x0032, 0x0032, "DA", "Study Verified Date" },
    { 0x0032, 0x0033, "TM", "Study Verified Time" },
    { 0x0032, 0x0034, "DA", "Study Read Date" },
    { 0x0032, 0x0035, "TM", "Study Read Time" },
    { 0x0032, 0x1000, "DA", "Scheduled Study Start Date" },
    { 0x0032, 0x1001, "TM", "Scheduled Study Start Time" },
    { 0x0032, 0x1010, "DA", "Scheduled Study Stop Date" },
    { 0x0032, 0x1011, "TM", "Scheduled Study Stop Time" },
    { 0x0032, 0x1020, "LO", "Scheduled Study Location" },
    { 0x0032, 0x1021, "AE", "Scheduled Study Location AE Title(s)" },
    { 0x0032, 0x1030, "LO", "Reason for Study" },
    { 0x0032, 0x1032, "PN", "Requesting Physician" },
    { 0x0032, 0x1033, "LO", "Requesting Service" },
    { 0x0032, 0x1040, "DA", "Study Arrival Date" },
    { 0x0032, 0x1041, "TM", "Study Arrival Time" },
    { 0x0032, 0x1050, "DA", "Study Completion Date" },
    { 0x0032, 0x1051, "TM", "Study Completion Time" },
    { 0x0032, 0x1055, "CS", "Study Component Status ID" },
    { 0x0032, 0x1060, "LO", "Requested Procedure Description" },
    { 0x0032, 0x1064, "SQ", "Requested Procedure Code Sequence" },
    { 0x0032, 0x1070, "LO", "Requested Contrast Agent" },
    { 0x0032, 0x4000, "LT", "Study Comments" },
    { 0x0033, 0x0001, "UN", "?" },
    { 0x0033, 0x0002, "UN", "?" },
    { 0x0033, 0x0005, "UN", "?" },
    { 0x0033, 0x0006, "UN", "?" },
    { 0x0033, 0x0010, "LT", "Patient Study UID" },
    { 0x0037, 0x0010, "LO", "ReferringDepartment" },
    { 0x0037, 0x0020, "US", "ScreenNumber" },
    { 0x0037, 0x0040, "SH", "LeftOrientation" },
    { 0x0037, 0x0042, "SH", "RightOrientation" },
    { 0x0037, 0x0050, "CS", "Inversion" },
    { 0x0037, 0x0060, "US", "DSA" },
    { 0x0038, 0x0000, "UL", "Visit Group Length" },
    { 0x0038, 0x0004, "SQ", "Referenced Patient Alias Sequence" },
    { 0x0038, 0x0008, "CS", "Visit Status ID" },
    { 0x0038, 0x0010, "LO", "Admission ID" },
    { 0x0038, 0x0011, "LO", "Issuer of Admission ID" },
    { 0x0038, 0x0016, "LO", "Route of Admissions" },
    { 0x0038, 0x001a, "DA", "Scheduled Admission Date" },
    { 0x0038, 0x001b, "TM", "Scheduled Admission Time" },
    { 0x0038, 0x001c, "DA", "Scheduled Discharge Date" },
    { 0x0038, 0x001d, "TM", "Scheduled Discharge Time" },
    { 0x0038, 0x001e, "LO", "Scheduled Patient Institution Residence" },
    { 0x0038, 0x0020, "DA", "Admitting Date" },
    { 0x0038, 0x0021, "TM", "Admitting Time" },
    { 0x0038, 0x0030, "DA", "Discharge Date" },
    { 0x0038, 0x0032, "TM", "Discharge Time" },
    { 0x0038, 0x0040, "LO", "Discharge Diagnosis Description" },
    { 0x0038, 0x0044, "SQ", "Discharge Diagnosis Code Sequence" },
    { 0x0038, 0x0050, "LO", "Special Needs" },
    { 0x0038, 0x0300, "LO", "Current Patient Location" },
    { 0x0038, 0x0400, "LO", "Patient's Institution Residence" },
    { 0x0038, 0x0500, "LO", "Patient State" },
    { 0x0038, 0x4000, "LT", "Visit Comments" },
    { 0x0039, 0x0080, "IS", "Private Entity Number" },
    { 0x0039, 0x0085, "DA", "Private Entity Date" },
    { 0x0039, 0x0090, "TM", "Private Entity Time" },
    { 0x0039, 0x0095, "LO", "Private Entity Launch Command" },
    { 0x0039, 0x00aa, "CS", "Private Entity Type" },
    { 0x003a, 0x0002, "SQ", "Waveform Sequence" },
    { 0x003a, 0x0005, "US", "Waveform Number of Channels" },
    { 0x003a, 0x0010, "UL", "Waveform Number of Samples" },
    { 0x003a, 0x001a, "DS", "Sampling Frequency" },
    { 0x003a, 0x0020, "SH", "Group Label" },
    { 0x003a, 0x0103, "CS", "Waveform Sample Value Representation" },
    { 0x003a, 0x0122, "OB", "Waveform Padding Value" },
    { 0x003a, 0x0200, "SQ", "Channel Definition" },
    { 0x003a, 0x0202, "IS", "Waveform Channel Number" },
    { 0x003a, 0x0203, "SH", "Channel Label" },
    { 0x003a, 0x0205, "CS", "Channel Status" },
    { 0x003a, 0x0208, "SQ", "Channel Source" },
    { 0x003a, 0x0209, "SQ", "Channel Source Modifiers" },
    { 0x003a, 0x020a, "SQ", "Differential Channel Source" },
    { 0x003a, 0x020b, "SQ", "Differential Channel Source Modifiers" },
    { 0x003a, 0x0210, "DS", "Channel Sensitivity" },
    { 0x003a, 0x0211, "SQ", "Channel Sensitivity Units" },
    { 0x003a, 0x0212, "DS", "Channel Sensitivity Correction Factor" },
    { 0x003a, 0x0213, "DS", "Channel Baseline" },
    { 0x003a, 0x0214, "DS", "Channel Time Skew" },
    { 0x003a, 0x0215, "DS", "Channel Sample Skew" },
    { 0x003a, 0x0216, "OB", "Channel Minimum Value" },
    { 0x003a, 0x0217, "OB", "Channel Maximum Value" },
    { 0x003a, 0x0218, "DS", "Channel Offset" },
    { 0x003a, 0x021a, "US", "Bits Per Sample" },
    { 0x003a, 0x0220, "DS", "Filter Low Frequency" },
    { 0x003a, 0x0221, "DS", "Filter High Frequency" },
    { 0x003a, 0x0222, "DS", "Notch Filter Frequency" },
    { 0x003a, 0x0223, "DS", "Notch Filter Bandwidth" },
    { 0x003a, 0x1000, "OB", "Waveform Data" },
    { 0x0040, 0x0001, "AE", "Scheduled Station AE Title" },
    { 0x0040, 0x0002, "DA", "Scheduled Procedure Step Start Date" },
    { 0x0040, 0x0003, "TM", "Scheduled Procedure Step Start Time" },
    { 0x0040, 0x0004, "DA", "Scheduled Procedure Step End Date" },
    { 0x0040, 0x0005, "TM", "Scheduled Procedure Step End Time" },
    { 0x0040, 0x0006, "PN", "Scheduled Performing Physician Name" },
    { 0x0040, 0x0007, "LO", "Scheduled Procedure Step Description" },
    { 0x0040, 0x0008, "SQ", "Scheduled Action Item Code Sequence" },
    { 0x0040, 0x0009, "SH", "Scheduled Procedure Step ID" },
    { 0x0040, 0x0010, "SH", "Scheduled Station Name" },
    { 0x0040, 0x0011, "SH", "Scheduled Procedure Step Location" },
    { 0x0040, 0x0012, "LO", "Pre-Medication" },
    { 0x0040, 0x0020, "CS", "Scheduled Procedure Step Status" },
    { 0x0040, 0x0100, "SQ", "Scheduled Procedure Step Sequence" },
    { 0x0040, 0x0302, "US", "Entrance Dose" },
    { 0x0040, 0x0303, "US", "Exposed Area" },
    { 0x0040, 0x0306, "DS", "Distance Source to Entrance" },
    { 0x0040, 0x0307, "DS", "Distance Source to Support" },
    { 0x0040, 0x0310, "ST", "Comments On Radiation Dose" },
    { 0x0040, 0x0312, "DS", "X-Ray Output" },
    { 0x0040, 0x0314, "DS", "Half Value Layer" },
    { 0x0040, 0x0316, "DS", "Organ Dose" },
    { 0x0040, 0x0318, "CS", "Organ Exposed" },
    { 0x0040, 0x0400, "LT", "Comments On Scheduled Procedure Step" },
    { 0x0040, 0x050a, "LO", "Specimen Accession Number" },
    { 0x0040, 0x0550, "SQ", "Specimen Sequence" },
    { 0x0040, 0x0551, "LO", "Specimen Identifier" },
    { 0x0040, 0x0552, "SQ", "Specimen Description Sequence" },
    { 0x0040, 0x0553, "ST", "Specimen Description" },
    { 0x0040, 0x0555, "SQ", "Acquisition Context Sequence" },
    { 0x0040, 0x0556, "ST", "Acquisition Context Description" },
    { 0x0040, 0x059a, "SQ", "Specimen Type Code Sequence" },
    { 0x0040, 0x06fa, "LO", "Slide Identifier" },
    { 0x0040, 0x071a, "SQ", "Image Center Point Coordinates Sequence" },
    { 0x0040, 0x072a, "DS", "X Offset In Slide Coordinate System" },
    { 0x0040, 0x073a, "DS", "Y Offset In Slide Coordinate System" },
    { 0x0040, 0x074a, "DS", "Z Offset In Slide Coordinate System" },
    { 0x0040, 0x08d8, "SQ", "Pixel Spacing Sequence" },
    { 0x0040, 0x08da, "SQ", "Coordinate System Axis Code Sequence" },
    { 0x0040, 0x08ea, "SQ", "Measurement Units Code Sequence" },
    { 0x0040, 0x09f8, "SQ", "Vital Stain Code Sequence" },
    { 0x0040, 0x1001, "SH", "Requested Procedure ID" },
    { 0x0040, 0x1002, "LO", "Reason For Requested Procedure" },
    { 0x0040, 0x1003, "SH", "Requested Procedure Priority" },
    { 0x0040, 0x1004, "LO", "Patient Transport Arrangements" },
    { 0x0040, 0x1005, "LO", "Requested Procedure Location" },
    { 0x0040, 0x1006, "SH", "Placer Order Number of Procedure" },
    { 0x0040, 0x1007, "SH", "Filler Order Number of Procedure" },
    { 0x0040, 0x1008, "LO", "Confidentiality Code" },
    { 0x0040, 0x1009, "SH", "Reporting Priority" },
    { 0x0040, 0x1010, "PN", "Names of Intended Recipients of Results" },
    { 0x0040, 0x1400, "LT", "Requested Procedure Comments" },
    { 0x0040, 0x2001, "LO", "Reason For Imaging Service Request" },
    { 0x0040, 0x2004, "DA", "Issue Date of Imaging Service Request" },
    { 0x0040, 0x2005, "TM", "Issue Time of Imaging Service Request" },
    { 0x0040, 0x2006, "SH", "Placer Order Number of Imaging Service Request" },
    { 0x0040, 0x2007, "SH", "Filler Order Number of Imaging Service Request" },
    { 0x0040, 0x2008, "PN", "Order Entered By" },
    { 0x0040, 0x2009, "SH", "Order Enterer Location" },
    { 0x0040, 0x2010, "SH", "Order Callback Phone Number" },
    { 0x0040, 0x2400, "LT", "Imaging Service Request Comments" },
    { 0x0040, 0x3001, "LO", "Confidentiality Constraint On Patient Data" },
    { 0x0040, 0xa007, "CS", "Findings Flag" },
    { 0x0040, 0xa020, "SQ", "Findings Sequence" },
    { 0x0040, 0xa021, "UI", "Findings Group UID" },
    { 0x0040, 0xa022, "UI", "Referenced Findings Group UID" },
    { 0x0040, 0xa023, "DA", "Findings Group Recording Date" },
    { 0x0040, 0xa024, "TM", "Findings Group Recording Time" },
    { 0x0040, 0xa026, "SQ", "Findings Source Category Code Sequence" },
    { 0x0040, 0xa027, "LO", "Documenting Organization" },
    { 0x0040, 0xa028, "SQ", "Documenting Organization Identifier Code Sequence" },
    { 0x0040, 0xa032, "LO", "History Reliability Qualifier Description" },
    { 0x0040, 0xa043, "SQ", "Concept Name Code Sequence" },
    { 0x0040, 0xa047, "LO", "Measurement Precision Description" },
    { 0x0040, 0xa057, "CS", "Urgency or Priority Alerts" },
    { 0x0040, 0xa060, "LO", "Sequencing Indicator" },
    { 0x0040, 0xa066, "SQ", "Document Identifier Code Sequence" },
    { 0x0040, 0xa067, "PN", "Document Author" },
    { 0x0040, 0xa068, "SQ", "Document Author Identifier Code Sequence" },
    { 0x0040, 0xa070, "SQ", "Identifier Code Sequence" },
    { 0x0040, 0xa073, "LO", "Object String Identifier" },
    { 0x0040, 0xa074, "OB", "Object Binary Identifier" },
    { 0x0040, 0xa075, "PN", "Documenting Observer" },
    { 0x0040, 0xa076, "SQ", "Documenting Observer Identifier Code Sequence" },
    { 0x0040, 0xa078, "SQ", "Observation Subject Identifier Code Sequence" },
    { 0x0040, 0xa080, "SQ", "Person Identifier Code Sequence" },
    { 0x0040, 0xa085, "SQ", "Procedure Identifier Code Sequence" },
    { 0x0040, 0xa088, "LO", "Object Directory String Identifier" },
    { 0x0040, 0xa089, "OB", "Object Directory Binary Identifier" },
    { 0x0040, 0xa090, "CS", "History Reliability Qualifier" },
    { 0x0040, 0xa0a0, "CS", "Referenced Type of Data" },
    { 0x0040, 0xa0b0, "US", "Referenced Waveform Channels" },
    { 0x0040, 0xa110, "DA", "Date of Document or Verbal Transaction" },
    { 0x0040, 0xa112, "TM", "Time of Document Creation or Verbal Transaction" },
    { 0x0040, 0xa121, "DA", "Date" },
    { 0x0040, 0xa122, "TM", "Time" },
    { 0x0040, 0xa123, "PN", "Person Name" },
    { 0x0040, 0xa124, "SQ", "Referenced Person Sequence" },
    { 0x0040, 0xa125, "CS", "Report Status ID" },
    { 0x0040, 0xa130, "CS", "Temporal Range Type" },
    { 0x0040, 0xa132, "UL", "Referenced Sample Offsets" },
    { 0x0040, 0xa136, "US", "Referenced Frame Numbers" },
    { 0x0040, 0xa138, "DS", "Referenced Time Offsets" },
    { 0x0040, 0xa13a, "DT", "Referenced Datetime" },
    { 0x0040, 0xa160, "UT", "Text Value" },
    { 0x0040, 0xa167, "SQ", "Observation Category Code Sequence" },
    { 0x0040, 0xa168, "SQ", "Concept Code Sequence" },
    { 0x0040, 0xa16a, "ST", "Bibliographic Citation" },
    { 0x0040, 0xa170, "CS", "Observation Class" },
    { 0x0040, 0xa171, "UI", "Observation UID" },
    { 0x0040, 0xa172, "UI", "Referenced Observation UID" },
    { 0x0040, 0xa173, "CS", "Referenced Observation Class" },
    { 0x0040, 0xa174, "CS", "Referenced Object Observation Class" },
    { 0x0040, 0xa180, "US", "Annotation Group Number" },
    { 0x0040, 0xa192, "DA", "Observation Date" },
    { 0x0040, 0xa193, "TM", "Observation Time" },
    { 0x0040, 0xa194, "CS", "Measurement Automation" },
    { 0x0040, 0xa195, "SQ", "Concept Name Code Sequence Modifier" },
    { 0x0040, 0xa224, "ST", "Identification Description" },
    { 0x0040, 0xa290, "CS", "Coordinates Set Geometric Type" },
    { 0x0040, 0xa296, "SQ", "Algorithm Code Sequence" },
    { 0x0040, 0xa297, "ST", "Algorithm Description" },
    { 0x0040, 0xa29a, "SL", "Pixel Coordinates Set" },
    { 0x0040, 0xa300, "SQ", "Measured Value Sequence" },
    { 0x0040, 0xa307, "PN", "Current Observer" },
    { 0x0040, 0xa30a, "DS", "Numeric Value" },
    { 0x0040, 0xa313, "SQ", "Referenced Accession Sequence" },
    { 0x0040, 0xa33a, "ST", "Report Status Comment" },
    { 0x0040, 0xa340, "SQ", "Procedure Context Sequence" },
    { 0x0040, 0xa352, "PN", "Verbal Source" },
    { 0x0040, 0xa353, "ST", "Address" },
    { 0x0040, 0xa354, "LO", "Telephone Number" },
    { 0x0040, 0xa358, "SQ", "Verbal Source Identifier Code Sequence" },
    { 0x0040, 0xa380, "SQ", "Report Detail Sequence" },
    { 0x0040, 0xa402, "UI", "Observation Subject UID" },
    { 0x0040, 0xa403, "CS", "Observation Subject Class" },
    { 0x0040, 0xa404, "SQ", "Observation Subject Type Code Sequence" },
    { 0x0040, 0xa600, "CS", "Observation Subject Context Flag" },
    { 0x0040, 0xa601, "CS", "Observer Context Flag" },
    { 0x0040, 0xa603, "CS", "Procedure Context Flag" },
    { 0x0040, 0xa730, "SQ", "Observations Sequence" },
    { 0x0040, 0xa731, "SQ", "Relationship Sequence" },
    { 0x0040, 0xa732, "SQ", "Relationship Type Code Sequence" },
    { 0x0040, 0xa744, "SQ", "Language Code Sequence" },
    { 0x0040, 0xa992, "ST", "Uniform Resource Locator" },
    { 0x0040, 0xb020, "SQ", "Annotation Sequence" },
    { 0x0040, 0xdb73, "SQ", "Relationship Type Code Sequence Modifier" },
    { 0x0041, 0x0000, "LT", "Papyrus Comments" },
    { 0x0041, 0x0010, "xs", "?" },
    { 0x0041, 0x0011, "xs", "?" },
    { 0x0041, 0x0012, "UL", "Pixel Offset" },
    { 0x0041, 0x0013, "SQ", "Image Identifier Sequence" },
    { 0x0041, 0x0014, "SQ", "External File Reference Sequence" },
    { 0x0041, 0x0015, "US", "Number of Images" },
    { 0x0041, 0x0020, "xs", "?" },
    { 0x0041, 0x0021, "UI", "Referenced SOP Class UID" },
    { 0x0041, 0x0022, "UI", "Referenced SOP Instance UID" },
    { 0x0041, 0x0030, "xs", "?" },
    { 0x0041, 0x0031, "xs", "?" },
    { 0x0041, 0x0032, "xs", "?" },
    { 0x0041, 0x0034, "DA", "Modified Date" },
    { 0x0041, 0x0036, "TM", "Modified Time" },
    { 0x0041, 0x0040, "LT", "Owner Name" },
    { 0x0041, 0x0041, "UI", "Referenced Image SOP Class UID" },
    { 0x0041, 0x0042, "UI", "Referenced Image SOP Instance UID" },
    { 0x0041, 0x0050, "xs", "?" },
    { 0x0041, 0x0060, "UL", "Number of Images" },
    { 0x0041, 0x0062, "UL", "Number of Other" },
    { 0x0041, 0x00a0, "LT", "External Folder Element DSID" },
    { 0x0041, 0x00a1, "US", "External Folder Element Data Set Type" },
    { 0x0041, 0x00a2, "LT", "External Folder Element File Location" },
    { 0x0041, 0x00a3, "UL", "External Folder Element Length" },
    { 0x0041, 0x00b0, "LT", "Internal Folder Element DSID" },
    { 0x0041, 0x00b1, "US", "Internal Folder Element Data Set Type" },
    { 0x0041, 0x00b2, "UL", "Internal Offset To Data Set" },
    { 0x0041, 0x00b3, "UL", "Internal Offset To Image" },
    { 0x0043, 0x0001, "SS", "Bitmap Of Prescan Options" },
    { 0x0043, 0x0002, "SS", "Gradient Offset In X" },
    { 0x0043, 0x0003, "SS", "Gradient Offset In Y" },
    { 0x0043, 0x0004, "SS", "Gradient Offset In Z" },
    { 0x0043, 0x0005, "SS", "Image Is Original Or Unoriginal" },
    { 0x0043, 0x0006, "SS", "Number Of EPI Shots" },
    { 0x0043, 0x0007, "SS", "Views Per Segment" },
    { 0x0043, 0x0008, "SS", "Respiratory Rate In BPM" },
    { 0x0043, 0x0009, "SS", "Respiratory Trigger Point" },
    { 0x0043, 0x000a, "SS", "Type Of Receiver Used" },
    { 0x0043, 0x000b, "DS", "Peak Rate Of Change Of Gradient Field" },
    { 0x0043, 0x000c, "DS", "Limits In Units Of Percent" },
    { 0x0043, 0x000d, "DS", "PSD Estimated Limit" },
    { 0x0043, 0x000e, "DS", "PSD Estimated Limit In Tesla Per Second" },
    { 0x0043, 0x000f, "DS", "SAR Avg Head" },
    { 0x0043, 0x0010, "US", "Window Value" },
    { 0x0043, 0x0011, "US", "Total Input Views" },
    { 0x0043, 0x0012, "SS", "Xray Chain" },
    { 0x0043, 0x0013, "SS", "Recon Kernel Parameters" },
    { 0x0043, 0x0014, "SS", "Calibration Parameters" },
    { 0x0043, 0x0015, "SS", "Total Output Views" },
    { 0x0043, 0x0016, "SS", "Number Of Overranges" },
    { 0x0043, 0x0017, "DS", "IBH Image Scale Factors" },
    { 0x0043, 0x0018, "DS", "BBH Coefficients" },
    { 0x0043, 0x0019, "SS", "Number Of BBH Chains To Blend" },
    { 0x0043, 0x001a, "SL", "Starting Channel Number" },
    { 0x0043, 0x001b, "SS", "PPScan Parameters" },
    { 0x0043, 0x001c, "SS", "GE Image Integrity" },
    { 0x0043, 0x001d, "SS", "Level Value" },
    { 0x0043, 0x001e, "xs", "?" },
    { 0x0043, 0x001f, "SL", "Max Overranges In A View" },
    { 0x0043, 0x0020, "DS", "Avg Overranges All Views" },
    { 0x0043, 0x0021, "SS", "Corrected Afterglow Terms" },
    { 0x0043, 0x0025, "SS", "Reference Channels" },
    { 0x0043, 0x0026, "US", "No Views Ref Channels Blocked" },
    { 0x0043, 0x0027, "xs", "?" },
    { 0x0043, 0x0028, "OB", "Unique Image Identifier" },
    { 0x0043, 0x0029, "OB", "Histogram Tables" },
    { 0x0043, 0x002a, "OB", "User Defined Data" },
    { 0x0043, 0x002b, "SS", "Private Scan Options" },
    { 0x0043, 0x002c, "SS", "Effective Echo Spacing" },
    { 0x0043, 0x002d, "SH", "String Slop Field 1" },
    { 0x0043, 0x002e, "SH", "String Slop Field 2" },
    { 0x0043, 0x002f, "SS", "Raw Data Type" },
    { 0x0043, 0x0030, "SS", "Raw Data Type" },
    { 0x0043, 0x0031, "DS", "RA Coord Of Target Recon Centre" },
    { 0x0043, 0x0032, "SS", "Raw Data Type" },
    { 0x0043, 0x0033, "FL", "Neg Scan Spacing" },
    { 0x0043, 0x0034, "IS", "Offset Frequency" },
    { 0x0043, 0x0035, "UL", "User Usage Tag" },
    { 0x0043, 0x0036, "UL", "User Fill Map MSW" },
    { 0x0043, 0x0037, "UL", "User Fill Map LSW" },
    { 0x0043, 0x0038, "FL", "User 25 To User 48" },
    { 0x0043, 0x0039, "IS", "Slop Integer 6 To Slop Integer 9" },
    { 0x0043, 0x0040, "FL", "Trigger On Position" },
    { 0x0043, 0x0041, "FL", "Degree Of Rotation" },
    { 0x0043, 0x0042, "SL", "DAS Trigger Source" },
    { 0x0043, 0x0043, "SL", "DAS Fpa Gain" },
    { 0x0043, 0x0044, "SL", "DAS Output Source" },
    { 0x0043, 0x0045, "SL", "DAS Ad Input" },
    { 0x0043, 0x0046, "SL", "DAS Cal Mode" },
    { 0x0043, 0x0047, "SL", "DAS Cal Frequency" },
    { 0x0043, 0x0048, "SL", "DAS Reg Xm" },
    { 0x0043, 0x0049, "SL", "DAS Auto Zero" },
    { 0x0043, 0x004a, "SS", "Starting Channel Of View" },
    { 0x0043, 0x004b, "SL", "DAS Xm Pattern" },
    { 0x0043, 0x004c, "SS", "TGGC Trigger Mode" },
    { 0x0043, 0x004d, "FL", "Start Scan To Xray On Delay" },
    { 0x0043, 0x004e, "FL", "Duration Of Xray On" },
    { 0x0044, 0x0000, "UI", "?" },
    { 0x0045, 0x0004, "CS", "AES" },
    { 0x0045, 0x0006, "DS", "Angulation" },
    { 0x0045, 0x0009, "DS", "Real Magnification Factor" },
    { 0x0045, 0x000b, "CS", "Senograph Type" },
    { 0x0045, 0x000c, "DS", "Integration Time" },
    { 0x0045, 0x000d, "DS", "ROI Origin X and Y" },
    { 0x0045, 0x0011, "DS", "Receptor Size cm X and Y" },
    { 0x0045, 0x0012, "IS", "Receptor Size Pixels X and Y" },
    { 0x0045, 0x0013, "ST", "Screen" },
    { 0x0045, 0x0014, "DS", "Pixel Pitch Microns" },
    { 0x0045, 0x0015, "IS", "Pixel Depth Bits" },
    { 0x0045, 0x0016, "IS", "Binning Factor X and Y" },
    { 0x0045, 0x001b, "CS", "Clinical View" },
    { 0x0045, 0x001d, "DS", "Mean Of Raw Gray Levels" },
    { 0x0045, 0x001e, "DS", "Mean Of Offset Gray Levels" },
    { 0x0045, 0x001f, "DS", "Mean Of Corrected Gray Levels" },
    { 0x0045, 0x0020, "DS", "Mean Of Region Gray Levels" },
    { 0x0045, 0x0021, "DS", "Mean Of Log Region Gray Levels" },
    { 0x0045, 0x0022, "DS", "Standard Deviation Of Raw Gray Levels" },
    { 0x0045, 0x0023, "DS", "Standard Deviation Of Corrected Gray Levels" },
    { 0x0045, 0x0024, "DS", "Standard Deviation Of Region Gray Levels" },
    { 0x0045, 0x0025, "DS", "Standard Deviation Of Log Region Gray Levels" },
    { 0x0045, 0x0026, "OB", "MAO Buffer" },
    { 0x0045, 0x0027, "IS", "Set Number" },
    { 0x0045, 0x0028, "CS", "WindowingType (LINEAR or GAMMA)" },
    { 0x0045, 0x0029, "DS", "WindowingParameters" },
    { 0x0045, 0x002a, "IS", "Crosshair Cursor X Coordinates" },
    { 0x0045, 0x002b, "IS", "Crosshair Cursor Y Coordinates" },
    { 0x0045, 0x0039, "US", "Vignette Rows" },
    { 0x0045, 0x003a, "US", "Vignette Columns" },
    { 0x0045, 0x003b, "US", "Vignette Bits Allocated" },
    { 0x0045, 0x003c, "US", "Vignette Bits Stored" },
    { 0x0045, 0x003d, "US", "Vignette High Bit" },
    { 0x0045, 0x003e, "US", "Vignette Pixel Representation" },
    { 0x0045, 0x003f, "OB", "Vignette Pixel Data" },
    { 0x0047, 0x0001, "SQ", "Reconstruction Parameters Sequence" },
    { 0x0047, 0x0050, "UL", "Volume Voxel Count" },
    { 0x0047, 0x0051, "UL", "Volume Segment Count" },
    { 0x0047, 0x0053, "US", "Volume Slice Size" },
    { 0x0047, 0x0054, "US", "Volume Slice Count" },
    { 0x0047, 0x0055, "SL", "Volume Threshold Value" },
    { 0x0047, 0x0057, "DS", "Volume Voxel Ratio" },
    { 0x0047, 0x0058, "DS", "Volume Voxel Size" },
    { 0x0047, 0x0059, "US", "Volume Z Position Size" },
    { 0x0047, 0x0060, "DS", "Volume Base Line" },
    { 0x0047, 0x0061, "DS", "Volume Center Point" },
    { 0x0047, 0x0063, "SL", "Volume Skew Base" },
    { 0x0047, 0x0064, "DS", "Volume Registration Transform Rotation Matrix" },
    { 0x0047, 0x0065, "DS", "Volume Registration Transform Translation Vector" },
    { 0x0047, 0x0070, "DS", "KVP List" },
    { 0x0047, 0x0071, "IS", "XRay Tube Current List" },
    { 0x0047, 0x0072, "IS", "Exposure List" },
    { 0x0047, 0x0080, "LO", "Acquisition DLX Identifier" },
    { 0x0047, 0x0085, "SQ", "Acquisition DLX 2D Series Sequence" },
    { 0x0047, 0x0089, "DS", "Contrast Agent Volume List" },
    { 0x0047, 0x008a, "US", "Number Of Injections" },
    { 0x0047, 0x008b, "US", "Frame Count" },
    { 0x0047, 0x0096, "IS", "Used Frames" },
    { 0x0047, 0x0091, "LO", "XA 3D Reconstruction Algorithm Name" },
    { 0x0047, 0x0092, "CS", "XA 3D Reconstruction Algorithm Version" },
    { 0x0047, 0x0093, "DA", "DLX Calibration Date" },
    { 0x0047, 0x0094, "TM", "DLX Calibration Time" },
    { 0x0047, 0x0095, "CS", "DLX Calibration Status" },
    { 0x0047, 0x0098, "US", "Transform Count" },
    { 0x0047, 0x0099, "SQ", "Transform Sequence" },
    { 0x0047, 0x009a, "DS", "Transform Rotation Matrix" },
    { 0x0047, 0x009b, "DS", "Transform Translation Vector" },
    { 0x0047, 0x009c, "LO", "Transform Label" },
    { 0x0047, 0x00b1, "US", "Wireframe Count" },
    { 0x0047, 0x00b2, "US", "Location System" },
    { 0x0047, 0x00b0, "SQ", "Wireframe List" },
    { 0x0047, 0x00b5, "LO", "Wireframe Name" },
    { 0x0047, 0x00b6, "LO", "Wireframe Group Name" },
    { 0x0047, 0x00b7, "LO", "Wireframe Color" },
    { 0x0047, 0x00b8, "SL", "Wireframe Attributes" },
    { 0x0047, 0x00b9, "SL", "Wireframe Point Count" },
    { 0x0047, 0x00ba, "SL", "Wireframe Timestamp" },
    { 0x0047, 0x00bb, "SQ", "Wireframe Point List" },
    { 0x0047, 0x00bc, "DS", "Wireframe Points Coordinates" },
    { 0x0047, 0x00c0, "DS", "Volume Upper Left High Corner RAS" },
    { 0x0047, 0x00c1, "DS", "Volume Slice To RAS Rotation Matrix" },
    { 0x0047, 0x00c2, "DS", "Volume Upper Left High Corner TLOC" },
    { 0x0047, 0x00d1, "OB", "Volume Segment List" },
    { 0x0047, 0x00d2, "OB", "Volume Gradient List" },
    { 0x0047, 0x00d3, "OB", "Volume Density List" },
    { 0x0047, 0x00d4, "OB", "Volume Z Position List" },
    { 0x0047, 0x00d5, "OB", "Volume Original Index List" },
    { 0x0050, 0x0000, "UL", "Calibration Group Length" },
    { 0x0050, 0x0004, "CS", "Calibration Object" },
    { 0x0050, 0x0010, "SQ", "DeviceSequence" },
    { 0x0050, 0x0014, "DS", "DeviceLength" },
    { 0x0050, 0x0016, "DS", "DeviceDiameter" },
    { 0x0050, 0x0017, "CS", "DeviceDiameterUnits" },
    { 0x0050, 0x0018, "DS", "DeviceVolume" },
    { 0x0050, 0x0019, "DS", "InterMarkerDistance" },
    { 0x0050, 0x0020, "LO", "DeviceDescription" },
    { 0x0050, 0x0030, "SQ", "CodedInterventionDeviceSequence" },
    { 0x0051, 0x0010, "xs", "Image Text" },
    { 0x0054, 0x0000, "UL", "Nuclear Acquisition Group Length" },
    { 0x0054, 0x0010, "US", "Energy Window Vector" },
    { 0x0054, 0x0011, "US", "Number of Energy Windows" },
    { 0x0054, 0x0012, "SQ", "Energy Window Information Sequence" },
    { 0x0054, 0x0013, "SQ", "Energy Window Range Sequence" },
    { 0x0054, 0x0014, "DS", "Energy Window Lower Limit" },
    { 0x0054, 0x0015, "DS", "Energy Window Upper Limit" },
    { 0x0054, 0x0016, "SQ", "Radiopharmaceutical Information Sequence" },
    { 0x0054, 0x0017, "IS", "Residual Syringe Counts" },
    { 0x0054, 0x0018, "SH", "Energy Window Name" },
    { 0x0054, 0x0020, "US", "Detector Vector" },
    { 0x0054, 0x0021, "US", "Number of Detectors" },
    { 0x0054, 0x0022, "SQ", "Detector Information Sequence" },
    { 0x0054, 0x0030, "US", "Phase Vector" },
    { 0x0054, 0x0031, "US", "Number of Phases" },
    { 0x0054, 0x0032, "SQ", "Phase Information Sequence" },
    { 0x0054, 0x0033, "US", "Number of Frames In Phase" },
    { 0x0054, 0x0036, "IS", "Phase Delay" },
    { 0x0054, 0x0038, "IS", "Pause Between Frames" },
    { 0x0054, 0x0050, "US", "Rotation Vector" },
    { 0x0054, 0x0051, "US", "Number of Rotations" },
    { 0x0054, 0x0052, "SQ", "Rotation Information Sequence" },
    { 0x0054, 0x0053, "US", "Number of Frames In Rotation" },
    { 0x0054, 0x0060, "US", "R-R Interval Vector" },
    { 0x0054, 0x0061, "US", "Number of R-R Intervals" },
    { 0x0054, 0x0062, "SQ", "Gated Information Sequence" },
    { 0x0054, 0x0063, "SQ", "Data Information Sequence" },
    { 0x0054, 0x0070, "US", "Time Slot Vector" },
    { 0x0054, 0x0071, "US", "Number of Time Slots" },
    { 0x0054, 0x0072, "SQ", "Time Slot Information Sequence" },
    { 0x0054, 0x0073, "DS", "Time Slot Time" },
    { 0x0054, 0x0080, "US", "Slice Vector" },
    { 0x0054, 0x0081, "US", "Number of Slices" },
    { 0x0054, 0x0090, "US", "Angular View Vector" },
    { 0x0054, 0x0100, "US", "Time Slice Vector" },
    { 0x0054, 0x0101, "US", "Number Of Time Slices" },
    { 0x0054, 0x0200, "DS", "Start Angle" },
    { 0x0054, 0x0202, "CS", "Type of Detector Motion" },
    { 0x0054, 0x0210, "IS", "Trigger Vector" },
    { 0x0054, 0x0211, "US", "Number of Triggers in Phase" },
    { 0x0054, 0x0220, "SQ", "View Code Sequence" },
    { 0x0054, 0x0222, "SQ", "View Modifier Code Sequence" },
    { 0x0054, 0x0300, "SQ", "Radionuclide Code Sequence" },
    { 0x0054, 0x0302, "SQ", "Radiopharmaceutical Route Code Sequence" },
    { 0x0054, 0x0304, "SQ", "Radiopharmaceutical Code Sequence" },
    { 0x0054, 0x0306, "SQ", "Calibration Data Sequence" },
    { 0x0054, 0x0308, "US", "Energy Window Number" },
    { 0x0054, 0x0400, "SH", "Image ID" },
    { 0x0054, 0x0410, "SQ", "Patient Orientation Code Sequence" },
    { 0x0054, 0x0412, "SQ", "Patient Orientation Modifier Code Sequence" },
    { 0x0054, 0x0414, "SQ", "Patient Gantry Relationship Code Sequence" },
    { 0x0054, 0x1000, "CS", "Positron Emission Tomography Series Type" },
    { 0x0054, 0x1001, "CS", "Positron Emission Tomography Units" },
    { 0x0054, 0x1002, "CS", "Counts Source" },
    { 0x0054, 0x1004, "CS", "Reprojection Method" },
    { 0x0054, 0x1100, "CS", "Randoms Correction Method" },
    { 0x0054, 0x1101, "LO", "Attenuation Correction Method" },
    { 0x0054, 0x1102, "CS", "Decay Correction" },
    { 0x0054, 0x1103, "LO", "Reconstruction Method" },
    { 0x0054, 0x1104, "LO", "Detector Lines of Response Used" },
    { 0x0054, 0x1105, "LO", "Scatter Correction Method" },
    { 0x0054, 0x1200, "DS", "Axial Acceptance" },
    { 0x0054, 0x1201, "IS", "Axial Mash" },
    { 0x0054, 0x1202, "IS", "Transverse Mash" },
    { 0x0054, 0x1203, "DS", "Detector Element Size" },
    { 0x0054, 0x1210, "DS", "Coincidence Window Width" },
    { 0x0054, 0x1220, "CS", "Secondary Counts Type" },
    { 0x0054, 0x1300, "DS", "Frame Reference Time" },
    { 0x0054, 0x1310, "IS", "Primary Prompts Counts Accumulated" },
    { 0x0054, 0x1311, "IS", "Secondary Counts Accumulated" },
    { 0x0054, 0x1320, "DS", "Slice Sensitivity Factor" },
    { 0x0054, 0x1321, "DS", "Decay Factor" },
    { 0x0054, 0x1322, "DS", "Dose Calibration Factor" },
    { 0x0054, 0x1323, "DS", "Scatter Fraction Factor" },
    { 0x0054, 0x1324, "DS", "Dead Time Factor" },
    { 0x0054, 0x1330, "US", "Image Index" },
    { 0x0054, 0x1400, "CS", "Counts Included" },
    { 0x0054, 0x1401, "CS", "Dead Time Correction Flag" },
    { 0x0055, 0x0046, "LT", "Current Ward" },
    { 0x0058, 0x0000, "SQ", "?" },
    { 0x0060, 0x3000, "SQ", "Histogram Sequence" },
    { 0x0060, 0x3002, "US", "Histogram Number of Bins" },
    { 0x0060, 0x3004, "xs", "Histogram First Bin Value" },
    { 0x0060, 0x3006, "xs", "Histogram Last Bin Value" },
    { 0x0060, 0x3008, "US", "Histogram Bin Width" },
    { 0x0060, 0x3010, "LO", "Histogram Explanation" },
    { 0x0060, 0x3020, "UL", "Histogram Data" },
    { 0x0070, 0x0001, "SQ", "Graphic Annotation Sequence" },
    { 0x0070, 0x0002, "CS", "Graphic Layer" },
    { 0x0070, 0x0003, "CS", "Bounding Box Annotation Units" },
    { 0x0070, 0x0004, "CS", "Anchor Point Annotation Units" },
    { 0x0070, 0x0005, "CS", "Graphic Annotation Units" },
    { 0x0070, 0x0006, "ST", "Unformatted Text Value" },
    { 0x0070, 0x0008, "SQ", "Text Object Sequence" },
    { 0x0070, 0x0009, "SQ", "Graphic Object Sequence" },
    { 0x0070, 0x0010, "FL", "Bounding Box TLHC" },
    { 0x0070, 0x0011, "FL", "Bounding Box BRHC" },
    { 0x0070, 0x0014, "FL", "Anchor Point" },
    { 0x0070, 0x0015, "CS", "Anchor Point Visibility" },
    { 0x0070, 0x0020, "US", "Graphic Dimensions" },
    { 0x0070, 0x0021, "US", "Number Of Graphic Points" },
    { 0x0070, 0x0022, "FL", "Graphic Data" },
    { 0x0070, 0x0023, "CS", "Graphic Type" },
    { 0x0070, 0x0024, "CS", "Graphic Filled" },
    { 0x0070, 0x0040, "IS", "Image Rotation" },
    { 0x0070, 0x0041, "CS", "Image Horizontal Flip" },
    { 0x0070, 0x0050, "US", "Displayed Area TLHC" },
    { 0x0070, 0x0051, "US", "Displayed Area BRHC" },
    { 0x0070, 0x0060, "SQ", "Graphic Layer Sequence" },
    { 0x0070, 0x0062, "IS", "Graphic Layer Order" },
    { 0x0070, 0x0066, "US", "Graphic Layer Recommended Display Value" },
    { 0x0070, 0x0068, "LO", "Graphic Layer Description" },
    { 0x0070, 0x0080, "CS", "Presentation Label" },
    { 0x0070, 0x0081, "LO", "Presentation Description" },
    { 0x0070, 0x0082, "DA", "Presentation Creation Date" },
    { 0x0070, 0x0083, "TM", "Presentation Creation Time" },
    { 0x0070, 0x0084, "PN", "Presentation Creator's Name" },
    { 0x0087, 0x0010, "CS", "Media Type" },
    { 0x0087, 0x0020, "CS", "Media Location" },
    { 0x0087, 0x0050, "IS", "Estimated Retrieve Time" },
    { 0x0088, 0x0000, "UL", "Storage Group Length" },
    { 0x0088, 0x0130, "SH", "Storage Media FileSet ID" },
    { 0x0088, 0x0140, "UI", "Storage Media FileSet UID" },
    { 0x0088, 0x0200, "SQ", "Icon Image Sequence" },
    { 0x0088, 0x0904, "LO", "Topic Title" },
    { 0x0088, 0x0906, "ST", "Topic Subject" },
    { 0x0088, 0x0910, "LO", "Topic Author" },
    { 0x0088, 0x0912, "LO", "Topic Key Words" },
    { 0x0095, 0x0001, "LT", "Examination Folder ID" },
    { 0x0095, 0x0004, "UL", "Folder Reported Status" },
    { 0x0095, 0x0005, "LT", "Folder Reporting Radiologist" },
    { 0x0095, 0x0007, "LT", "SIENET ISA PLA" },
    { 0x0099, 0x0002, "UL", "Data Object Attributes" },
    { 0x00e1, 0x0001, "US", "Data Dictionary Version" },
    { 0x00e1, 0x0014, "LT", "?" },
    { 0x00e1, 0x0022, "DS", "?" },
    { 0x00e1, 0x0023, "DS", "?" },
    { 0x00e1, 0x0024, "LT", "?" },
    { 0x00e1, 0x0025, "LT", "?" },
    { 0x00e1, 0x0040, "SH", "Offset From CT MR Images" },
    { 0x0193, 0x0002, "DS", "RIS Key" },
    { 0x0307, 0x0001, "UN", "RIS Worklist IMGEF" },
    { 0x0309, 0x0001, "UN", "RIS Report IMGEF" },
    { 0x0601, 0x0000, "SH", "Implementation Version" },
    { 0x0601, 0x0020, "DS", "Relative Table Position" },
    { 0x0601, 0x0021, "DS", "Relative Table Height" },
    { 0x0601, 0x0030, "SH", "Surview Direction" },
    { 0x0601, 0x0031, "DS", "Surview Length" },
    { 0x0601, 0x0050, "SH", "Image View Type" },
    { 0x0601, 0x0070, "DS", "Batch Number" },
    { 0x0601, 0x0071, "DS", "Batch Size" },
    { 0x0601, 0x0072, "DS", "Batch Slice Number" },
    { 0x1000, 0x0000, "xs", "?" },
    { 0x1000, 0x0001, "US", "Run Length Triplet" },
    { 0x1000, 0x0002, "US", "Huffman Table Size" },
    { 0x1000, 0x0003, "US", "Huffman Table Triplet" },
    { 0x1000, 0x0004, "US", "Shift Table Size" },
    { 0x1000, 0x0005, "US", "Shift Table Triplet" },
    { 0x1010, 0x0000, "xs", "?" },
    { 0x1369, 0x0000, "US", "?" },
    { 0x2000, 0x0000, "UL", "Film Session Group Length" },
    { 0x2000, 0x0010, "IS", "Number of Copies" },
    { 0x2000, 0x0020, "CS", "Print Priority" },
    { 0x2000, 0x0030, "CS", "Medium Type" },
    { 0x2000, 0x0040, "CS", "Film Destination" },
    { 0x2000, 0x0050, "LO", "Film Session Label" },
    { 0x2000, 0x0060, "IS", "Memory Allocation" },
    { 0x2000, 0x0500, "SQ", "Referenced Film Box Sequence" },
    { 0x2010, 0x0000, "UL", "Film Box Group Length" },
    { 0x2010, 0x0010, "ST", "Image Display Format" },
    { 0x2010, 0x0030, "CS", "Annotation Display Format ID" },
    { 0x2010, 0x0040, "CS", "Film Orientation" },
    { 0x2010, 0x0050, "CS", "Film Size ID" },
    { 0x2010, 0x0060, "CS", "Magnification Type" },
    { 0x2010, 0x0080, "CS", "Smoothing Type" },
    { 0x2010, 0x0100, "CS", "Border Density" },
    { 0x2010, 0x0110, "CS", "Empty Image Density" },
    { 0x2010, 0x0120, "US", "Min Density" },
    { 0x2010, 0x0130, "US", "Max Density" },
    { 0x2010, 0x0140, "CS", "Trim" },
    { 0x2010, 0x0150, "ST", "Configuration Information" },
    { 0x2010, 0x0500, "SQ", "Referenced Film Session Sequence" },
    { 0x2010, 0x0510, "SQ", "Referenced Image Box Sequence" },
    { 0x2010, 0x0520, "SQ", "Referenced Basic Annotation Box Sequence" },
    { 0x2020, 0x0000, "UL", "Image Box Group Length" },
    { 0x2020, 0x0010, "US", "Image Box Position" },
    { 0x2020, 0x0020, "CS", "Polarity" },
    { 0x2020, 0x0030, "DS", "Requested Image Size" },
    { 0x2020, 0x0110, "SQ", "Preformatted Grayscale Image Sequence" },
    { 0x2020, 0x0111, "SQ", "Preformatted Color Image Sequence" },
    { 0x2020, 0x0130, "SQ", "Referenced Image Overlay Box Sequence" },
    { 0x2020, 0x0140, "SQ", "Referenced VOI LUT Box Sequence" },
    { 0x2030, 0x0000, "UL", "Annotation Group Length" },
    { 0x2030, 0x0010, "US", "Annotation Position" },
    { 0x2030, 0x0020, "LO", "Text String" },
    { 0x2040, 0x0000, "UL", "Overlay Box Group Length" },
    { 0x2040, 0x0010, "SQ", "Referenced Overlay Plane Sequence" },
    { 0x2040, 0x0011, "US", "Referenced Overlay Plane Groups" },
    { 0x2040, 0x0060, "CS", "Overlay Magnification Type" },
    { 0x2040, 0x0070, "CS", "Overlay Smoothing Type" },
    { 0x2040, 0x0080, "CS", "Overlay Foreground Density" },
    { 0x2040, 0x0090, "CS", "Overlay Mode" },
    { 0x2040, 0x0100, "CS", "Threshold Density" },
    { 0x2040, 0x0500, "SQ", "Referenced Overlay Image Box Sequence" },
    { 0x2050, 0x0010, "SQ", "Presentation LUT Sequence" },
    { 0x2050, 0x0020, "CS", "Presentation LUT Shape" },
    { 0x2100, 0x0000, "UL", "Print Job Group Length" },
    { 0x2100, 0x0020, "CS", "Execution Status" },
    { 0x2100, 0x0030, "CS", "Execution Status Info" },
    { 0x2100, 0x0040, "DA", "Creation Date" },
    { 0x2100, 0x0050, "TM", "Creation Time" },
    { 0x2100, 0x0070, "AE", "Originator" },
    { 0x2100, 0x0500, "SQ", "Referenced Print Job Sequence" },
    { 0x2110, 0x0000, "UL", "Printer Group Length" },
    { 0x2110, 0x0010, "CS", "Printer Status" },
    { 0x2110, 0x0020, "CS", "Printer Status Info" },
    { 0x2110, 0x0030, "LO", "Printer Name" },
    { 0x2110, 0x0099, "SH", "Print Queue ID" },
    { 0x3002, 0x0002, "SH", "RT Image Label" },
    { 0x3002, 0x0003, "LO", "RT Image Name" },
    { 0x3002, 0x0004, "ST", "RT Image Description" },
    { 0x3002, 0x000a, "CS", "Reported Values Origin" },
    { 0x3002, 0x000c, "CS", "RT Image Plane" },
    { 0x3002, 0x000e, "DS", "X-Ray Image Receptor Angle" },
    { 0x3002, 0x0010, "DS", "RTImageOrientation" },
    { 0x3002, 0x0011, "DS", "Image Plane Pixel Spacing" },
    { 0x3002, 0x0012, "DS", "RT Image Position" },
    { 0x3002, 0x0020, "SH", "Radiation Machine Name" },
    { 0x3002, 0x0022, "DS", "Radiation Machine SAD" },
    { 0x3002, 0x0024, "DS", "Radiation Machine SSD" },
    { 0x3002, 0x0026, "DS", "RT Image SID" },
    { 0x3002, 0x0028, "DS", "Source to Reference Object Distance" },
    { 0x3002, 0x0029, "IS", "Fraction Number" },
    { 0x3002, 0x0030, "SQ", "Exposure Sequence" },
    { 0x3002, 0x0032, "DS", "Meterset Exposure" },
    { 0x3004, 0x0001, "CS", "DVH Type" },
    { 0x3004, 0x0002, "CS", "Dose Units" },
    { 0x3004, 0x0004, "CS", "Dose Type" },
    { 0x3004, 0x0006, "LO", "Dose Comment" },
    { 0x3004, 0x0008, "DS", "Normalization Point" },
    { 0x3004, 0x000a, "CS", "Dose Summation Type" },
    { 0x3004, 0x000c, "DS", "GridFrame Offset Vector" },
    { 0x3004, 0x000e, "DS", "Dose Grid Scaling" },
    { 0x3004, 0x0010, "SQ", "RT Dose ROI Sequence" },
    { 0x3004, 0x0012, "DS", "Dose Value" },
    { 0x3004, 0x0040, "DS", "DVH Normalization Point" },
    { 0x3004, 0x0042, "DS", "DVH Normalization Dose Value" },
    { 0x3004, 0x0050, "SQ", "DVH Sequence" },
    { 0x3004, 0x0052, "DS", "DVH Dose Scaling" },
    { 0x3004, 0x0054, "CS", "DVH Volume Units" },
    { 0x3004, 0x0056, "IS", "DVH Number of Bins" },
    { 0x3004, 0x0058, "DS", "DVH Data" },
    { 0x3004, 0x0060, "SQ", "DVH Referenced ROI Sequence" },
    { 0x3004, 0x0062, "CS", "DVH ROI Contribution Type" },
    { 0x3004, 0x0070, "DS", "DVH Minimum Dose" },
    { 0x3004, 0x0072, "DS", "DVH Maximum Dose" },
    { 0x3004, 0x0074, "DS", "DVH Mean Dose" },
    { 0x3006, 0x0002, "SH", "Structure Set Label" },
    { 0x3006, 0x0004, "LO", "Structure Set Name" },
    { 0x3006, 0x0006, "ST", "Structure Set Description" },
    { 0x3006, 0x0008, "DA", "Structure Set Date" },
    { 0x3006, 0x0009, "TM", "Structure Set Time" },
    { 0x3006, 0x0010, "SQ", "Referenced Frame of Reference Sequence" },
    { 0x3006, 0x0012, "SQ", "RT Referenced Study Sequence" },
    { 0x3006, 0x0014, "SQ", "RT Referenced Series Sequence" },
    { 0x3006, 0x0016, "SQ", "Contour Image Sequence" },
    { 0x3006, 0x0020, "SQ", "Structure Set ROI Sequence" },
    { 0x3006, 0x0022, "IS", "ROI Number" },
    { 0x3006, 0x0024, "UI", "Referenced Frame of Reference UID" },
    { 0x3006, 0x0026, "LO", "ROI Name" },
    { 0x3006, 0x0028, "ST", "ROI Description" },
    { 0x3006, 0x002a, "IS", "ROI Display Color" },
    { 0x3006, 0x002c, "DS", "ROI Volume" },
    { 0x3006, 0x0030, "SQ", "RT Related ROI Sequence" },
    { 0x3006, 0x0033, "CS", "RT ROI Relationship" },
    { 0x3006, 0x0036, "CS", "ROI Generation Algorithm" },
    { 0x3006, 0x0038, "LO", "ROI Generation Description" },
    { 0x3006, 0x0039, "SQ", "ROI Contour Sequence" },
    { 0x3006, 0x0040, "SQ", "Contour Sequence" },
    { 0x3006, 0x0042, "CS", "Contour Geometric Type" },
    { 0x3006, 0x0044, "DS", "Contour SlabT hickness" },
    { 0x3006, 0x0045, "DS", "Contour Offset Vector" },
    { 0x3006, 0x0046, "IS", "Number of Contour Points" },
    { 0x3006, 0x0050, "DS", "Contour Data" },
    { 0x3006, 0x0080, "SQ", "RT ROI Observations Sequence" },
    { 0x3006, 0x0082, "IS", "Observation Number" },
    { 0x3006, 0x0084, "IS", "Referenced ROI Number" },
    { 0x3006, 0x0085, "SH", "ROI Observation Label" },
    { 0x3006, 0x0086, "SQ", "RT ROI Identification Code Sequence" },
    { 0x3006, 0x0088, "ST", "ROI Observation Description" },
    { 0x3006, 0x00a0, "SQ", "Related RT ROI Observations Sequence" },
    { 0x3006, 0x00a4, "CS", "RT ROI Interpreted Type" },
    { 0x3006, 0x00a6, "PN", "ROI Interpreter" },
    { 0x3006, 0x00b0, "SQ", "ROI Physical Properties Sequence" },
    { 0x3006, 0x00b2, "CS", "ROI Physical Property" },
    { 0x3006, 0x00b4, "DS", "ROI Physical Property Value" },
    { 0x3006, 0x00c0, "SQ", "Frame of Reference Relationship Sequence" },
    { 0x3006, 0x00c2, "UI", "Related Frame of Reference UID" },
    { 0x3006, 0x00c4, "CS", "Frame of Reference Transformation Type" },
    { 0x3006, 0x00c6, "DS", "Frame of Reference Transformation Matrix" },
    { 0x3006, 0x00c8, "LO", "Frame of Reference Transformation Comment" },
    { 0x300a, 0x0002, "SH", "RT Plan Label" },
    { 0x300a, 0x0003, "LO", "RT Plan Name" },
    { 0x300a, 0x0004, "ST", "RT Plan Description" },
    { 0x300a, 0x0006, "DA", "RT Plan Date" },
    { 0x300a, 0x0007, "TM", "RT Plan Time" },
    { 0x300a, 0x0009, "LO", "Treatment Protocols" },
    { 0x300a, 0x000a, "CS", "Treatment Intent" },
    { 0x300a, 0x000b, "LO", "Treatment Sites" },
    { 0x300a, 0x000c, "CS", "RT Plan Geometry" },
    { 0x300a, 0x000e, "ST", "Prescription Description" },
    { 0x300a, 0x0010, "SQ", "Dose ReferenceSequence" },
    { 0x300a, 0x0012, "IS", "Dose ReferenceNumber" },
    { 0x300a, 0x0014, "CS", "Dose Reference Structure Type" },
    { 0x300a, 0x0016, "LO", "Dose ReferenceDescription" },
    { 0x300a, 0x0018, "DS", "Dose Reference Point Coordinates" },
    { 0x300a, 0x001a, "DS", "Nominal Prior Dose" },
    { 0x300a, 0x0020, "CS", "Dose Reference Type" },
    { 0x300a, 0x0021, "DS", "Constraint Weight" },
    { 0x300a, 0x0022, "DS", "Delivery Warning Dose" },
    { 0x300a, 0x0023, "DS", "Delivery Maximum Dose" },
    { 0x300a, 0x0025, "DS", "Target Minimum Dose" },
    { 0x300a, 0x0026, "DS", "Target Prescription Dose" },
    { 0x300a, 0x0027, "DS", "Target Maximum Dose" },
    { 0x300a, 0x0028, "DS", "Target Underdose Volume Fraction" },
    { 0x300a, 0x002a, "DS", "Organ at Risk Full-volume Dose" },
    { 0x300a, 0x002b, "DS", "Organ at Risk Limit Dose" },
    { 0x300a, 0x002c, "DS", "Organ at Risk Maximum Dose" },
    { 0x300a, 0x002d, "DS", "Organ at Risk Overdose Volume Fraction" },
    { 0x300a, 0x0040, "SQ", "Tolerance Table Sequence" },
    { 0x300a, 0x0042, "IS", "Tolerance Table Number" },
    { 0x300a, 0x0043, "SH", "Tolerance Table Label" },
    { 0x300a, 0x0044, "DS", "Gantry Angle Tolerance" },
    { 0x300a, 0x0046, "DS", "Beam Limiting Device Angle Tolerance" },
    { 0x300a, 0x0048, "SQ", "Beam Limiting Device Tolerance Sequence" },
    { 0x300a, 0x004a, "DS", "Beam Limiting Device Position Tolerance" },
    { 0x300a, 0x004c, "DS", "Patient Support Angle Tolerance" },
    { 0x300a, 0x004e, "DS", "Table Top Eccentric Angle Tolerance" },
    { 0x300a, 0x0051, "DS", "Table Top Vertical Position Tolerance" },
    { 0x300a, 0x0052, "DS", "Table Top Longitudinal Position Tolerance" },
    { 0x300a, 0x0053, "DS", "Table Top Lateral Position Tolerance" },
    { 0x300a, 0x0055, "CS", "RT Plan Relationship" },
    { 0x300a, 0x0070, "SQ", "Fraction Group Sequence" },
    { 0x300a, 0x0071, "IS", "Fraction Group Number" },
    { 0x300a, 0x0078, "IS", "Number of Fractions Planned" },
    { 0x300a, 0x0079, "IS", "Number of Fractions Per Day" },
    { 0x300a, 0x007a, "IS", "Repeat Fraction Cycle Length" },
    { 0x300a, 0x007b, "LT", "Fraction Pattern" },
    { 0x300a, 0x0080, "IS", "Number of Beams" },
    { 0x300a, 0x0082, "DS", "Beam Dose Specification Point" },
    { 0x300a, 0x0084, "DS", "Beam Dose" },
    { 0x300a, 0x0086, "DS", "Beam Meterset" },
    { 0x300a, 0x00a0, "IS", "Number of Brachy Application Setups" },
    { 0x300a, 0x00a2, "DS", "Brachy Application Setup Dose Specification Point" },
    { 0x300a, 0x00a4, "DS", "Brachy Application Setup Dose" },
    { 0x300a, 0x00b0, "SQ", "Beam Sequence" },
    { 0x300a, 0x00b2, "SH", "Treatment Machine Name " },
    { 0x300a, 0x00b3, "CS", "Primary Dosimeter Unit" },
    { 0x300a, 0x00b4, "DS", "Source-Axis Distance" },
    { 0x300a, 0x00b6, "SQ", "Beam Limiting Device Sequence" },
    { 0x300a, 0x00b8, "CS", "RT Beam Limiting Device Type" },
    { 0x300a, 0x00ba, "DS", "Source to Beam Limiting Device Distance" },
    { 0x300a, 0x00bc, "IS", "Number of Leaf/Jaw Pairs" },
    { 0x300a, 0x00be, "DS", "Leaf Position Boundaries" },
    { 0x300a, 0x00c0, "IS", "Beam Number" },
    { 0x300a, 0x00c2, "LO", "Beam Name" },
    { 0x300a, 0x00c3, "ST", "Beam Description" },
    { 0x300a, 0x00c4, "CS", "Beam Type" },
    { 0x300a, 0x00c6, "CS", "Radiation Type" },
    { 0x300a, 0x00c8, "IS", "Reference Image Number" },
    { 0x300a, 0x00ca, "SQ", "Planned Verification Image Sequence" },
    { 0x300a, 0x00cc, "LO", "Imaging Device Specific Acquisition Parameters" },
    { 0x300a, 0x00ce, "CS", "Treatment Delivery Type" },
    { 0x300a, 0x00d0, "IS", "Number of Wedges" },
    { 0x300a, 0x00d1, "SQ", "Wedge Sequence" },
    { 0x300a, 0x00d2, "IS", "Wedge Number" },
    { 0x300a, 0x00d3, "CS", "Wedge Type" },
    { 0x300a, 0x00d4, "SH", "Wedge ID" },
    { 0x300a, 0x00d5, "IS", "Wedge Angle" },
    { 0x300a, 0x00d6, "DS", "Wedge Factor" },
    { 0x300a, 0x00d8, "DS", "Wedge Orientation" },
    { 0x300a, 0x00da, "DS", "Source to Wedge Tray Distance" },
    { 0x300a, 0x00e0, "IS", "Number of Compensators" },
    { 0x300a, 0x00e1, "SH", "Material ID" },
    { 0x300a, 0x00e2, "DS", "Total Compensator Tray Factor" },
    { 0x300a, 0x00e3, "SQ", "Compensator Sequence" },
    { 0x300a, 0x00e4, "IS", "Compensator Number" },
    { 0x300a, 0x00e5, "SH", "Compensator ID" },
    { 0x300a, 0x00e6, "DS", "Source to Compensator Tray Distance" },
    { 0x300a, 0x00e7, "IS", "Compensator Rows" },
    { 0x300a, 0x00e8, "IS", "Compensator Columns" },
    { 0x300a, 0x00e9, "DS", "Compensator Pixel Spacing" },
    { 0x300a, 0x00ea, "DS", "Compensator Position" },
    { 0x300a, 0x00eb, "DS", "Compensator Transmission Data" },
    { 0x300a, 0x00ec, "DS", "Compensator Thickness Data" },
    { 0x300a, 0x00ed, "IS", "Number of Boli" },
    { 0x300a, 0x00f0, "IS", "Number of Blocks" },
    { 0x300a, 0x00f2, "DS", "Total Block Tray Factor" },
    { 0x300a, 0x00f4, "SQ", "Block Sequence" },
    { 0x300a, 0x00f5, "SH", "Block Tray ID" },
    { 0x300a, 0x00f6, "DS", "Source to Block Tray Distance" },
    { 0x300a, 0x00f8, "CS", "Block Type" },
    { 0x300a, 0x00fa, "CS", "Block Divergence" },
    { 0x300a, 0x00fc, "IS", "Block Number" },
    { 0x300a, 0x00fe, "LO", "Block Name" },
    { 0x300a, 0x0100, "DS", "Block Thickness" },
    { 0x300a, 0x0102, "DS", "Block Transmission" },
    { 0x300a, 0x0104, "IS", "Block Number of Points" },
    { 0x300a, 0x0106, "DS", "Block Data" },
    { 0x300a, 0x0107, "SQ", "Applicator Sequence" },
    { 0x300a, 0x0108, "SH", "Applicator ID" },
    { 0x300a, 0x0109, "CS", "Applicator Type" },
    { 0x300a, 0x010a, "LO", "Applicator Description" },
    { 0x300a, 0x010c, "DS", "Cumulative Dose Reference Coefficient" },
    { 0x300a, 0x010e, "DS", "Final Cumulative Meterset Weight" },
    { 0x300a, 0x0110, "IS", "Number of Control Points" },
    { 0x300a, 0x0111, "SQ", "Control Point Sequence" },
    { 0x300a, 0x0112, "IS", "Control Point Index" },
    { 0x300a, 0x0114, "DS", "Nominal Beam Energy" },
    { 0x300a, 0x0115, "DS", "Dose Rate Set" },
    { 0x300a, 0x0116, "SQ", "Wedge Position Sequence" },
    { 0x300a, 0x0118, "CS", "Wedge Position" },
    { 0x300a, 0x011a, "SQ", "Beam Limiting Device Position Sequence" },
    { 0x300a, 0x011c, "DS", "Leaf Jaw Positions" },
    { 0x300a, 0x011e, "DS", "Gantry Angle" },
    { 0x300a, 0x011f, "CS", "Gantry Rotation Direction" },
    { 0x300a, 0x0120, "DS", "Beam Limiting Device Angle" },
    { 0x300a, 0x0121, "CS", "Beam Limiting Device Rotation Direction" },
    { 0x300a, 0x0122, "DS", "Patient Support Angle" },
    { 0x300a, 0x0123, "CS", "Patient Support Rotation Direction" },
    { 0x300a, 0x0124, "DS", "Table Top Eccentric Axis Distance" },
    { 0x300a, 0x0125, "DS", "Table Top Eccentric Angle" },
    { 0x300a, 0x0126, "CS", "Table Top Eccentric Rotation Direction" },
    { 0x300a, 0x0128, "DS", "Table Top Vertical Position" },
    { 0x300a, 0x0129, "DS", "Table Top Longitudinal Position" },
    { 0x300a, 0x012a, "DS", "Table Top Lateral Position" },
    { 0x300a, 0x012c, "DS", "Isocenter Position" },
    { 0x300a, 0x012e, "DS", "Surface Entry Point" },
    { 0x300a, 0x0130, "DS", "Source to Surface Distance" },
    { 0x300a, 0x0134, "DS", "Cumulative Meterset Weight" },
    { 0x300a, 0x0180, "SQ", "Patient Setup Sequence" },
    { 0x300a, 0x0182, "IS", "Patient Setup Number" },
    { 0x300a, 0x0184, "LO", "Patient Additional Position" },
    { 0x300a, 0x0190, "SQ", "Fixation Device Sequence" },
    { 0x300a, 0x0192, "CS", "Fixation Device Type" },
    { 0x300a, 0x0194, "SH", "Fixation Device Label" },
    { 0x300a, 0x0196, "ST", "Fixation Device Description" },
    { 0x300a, 0x0198, "SH", "Fixation Device Position" },
    { 0x300a, 0x01a0, "SQ", "Shielding Device Sequence" },
    { 0x300a, 0x01a2, "CS", "Shielding Device Type" },
    { 0x300a, 0x01a4, "SH", "Shielding Device Label" },
    { 0x300a, 0x01a6, "ST", "Shielding Device Description" },
    { 0x300a, 0x01a8, "SH", "Shielding Device Position" },
    { 0x300a, 0x01b0, "CS", "Setup Technique" },
    { 0x300a, 0x01b2, "ST", "Setup TechniqueDescription" },
    { 0x300a, 0x01b4, "SQ", "Setup Device Sequence" },
    { 0x300a, 0x01b6, "CS", "Setup Device Type" },
    { 0x300a, 0x01b8, "SH", "Setup Device Label" },
    { 0x300a, 0x01ba, "ST", "Setup Device Description" },
    { 0x300a, 0x01bc, "DS", "Setup Device Parameter" },
    { 0x300a, 0x01d0, "ST", "Setup ReferenceDescription" },
    { 0x300a, 0x01d2, "DS", "Table Top Vertical Setup Displacement" },
    { 0x300a, 0x01d4, "DS", "Table Top Longitudinal Setup Displacement" },
    { 0x300a, 0x01d6, "DS", "Table Top Lateral Setup Displacement" },
    { 0x300a, 0x0200, "CS", "Brachy Treatment Technique" },
    { 0x300a, 0x0202, "CS", "Brachy Treatment Type" },
    { 0x300a, 0x0206, "SQ", "Treatment Machine Sequence" },
    { 0x300a, 0x0210, "SQ", "Source Sequence" },
    { 0x300a, 0x0212, "IS", "Source Number" },
    { 0x300a, 0x0214, "CS", "Source Type" },
    { 0x300a, 0x0216, "LO", "Source Manufacturer" },
    { 0x300a, 0x0218, "DS", "Active Source Diameter" },
    { 0x300a, 0x021a, "DS", "Active Source Length" },
    { 0x300a, 0x0222, "DS", "Source Encapsulation Nominal Thickness" },
    { 0x300a, 0x0224, "DS", "Source Encapsulation Nominal Transmission" },
    { 0x300a, 0x0226, "LO", "Source IsotopeName" },
    { 0x300a, 0x0228, "DS", "Source Isotope Half Life" },
    { 0x300a, 0x022a, "DS", "Reference Air Kerma Rate" },
    { 0x300a, 0x022c, "DA", "Air Kerma Rate Reference Date" },
    { 0x300a, 0x022e, "TM", "Air Kerma Rate Reference Time" },
    { 0x300a, 0x0230, "SQ", "Application Setup Sequence" },
    { 0x300a, 0x0232, "CS", "Application Setup Type" },
    { 0x300a, 0x0234, "IS", "Application Setup Number" },
    { 0x300a, 0x0236, "LO", "Application Setup Name" },
    { 0x300a, 0x0238, "LO", "Application Setup Manufacturer" },
    { 0x300a, 0x0240, "IS", "Template Number" },
    { 0x300a, 0x0242, "SH", "Template Type" },
    { 0x300a, 0x0244, "LO", "Template Name" },
    { 0x300a, 0x0250, "DS", "Total Reference Air Kerma" },
    { 0x300a, 0x0260, "SQ", "Brachy Accessory Device Sequence" },
    { 0x300a, 0x0262, "IS", "Brachy Accessory Device Number" },
    { 0x300a, 0x0263, "SH", "Brachy Accessory Device ID" },
    { 0x300a, 0x0264, "CS", "Brachy Accessory Device Type" },
    { 0x300a, 0x0266, "LO", "Brachy Accessory Device Name" },
    { 0x300a, 0x026a, "DS", "Brachy Accessory Device Nominal Thickness" },
    { 0x300a, 0x026c, "DS", "Brachy Accessory Device Nominal Transmission" },
    { 0x300a, 0x0280, "SQ", "Channel Sequence" },
    { 0x300a, 0x0282, "IS", "Channel Number" },
    { 0x300a, 0x0284, "DS", "Channel Length" },
    { 0x300a, 0x0286, "DS", "Channel Total Time" },
    { 0x300a, 0x0288, "CS", "Source Movement Type" },
    { 0x300a, 0x028a, "IS", "Number of Pulses" },
    { 0x300a, 0x028c, "DS", "Pulse Repetition Interval" },
    { 0x300a, 0x0290, "IS", "Source Applicator Number" },
    { 0x300a, 0x0291, "SH", "Source Applicator ID" },
    { 0x300a, 0x0292, "CS", "Source Applicator Type" },
    { 0x300a, 0x0294, "LO", "Source Applicator Name" },
    { 0x300a, 0x0296, "DS", "Source Applicator Length" },
    { 0x300a, 0x0298, "LO", "Source Applicator Manufacturer" },
    { 0x300a, 0x029c, "DS", "Source Applicator Wall Nominal Thickness" },
    { 0x300a, 0x029e, "DS", "Source Applicator Wall Nominal Transmission" },
    { 0x300a, 0x02a0, "DS", "Source Applicator Step Size" },
    { 0x300a, 0x02a2, "IS", "Transfer Tube Number" },
    { 0x300a, 0x02a4, "DS", "Transfer Tube Length" },
    { 0x300a, 0x02b0, "SQ", "Channel Shield Sequence" },
    { 0x300a, 0x02b2, "IS", "Channel Shield Number" },
    { 0x300a, 0x02b3, "SH", "Channel Shield ID" },
    { 0x300a, 0x02b4, "LO", "Channel Shield Name" },
    { 0x300a, 0x02b8, "DS", "Channel Shield Nominal Thickness" },
    { 0x300a, 0x02ba, "DS", "Channel Shield Nominal Transmission" },
    { 0x300a, 0x02c8, "DS", "Final Cumulative Time Weight" },
    { 0x300a, 0x02d0, "SQ", "Brachy Control Point Sequence" },
    { 0x300a, 0x02d2, "DS", "Control Point Relative Position" },
    { 0x300a, 0x02d4, "DS", "Control Point 3D Position" },
    { 0x300a, 0x02d6, "DS", "Cumulative Time Weight" },
    { 0x300c, 0x0002, "SQ", "Referenced RT Plan Sequence" },
    { 0x300c, 0x0004, "SQ", "Referenced Beam Sequence" },
    { 0x300c, 0x0006, "IS", "Referenced Beam Number" },
    { 0x300c, 0x0007, "IS", "Referenced Reference Image Number" },
    { 0x300c, 0x0008, "DS", "Start Cumulative Meterset Weight" },
    { 0x300c, 0x0009, "DS", "End Cumulative Meterset Weight" },
    { 0x300c, 0x000a, "SQ", "Referenced Brachy Application Setup Sequence" },
    { 0x300c, 0x000c, "IS", "Referenced Brachy Application Setup Number" },
    { 0x300c, 0x000e, "IS", "Referenced Source Number" },
    { 0x300c, 0x0020, "SQ", "Referenced Fraction Group Sequence" },
    { 0x300c, 0x0022, "IS", "Referenced Fraction Group Number" },
    { 0x300c, 0x0040, "SQ", "Referenced Verification Image Sequence" },
    { 0x300c, 0x0042, "SQ", "Referenced Reference Image Sequence" },
    { 0x300c, 0x0050, "SQ", "Referenced Dose Reference Sequence" },
    { 0x300c, 0x0051, "IS", "Referenced Dose Reference Number" },
    { 0x300c, 0x0055, "SQ", "Brachy Referenced Dose Reference Sequence" },
    { 0x300c, 0x0060, "SQ", "Referenced Structure Set Sequence" },
    { 0x300c, 0x006a, "IS", "Referenced Patient Setup Number" },
    { 0x300c, 0x0080, "SQ", "Referenced Dose Sequence" },
    { 0x300c, 0x00a0, "IS", "Referenced Tolerance Table Number" },
    { 0x300c, 0x00b0, "SQ", "Referenced Bolus Sequence" },
    { 0x300c, 0x00c0, "IS", "Referenced Wedge Number" },
    { 0x300c, 0x00d0, "IS", "Referenced Compensato rNumber" },
    { 0x300c, 0x00e0, "IS", "Referenced Block Number" },
    { 0x300c, 0x00f0, "IS", "Referenced Control Point" },
    { 0x300e, 0x0002, "CS", "Approval Status" },
    { 0x300e, 0x0004, "DA", "Review Date" },
    { 0x300e, 0x0005, "TM", "Review Time" },
    { 0x300e, 0x0008, "PN", "Reviewer Name" },
    { 0x4000, 0x0000, "UL", "Text Group Length" },
    { 0x4000, 0x0010, "LT", "Text Arbitrary" },
    { 0x4000, 0x4000, "LT", "Text Comments" },
    { 0x4008, 0x0000, "UL", "Results Group Length" },
    { 0x4008, 0x0040, "SH", "Results ID" },
    { 0x4008, 0x0042, "LO", "Results ID Issuer" },
    { 0x4008, 0x0050, "SQ", "Referenced Interpretation Sequence" },
    { 0x4008, 0x00ff, "CS", "Report Production Status" },
    { 0x4008, 0x0100, "DA", "Interpretation Recorded Date" },
    { 0x4008, 0x0101, "TM", "Interpretation Recorded Time" },
    { 0x4008, 0x0102, "PN", "Interpretation Recorder" },
    { 0x4008, 0x0103, "LO", "Reference to Recorded Sound" },
    { 0x4008, 0x0108, "DA", "Interpretation Transcription Date" },
    { 0x4008, 0x0109, "TM", "Interpretation Transcription Time" },
    { 0x4008, 0x010a, "PN", "Interpretation Transcriber" },
    { 0x4008, 0x010b, "ST", "Interpretation Text" },
    { 0x4008, 0x010c, "PN", "Interpretation Author" },
    { 0x4008, 0x0111, "SQ", "Interpretation Approver Sequence" },
    { 0x4008, 0x0112, "DA", "Interpretation Approval Date" },
    { 0x4008, 0x0113, "TM", "Interpretation Approval Time" },
    { 0x4008, 0x0114, "PN", "Physician Approving Interpretation" },
    { 0x4008, 0x0115, "LT", "Interpretation Diagnosis Description" },
    { 0x4008, 0x0117, "SQ", "InterpretationDiagnosis Code Sequence" },
    { 0x4008, 0x0118, "SQ", "Results Distribution List Sequence" },
    { 0x4008, 0x0119, "PN", "Distribution Name" },
    { 0x4008, 0x011a, "LO", "Distribution Address" },
    { 0x4008, 0x0200, "SH", "Interpretation ID" },
    { 0x4008, 0x0202, "LO", "Interpretation ID Issuer" },
    { 0x4008, 0x0210, "CS", "Interpretation Type ID" },
    { 0x4008, 0x0212, "CS", "Interpretation Status ID" },
    { 0x4008, 0x0300, "ST", "Impressions" },
    { 0x4008, 0x4000, "ST", "Results Comments" },
    { 0x4009, 0x0001, "LT", "Report ID" },
    { 0x4009, 0x0020, "LT", "Report Status" },
    { 0x4009, 0x0030, "DA", "Report Creation Date" },
    { 0x4009, 0x0070, "LT", "Report Approving Physician" },
    { 0x4009, 0x00e0, "LT", "Report Text" },
    { 0x4009, 0x00e1, "LT", "Report Author" },
    { 0x4009, 0x00e3, "LT", "Reporting Radiologist" },
    { 0x5000, 0x0000, "UL", "Curve Group Length" },
    { 0x5000, 0x0005, "US", "Curve Dimensions" },
    { 0x5000, 0x0010, "US", "Number of Points" },
    { 0x5000, 0x0020, "CS", "Type of Data" },
    { 0x5000, 0x0022, "LO", "Curve Description" },
    { 0x5000, 0x0030, "SH", "Axis Units" },
    { 0x5000, 0x0040, "SH", "Axis Labels" },
    { 0x5000, 0x0103, "US", "Data Value Representation" },
    { 0x5000, 0x0104, "US", "Minimum Coordinate Value" },
    { 0x5000, 0x0105, "US", "Maximum Coordinate Value" },
    { 0x5000, 0x0106, "SH", "Curve Range" },
    { 0x5000, 0x0110, "US", "Curve Data Descriptor" },
    { 0x5000, 0x0112, "US", "Coordinate Start Value" },
    { 0x5000, 0x0114, "US", "Coordinate Step Value" },
    { 0x5000, 0x1001, "CS", "Curve Activation Layer" },
    { 0x5000, 0x2000, "US", "Audio Type" },
    { 0x5000, 0x2002, "US", "Audio Sample Format" },
    { 0x5000, 0x2004, "US", "Number of Channels" },
    { 0x5000, 0x2006, "UL", "Number of Samples" },
    { 0x5000, 0x2008, "UL", "Sample Rate" },
    { 0x5000, 0x200a, "UL", "Total Time" },
    { 0x5000, 0x200c, "xs", "Audio Sample Data" },
    { 0x5000, 0x200e, "LT", "Audio Comments" },
    { 0x5000, 0x2500, "LO", "Curve Label" },
    { 0x5000, 0x2600, "SQ", "CurveReferenced Overlay Sequence" },
    { 0x5000, 0x2610, "US", "CurveReferenced Overlay Group" },
    { 0x5000, 0x3000, "OW", "Curve Data" },
    { 0x6000, 0x0000, "UL", "Overlay Group Length" },
    { 0x6000, 0x0001, "US", "Gray Palette Color Lookup Table Descriptor" },
    { 0x6000, 0x0002, "US", "Gray Palette Color Lookup Table Data" },
    { 0x6000, 0x0010, "US", "Overlay Rows" },
    { 0x6000, 0x0011, "US", "Overlay Columns" },
    { 0x6000, 0x0012, "US", "Overlay Planes" },
    { 0x6000, 0x0015, "IS", "Number of Frames in Overlay" },
    { 0x6000, 0x0022, "LO", "Overlay Description" },
    { 0x6000, 0x0040, "CS", "Overlay Type" },
    { 0x6000, 0x0045, "CS", "Overlay Subtype" },
    { 0x6000, 0x0050, "SS", "Overlay Origin" },
    { 0x6000, 0x0051, "US", "Image Frame Origin" },
    { 0x6000, 0x0052, "US", "Plane Origin" },
    { 0x6000, 0x0060, "LO", "Overlay Compression Code" },
    { 0x6000, 0x0061, "SH", "Overlay Compression Originator" },
    { 0x6000, 0x0062, "SH", "Overlay Compression Label" },
    { 0x6000, 0x0063, "SH", "Overlay Compression Description" },
    { 0x6000, 0x0066, "AT", "Overlay Compression Step Pointers" },
    { 0x6000, 0x0068, "US", "Overlay Repeat Interval" },
    { 0x6000, 0x0069, "US", "Overlay Bits Grouped" },
    { 0x6000, 0x0100, "US", "Overlay Bits Allocated" },
    { 0x6000, 0x0102, "US", "Overlay Bit Position" },
    { 0x6000, 0x0110, "LO", "Overlay Format" },
    { 0x6000, 0x0200, "xs", "Overlay Location" },
    { 0x6000, 0x0800, "LO", "Overlay Code Label" },
    { 0x6000, 0x0802, "US", "Overlay Number of Tables" },
    { 0x6000, 0x0803, "AT", "Overlay Code Table Location" },
    { 0x6000, 0x0804, "US", "Overlay Bits For Code Word" },
    { 0x6000, 0x1001, "CS", "Overlay Activation Layer" },
    { 0x6000, 0x1100, "US", "Overlay Descriptor - Gray" },
    { 0x6000, 0x1101, "US", "Overlay Descriptor - Red" },
    { 0x6000, 0x1102, "US", "Overlay Descriptor - Green" },
    { 0x6000, 0x1103, "US", "Overlay Descriptor - Blue" },
    { 0x6000, 0x1200, "US", "Overlays - Gray" },
    { 0x6000, 0x1201, "US", "Overlays - Red" },
    { 0x6000, 0x1202, "US", "Overlays - Green" },
    { 0x6000, 0x1203, "US", "Overlays - Blue" },
    { 0x6000, 0x1301, "IS", "ROI Area" },
    { 0x6000, 0x1302, "DS", "ROI Mean" },
    { 0x6000, 0x1303, "DS", "ROI Standard Deviation" },
    { 0x6000, 0x1500, "LO", "Overlay Label" },
    { 0x6000, 0x3000, "OW", "Overlay Data" },
    { 0x6000, 0x4000, "LT", "Overlay Comments" },
    { 0x6001, 0x0000, "UN", "?" },
    { 0x6001, 0x0010, "LO", "?" },
    { 0x6001, 0x1010, "xs", "?" },
    { 0x6001, 0x1030, "xs", "?" },
    { 0x6021, 0x0000, "xs", "?" },
    { 0x6021, 0x0010, "xs", "?" },
    { 0x7001, 0x0010, "LT", "Dummy" },
    { 0x7003, 0x0010, "LT", "Info" },
    { 0x7005, 0x0010, "LT", "Dummy" },
    { 0x7000, 0x0004, "ST", "TextAnnotation" },
    { 0x7000, 0x0005, "IS", "Box" },
    { 0x7000, 0x0007, "IS", "ArrowEnd" },
    { 0x7fe0, 0x0000, "UL", "Pixel Data Group Length" },
    { 0x7fe0, 0x0010, "xs", "Pixel Data" },
    { 0x7fe0, 0x0020, "OW", "Coefficients SDVN" },
    { 0x7fe0, 0x0030, "OW", "Coefficients SDHN" },
    { 0x7fe0, 0x0040, "OW", "Coefficients SDDN" },
    { 0x7fe1, 0x0010, "xs", "Pixel Data" },
    { 0x7f00, 0x0000, "UL", "Variable Pixel Data Group Length" },
    { 0x7f00, 0x0010, "xs", "Variable Pixel Data" },
    { 0x7f00, 0x0011, "US", "Variable Next Data Group" },
    { 0x7f00, 0x0020, "OW", "Variable Coefficients SDVN" },
    { 0x7f00, 0x0030, "OW", "Variable Coefficients SDHN" },
    { 0x7f00, 0x0040, "OW", "Variable Coefficients SDDN" },
    { 0x7fe1, 0x0000, "OB", "Binary Data" },
    { 0x7fe3, 0x0000, "LT", "Image Graphics Format Code" },
    { 0x7fe3, 0x0010, "OB", "Image Graphics" },
    { 0x7fe3, 0x0020, "OB", "Image Graphics Dummy" },
    { 0x7ff1, 0x0001, "US", "?" },
    { 0x7ff1, 0x0002, "US", "?" },
    { 0x7ff1, 0x0003, "xs", "?" },
    { 0x7ff1, 0x0004, "IS", "?" },
    { 0x7ff1, 0x0005, "US", "?" },
    { 0x7ff1, 0x0007, "US", "?" },
    { 0x7ff1, 0x0008, "US", "?" },
    { 0x7ff1, 0x0009, "US", "?" },
    { 0x7ff1, 0x000a, "LT", "?" },
    { 0x7ff1, 0x000b, "US", "?" },
    { 0x7ff1, 0x000c, "US", "?" },
    { 0x7ff1, 0x000d, "US", "?" },
    { 0x7ff1, 0x0010, "US", "?" },
    { 0xfffc, 0xfffc, "OB", "Data Set Trailing Padding" },
    { 0xfffe, 0xe000, "!!", "Item" },
    { 0xfffe, 0xe00d, "!!", "Item Delimitation Item" },
    { 0xfffe, 0xe0dd, "!!", "Sequence Delimitation Item" },
    { 0xffff, 0xffff, "xs", "" }
  };

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I s D C M                                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsDCM() returns MagickTrue if the image format type, identified by the
%  magick string, is DCM.
%
%  The format of the ReadDCMImage method is:
%
%      MagickBooleanType IsDCM(const unsigned char *magick,const size_t length)
%
%  A description of each parameter follows:
%
%    o magick: compare image format pattern against these bytes.
%
%    o length: Specifies the length of the magick string.
%
*/
static MagickBooleanType IsDCM(const unsigned char *magick,const size_t length)
{
  if (length < 132)
    return(MagickFalse);
  if (LocaleNCompare((char *) (magick+128),"DICM",4) == 0)
    return(MagickTrue);
  return(MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d D C M I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadDCMImage() reads a Digital Imaging and Communications in Medicine
%  (DICOM) file and returns it.  It allocates the memory necessary for the
%  new Image structure and returns a pointer to the new image.
%
%  The format of the ReadDCMImage method is:
%
%      Image *ReadDCMImage(const ImageInfo *image_info,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static inline size_t MagickMax(const size_t x,const size_t y)
{
  if (x > y)
    return(x);
  return(y);
}

static inline size_t MagickMin(const size_t x,const size_t y)
{
  if (x < y)
    return(x);
  return(y);
}

typedef struct _DCMStreamInfo
{
  size_t
    remaining,
    segment_count;

  ssize_t
    segments[15];

  size_t
    offset_count;

  ssize_t
    *offsets;

  ssize_t
    count;

  int
    byte;
} DCMStreamInfo;

static int ReadDCMByte(DCMStreamInfo *stream_info,Image *image)
{
  if (image->compression != RLECompression)
    return(ReadBlobByte(image));
  if (stream_info->count == 0)
    {
      int
        byte;

      ssize_t
        count;

      if (stream_info->remaining <= 2)
        stream_info->remaining=0;
      else
        stream_info->remaining-=2;
      count=(ssize_t) ReadBlobByte(image);
      byte=ReadBlobByte(image);
      if (count == 128)
        return(0);
      else
        if (count < 128)
          {
            /*
              Literal bytes.
            */
            stream_info->count=count;
            stream_info->byte=(-1);
            return(byte);
          }
        else
          {
            /*
              Repeated bytes.
            */
            stream_info->count=256-count;
            stream_info->byte=byte;
            return(byte);
          }
    }
  stream_info->count--;
  if (stream_info->byte >= 0)
    return(stream_info->byte);
  if (stream_info->remaining > 0)
    stream_info->remaining--;
  return(ReadBlobByte(image));
}

static unsigned short ReadDCMLSBShort(DCMStreamInfo *stream_info,Image *image)
{
  int
    shift;

  unsigned short
    value;

  if (image->compression != RLECompression)
    return(ReadBlobLSBShort(image));
  shift=image->depth < 16 ? 4 : 8;
  value=ReadDCMByte(stream_info,image) | (unsigned short)
    (ReadDCMByte(stream_info,image) << shift);
  return(value);
}

static unsigned short ReadDCMMSBShort(DCMStreamInfo *stream_info,Image *image)
{
  int
    shift;
  unsigned short
    value;

  if (image->compression != RLECompression)
    return(ReadBlobMSBShort(image));
  shift=image->depth < 16 ? 4 : 8;
  value=(ReadDCMByte(stream_info,image) << shift) | (unsigned short)
    ReadDCMByte(stream_info,image);
  return(value);
}

static Image *ReadDCMImage(const ImageInfo *image_info,ExceptionInfo *exception)
{
  char
    explicit_vr[MaxTextExtent],
    implicit_vr[MaxTextExtent],
    magick[MaxTextExtent],
    photometric[MaxTextExtent];

  DCMStreamInfo
    *stream_info;

  Image
    *image;

  int
    *bluemap,
    datum,
    *greenmap,
    *graymap,
    index,
    *redmap;

  MagickBooleanType
    explicit_file,
    use_explicit,
    explicit_retry,
    polarity;

  MagickOffsetType
    offset;

  Quantum
    *scale;

  register IndexPacket
    *indexes;

  register ssize_t
    i,
    x;

  register PixelPacket
    *q;

  register unsigned char
    *p;

  size_t
    bits_allocated,
    bytes_per_pixel,
    colors,
    depth,
    height,
    length,
    mask,
    max_value,
    number_scenes,
    quantum,
    samples_per_pixel,
    signed_data,
    significant_bits,
    status,
    width,
    window_width;

  ssize_t
    count,
    element,
    group,
    scene,
    window_center,
    y;


  unsigned char
    *data;

  /*
    Open image file.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickSignature);
  if (image_info->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  image=AcquireImage(image_info);
  status=OpenBlob(image_info,image,ReadBinaryBlobMode,exception);
  if (status == MagickFalse)
    {
      image=DestroyImageList(image);
      return((Image *) NULL);
    }
  image->depth=8UL;
  /*
    Read DCM preamble.
  */
  stream_info=(DCMStreamInfo *) AcquireMagickMemory(sizeof(*stream_info));
  if (stream_info == (DCMStreamInfo *) NULL)
    ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
  (void) ResetMagickMemory(stream_info,0,sizeof(*stream_info));
  count=ReadBlob(image,128,(unsigned char *) magick);
  if (count != 128)
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");
  count=ReadBlob(image,4,(unsigned char *) magick);
  if ((count != 4) || (LocaleNCompare(magick,"DICM",4) != 0))
    {
      offset=SeekBlob(image,0L,SEEK_SET);
      if (offset < 0)
        ThrowReaderException(CorruptImageError,"ImproperImageHeader");
    }
  /*
    Read DCM Medical image.
  */
  (void) CopyMagickString(photometric,"MONOCHROME1 ",MaxTextExtent);
  bits_allocated=8;
  bytes_per_pixel=1;
  polarity=MagickFalse;
  data=(unsigned char *) NULL;
  depth=8;
  element=0;
  explicit_vr[2]='\0';
  explicit_file=MagickFalse;
  colors=0;
  redmap=(int *) NULL;
  greenmap=(int *) NULL;
  bluemap=(int *) NULL;
  graymap=(int *) NULL;
  height=0;
  max_value=255UL;
  mask=0xffff;
  number_scenes=1;
  samples_per_pixel=1;
  scale=(Quantum *) NULL;
  signed_data=(~0UL);
  significant_bits=0;
  use_explicit=MagickFalse;
  explicit_retry = MagickFalse;
  width=0;
  window_center=0;
  window_width=0;
  for (group=0; (group != 0x7FE0) || (element != 0x0010); )
  {
    /*
      Read a group.
    */
    image->offset=(ssize_t) TellBlob(image);
    group=(ssize_t) ReadBlobLSBShort(image);
    element=(ssize_t) ReadBlobLSBShort(image);
    quantum=0;
    /*
      Find corresponding VR for this group and element.
    */
    for (i=0; dicom_info[i].group < 0xffff; i++)
      if ((group == (ssize_t) dicom_info[i].group) &&
          (element == (ssize_t) dicom_info[i].element))
        break;
    (void) CopyMagickString(implicit_vr,dicom_info[i].vr,MaxTextExtent);
    count=ReadBlob(image,2,(unsigned char *) explicit_vr);
    /*
      Check for "explicitness", but meta-file headers always explicit.
    */
    if ((explicit_file == MagickFalse) && (group != 0x0002))
      explicit_file=(isupper((int) *explicit_vr) != MagickFalse) &&
        (isupper((int) *(explicit_vr+1)) != MagickFalse) ? MagickTrue :
        MagickFalse;
    use_explicit=((group == 0x0002) && (explicit_retry == MagickFalse)) ||
      (explicit_file != MagickFalse) ? MagickTrue : MagickFalse;
    if ((use_explicit != MagickFalse) && (strcmp(implicit_vr,"xs") == 0))
      (void) CopyMagickString(implicit_vr,explicit_vr,MaxTextExtent);
    if ((use_explicit == MagickFalse) || (strcmp(implicit_vr,"!!") == 0))
      {
        offset=SeekBlob(image,(MagickOffsetType) -2,SEEK_CUR);
        if (offset < 0)
          ThrowReaderException(CorruptImageError,"ImproperImageHeader");
        quantum=4;
      }
    else
      {
        /*
          Assume explicit type.
        */
        quantum=2;
        if ((strcmp(explicit_vr,"OB") == 0) ||
            (strcmp(explicit_vr,"UN") == 0) ||
            (strcmp(explicit_vr,"OW") == 0) || (strcmp(explicit_vr,"SQ") == 0))
          {
            (void) ReadBlobLSBShort(image);
            quantum=4;
          }
      }
    datum=0;
    if (quantum == 4)
      datum=(int) ReadBlobLSBLong(image);
    else
      if (quantum == 2)
        datum=(int) ReadBlobLSBShort(image);
    quantum=0;
    length=1;
    if (datum != 0)
      {
        if ((strcmp(implicit_vr,"SS") == 0) ||
            (strcmp(implicit_vr,"US") == 0))
          quantum=2;
        else
          if ((strcmp(implicit_vr,"UL") == 0) ||
              (strcmp(implicit_vr,"SL") == 0) ||
              (strcmp(implicit_vr,"FL") == 0))
            quantum=4;
          else
            if (strcmp(implicit_vr,"FD") != 0)
              quantum=1;
            else
              quantum=8;
        if (datum != ~0)
          length=(size_t) datum/quantum;
        else
          {
            /*
              Sequence and item of undefined length.
            */
            quantum=0;
            length=0;
          }
      }
    if (image_info->verbose != MagickFalse)
      {
        /*
          Display Dicom info.
        */
        if (use_explicit == MagickFalse)
          explicit_vr[0]='\0';
        for (i=0; dicom_info[i].description != (char *) NULL; i++)
          if ((group == (ssize_t) dicom_info[i].group) &&
              (element == (ssize_t) dicom_info[i].element))
            break;
        (void) FormatLocaleFile(stdout,"0x%04lX %4ld %s-%s (0x%04lx,0x%04lx)",
          (unsigned long) image->offset,(long) length,implicit_vr,
          explicit_vr,(unsigned long) group,(unsigned long) element);
        if (dicom_info[i].description != (char *) NULL)
          (void) FormatLocaleFile(stdout," %s",dicom_info[i].description);
        (void) FormatLocaleFile(stdout,": ");
      }
    if ((group == 0x7FE0) && (element == 0x0010))
      {
        if (image_info->verbose != MagickFalse)
          (void) FormatLocaleFile(stdout,"\n");
        break;
      }
    /*
      Allocate space and read an array.
    */
    data=(unsigned char *) NULL;
    if ((length == 1) && (quantum == 1))
      datum=(int) ReadBlobByte(image);
    else
      if ((length == 1) && (quantum == 2))
        datum=(int) ReadBlobLSBShort(image);
      else
        if ((length == 1) && (quantum == 4))
          datum=(int) ReadBlobLSBLong(image);
        else
          if ((quantum != 0) && (length != 0))
            {
              data=(unsigned char *) NULL;
              if (~length >= 1)
                data=(unsigned char *) AcquireQuantumMemory(length+1,quantum*
                  sizeof(*data));
              if (data == (unsigned char *) NULL)
                ThrowReaderException(ResourceLimitError,
                  "MemoryAllocationFailed");
              count=ReadBlob(image,(size_t) quantum*length,data);
              if (count != (ssize_t) (quantum*length))
                {
                  (void) FormatLocaleFile(stdout,"count=%d quantum=%d "
                    "length=%d group=%d\n",(int) count,(int) quantum,(int)
                    length,(int) group);
                   ThrowReaderException(CorruptImageError,
                     "InsufficientImageDataInFile");
                }
              data[length*quantum]='\0';
            }
    switch (group)
    {
      case 0x0002:
      {
        switch (element)
        {
          case 0x0010:
          {
            char
              transfer_syntax[MaxTextExtent];

            /*
              Transfer Syntax.
            */
            if ((datum == 0) && (explicit_retry == MagickFalse))
              {
                explicit_retry=MagickTrue;
                (void) SeekBlob(image,(MagickOffsetType) 0,SEEK_SET);
                group=0;
                element=0;
                if (image_info->verbose != MagickFalse)
                  (void) FormatLocaleFile(stdout,
                    "Corrupted image - trying explicit format\n");
                break;
              }
            *transfer_syntax='\0';
            if (data != (unsigned char *) NULL)
              (void) CopyMagickString(transfer_syntax,(char *) data,
                MaxTextExtent);
            if (image_info->verbose != MagickFalse)
              (void) FormatLocaleFile(stdout,"transfer_syntax=%s\n",
                (const char*) transfer_syntax);
            if (strncmp(transfer_syntax,"1.2.840.10008.1.2",17) == 0)
              {
                int
                  subtype,
                  type;

                type=0;
                subtype=0;
                (void) sscanf(transfer_syntax+17,".%d.%d",&type,&subtype);
                switch (type)
                {
                  case 1:
                  {
                    image->endian=LSBEndian;
                    break;
                  }
                  case 2:
                  {
                    image->endian=MSBEndian;
                    break;
                  }
                  case 4:
                  {
                    if ((subtype >= 80) && (subtype <= 81))
                      image->compression=JPEGCompression;
                    else
                      if ((subtype >= 90) && (subtype <= 93))
                        image->compression=JPEG2000Compression;
                      else
                        image->compression=JPEGCompression;
                    break;
                  }
                  case 5:
                  {
                    image->compression=RLECompression;
                    break;
                  }
                }
              }
            break;
          }
          default:
            break;
        }
        break;
      }
      case 0x0028:
      {
        switch (element)
        {
          case 0x0002:
          {
            /*
              Samples per pixel.
            */
            samples_per_pixel=datum;
            break;
          }
          case 0x0004:
          {
            /*
              Photometric interpretation.
            */
            for (i=0; i < (ssize_t) MagickMin(length,MaxTextExtent-1); i++)
              photometric[i]=(char) data[i];
            photometric[i]='\0';
            polarity=LocaleCompare(photometric,"MONOCHROME1 ") == 0 ?
              MagickTrue : MagickFalse;
            break;
          }
          case 0x0006:
          {
            /*
              Planar configuration.
            */
            if (datum == 1)
              image->interlace=PlaneInterlace;
            break;
          }
          case 0x0008:
          {
            /*
              Number of frames.
            */
            number_scenes=StringToUnsignedLong((char *) data);
            break;
          }
          case 0x0010:
          {
            /*
              Image rows.
            */
            height=datum;
            break;
          }
          case 0x0011:
          {
            /*
              Image columns.
            */
            width=datum;
            break;
          }
          case 0x0100:
          {
            /*
              Bits allocated.
            */
            bits_allocated=datum;
            bytes_per_pixel=1;
            if (datum > 8)
              bytes_per_pixel=2;
            depth=bits_allocated;
            if (depth > 32)
              ThrowReaderException(CorruptImageError,"ImproperImageHeader");
            max_value=(1UL << bits_allocated)-1;
            break;
          }
          case 0x0101:
          {
            /*
              Bits stored.
            */
            significant_bits=datum;
            bytes_per_pixel=1;
            if (significant_bits > 8)
              bytes_per_pixel=2;
            depth=significant_bits;
            if (depth > 32)
              ThrowReaderException(CorruptImageError,"ImproperImageHeader");
            max_value=(1UL << significant_bits)-1;
            mask=(size_t) GetQuantumRange(significant_bits);
            break;
          }
          case 0x0102:
          {
            /*
              High bit.
            */
            break;
          }
          case 0x0103:
          {
            /*
              Pixel representation.
            */
            signed_data=datum;
            break;
          }
          case 0x1050:
          {
            /*
              Visible pixel range: center.
            */
            if (data != (unsigned char *) NULL)
              window_center=StringToLong((char *) data);
            break;
          }
          case 0x1051:
          {
            /*
              Visible pixel range: width.
            */
            if (data != (unsigned char *) NULL)
              window_width=StringToUnsignedLong((char *) data);
            break;
          }
          case 0x1200:
          case 0x3006:
          {
            /*
              Populate graymap.
            */
            if (data == (unsigned char *) NULL)
              break;
            colors=(size_t) (length/bytes_per_pixel);
            datum=(int) colors;
            graymap=(int *) AcquireQuantumMemory((size_t) colors,
              sizeof(*graymap));
            if (graymap == (int *) NULL)
              ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
            for (i=0; i < (ssize_t) colors; i++)
              if (bytes_per_pixel == 1)
                graymap[i]=(int) data[i];
              else
                graymap[i]=(int) ((short *) data)[i];
            break;
          }
          case 0x1201:
          {
            unsigned short
              index;

            /*
              Populate redmap.
            */
            if (data == (unsigned char *) NULL)
              break;
            colors=(size_t) (length/2);
            datum=(int) colors;
            redmap=(int *) AcquireQuantumMemory((size_t) colors,
              sizeof(*redmap));
            if (redmap == (int *) NULL)
              ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
            p=data;
            for (i=0; i < (ssize_t) colors; i++)
            {
              if (image->endian == MSBEndian)
                index=(unsigned short) ((*p << 8) | *(p+1));
              else
                index=(unsigned short) (*p | (*(p+1) << 8));
              redmap[i]=(int) index;
              p+=2;
            }
            break;
          }
          case 0x1202:
          {
            unsigned short
              index;

            /*
              Populate greenmap.
            */
            if (data == (unsigned char *) NULL)
              break;
            colors=(size_t) (length/2);
            datum=(int) colors;
            greenmap=(int *) AcquireQuantumMemory((size_t) colors,
              sizeof(*greenmap));
            if (greenmap == (int *) NULL)
              ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
            p=data;
            for (i=0; i < (ssize_t) colors; i++)
            {
              if (image->endian == MSBEndian)
                index=(unsigned short) ((*p << 8) | *(p+1));
              else
                index=(unsigned short) (*p | (*(p+1) << 8));
              greenmap[i]=(int) index;
              p+=2;
            }
            break;
          }
          case 0x1203:
          {
            unsigned short
              index;

            /*
              Populate bluemap.
            */
            if (data == (unsigned char *) NULL)
              break;
            colors=(size_t) (length/2);
            datum=(int) colors;
            bluemap=(int *) AcquireQuantumMemory((size_t) colors,
              sizeof(*bluemap));
            if (bluemap == (int *) NULL)
              ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
            p=data;
            for (i=0; i < (ssize_t) colors; i++)
            {
              if (image->endian == MSBEndian)
                index=(unsigned short) ((*p << 8) | *(p+1));
              else
                index=(unsigned short) (*p | (*(p+1) << 8));
              bluemap[i]=(int) index;
              p+=2;
            }
            break;
          }
        }
        break;
      }
      case 0x2050:
      {
        switch (element)
        {
          case 0x0020:
          {
            if ((data != (unsigned char *) NULL) &&
                (strncmp((char*) data,"INVERSE", 7) == 0))
              polarity=MagickTrue;
            break;
          }
          default:
            break;
        }
        break;
      }
      default:
        break;
    }
    if (data != (unsigned char *) NULL)
      {
        char
          *attribute;

        for (i=0; dicom_info[i].description != (char *) NULL; i++)
          if ((group == (ssize_t) dicom_info[i].group) &&
              (element == (ssize_t) dicom_info[i].element))
            break;
        attribute=AcquireString("dcm:");
        (void) ConcatenateString(&attribute,dicom_info[i].description);
        for (i=0; i < (ssize_t) MagickMax(length,4); i++)
          if (isprint((int) data[i]) == MagickFalse)
            break;
        if ((i == (ssize_t) length) || (length > 4))
          {
            (void) SubstituteString(&attribute," ","");
            (void) SetImageProperty(image,attribute,(char *) data);
          }
        attribute=DestroyString(attribute);
      }
    if (image_info->verbose != MagickFalse)
      {
        if (data == (unsigned char *) NULL)
          (void) FormatLocaleFile(stdout,"%d\n",datum);
        else
          {
            /*
              Display group data.
            */
            for (i=0; i < (ssize_t) MagickMax(length,4); i++)
              if (isprint((int) data[i]) == MagickFalse)
                break;
            if ((i != (ssize_t) length) && (length <= 4))
              {
                ssize_t
                  j;

                datum=0;
                for (j=(ssize_t) length-1; j >= 0; j--)
                  datum=(256*datum+data[j]);
                (void) FormatLocaleFile(stdout,"%d",datum);
              }
            else
              for (i=0; i < (ssize_t) length; i++)
                if (isprint((int) data[i]) != MagickFalse)
                  (void) FormatLocaleFile(stdout,"%c",data[i]);
                else
                  (void) FormatLocaleFile(stdout,"%c",'.');
            (void) FormatLocaleFile(stdout,"\n");
          }
      }
    if (data != (unsigned char *) NULL)
      data=(unsigned char *) RelinquishMagickMemory(data);
    if (EOFBlob(image) != MagickFalse)
      {
        ThrowFileException(exception,CorruptImageError,"UnexpectedEndOfFile",
          image->filename);
        break;
      }
  }
  if ((width == 0) || (height == 0))
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");
  image->columns=(size_t) width;
  image->rows=(size_t) height;
  if (signed_data == 0xffff)
    signed_data=(size_t) (significant_bits == 16 ? 1 : 0);
  if ((image->compression == JPEGCompression) ||
      (image->compression == JPEG2000Compression))
    {
      Image
        *images;

      ImageInfo
        *read_info;

      int
        c;

      size_t
        length;

      unsigned int
        tag;

      /*
        Read offset table.
      */
      for (i=0; i < (ssize_t) stream_info->remaining; i++)
        (void) ReadBlobByte(image);
      tag=(ReadBlobLSBShort(image) << 16) | ReadBlobLSBShort(image);
      (void) tag;
      length=(size_t) ReadBlobLSBLong(image);
      stream_info->offset_count=length >> 2;
      if (stream_info->offset_count != 0)
        {
          MagickOffsetType
            offset;

          stream_info->offsets=(ssize_t *) AcquireQuantumMemory(
            stream_info->offset_count,sizeof(*stream_info->offsets));
          if (stream_info->offsets == (ssize_t *) NULL)
            ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
          for (i=0; i < (ssize_t) stream_info->offset_count; i++)
            stream_info->offsets[i]=(int) ReadBlobLSBLong(image);
          offset=TellBlob(image);
          for (i=0; i < (ssize_t) stream_info->offset_count; i++)
            stream_info->offsets[i]+=offset;
        }
      /*
        Handle non-native image formats.
      */
      read_info=CloneImageInfo(image_info);
      SetImageInfoBlob(read_info,(void *) NULL,0);
      images=NewImageList();
      for (scene=0; scene < (ssize_t) number_scenes; scene++)
      {
        char
          filename[MaxTextExtent];

        const char
          *property;

        FILE
          *file;

        Image
          *jpeg_image;

        int
          unique_file;

        unsigned int
          tag;

        file=(FILE *) NULL;
        unique_file=AcquireUniqueFileResource(filename);
        if (unique_file != -1)
          file=fdopen(unique_file,"wb");
        if ((unique_file == -1) || (file == (FILE *) NULL))
          {
            ThrowFileException(exception,FileOpenError,
              "UnableToCreateTemporaryFile",filename);
            break;
          }
        tag=(ReadBlobLSBShort(image) << 16) | ReadBlobLSBShort(image);
        length=(size_t) ReadBlobLSBLong(image);
        if (tag == 0xFFFEE0DD)
          break; /* sequence delimiter tag */
        if (tag != 0xFFFEE000)
          ThrowReaderException(CorruptImageError,"ImproperImageHeader");
        for ( ; length != 0; length--)
        {
          c=ReadBlobByte(image);
          if (c == EOF)
            ThrowFileException(exception,CorruptImageError,
              "UnexpectedEndOfFile",image->filename);
          (void) fputc(c,file);
        }
        (void) fclose(file);
        (void) FormatLocaleString(read_info->filename,MaxTextExtent,
          "jpeg:%s",filename);
        if (image->compression == JPEG2000Compression)
          (void) FormatLocaleString(read_info->filename,MaxTextExtent,
            "jp2:%s",filename);
        jpeg_image=ReadImage(read_info,exception);
        if (jpeg_image != (Image *) NULL)
          {
            ResetImagePropertyIterator(image);
            property=GetNextImageProperty(image);
            while (property != (const char *) NULL)
            {
              (void) SetImageProperty(jpeg_image,property,
                GetImageProperty(image,property));
              property=GetNextImageProperty(image);
            }
            AppendImageToList(&images,jpeg_image);
          }
        (void) RelinquishUniqueFileResource(filename);
      }
      read_info=DestroyImageInfo(read_info);
      image=DestroyImage(image);
      return(GetFirstImageInList(images));
    }
  if (depth != (1UL*MAGICKCORE_QUANTUM_DEPTH))
    {
      QuantumAny
        range;

      size_t
        length;

      /*
        Compute pixel scaling table.
      */
      length=(size_t) (GetQuantumRange(depth)+1);
      scale=(Quantum *) AcquireQuantumMemory(length,sizeof(*scale));
      if (scale == (Quantum *) NULL)
        ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
      range=GetQuantumRange(depth);
      for (i=0; i < (ssize_t) (GetQuantumRange(depth)+1); i++)
        scale[i]=ScaleAnyToQuantum((size_t) i,range);
    }
  if (image->compression == RLECompression)
    {
      size_t
        length;

      unsigned int
        tag;

      /*
        Read RLE offset table.
      */
      for (i=0; i < (ssize_t) stream_info->remaining; i++)
        (void) ReadBlobByte(image);
      tag=(ReadBlobLSBShort(image) << 16) | ReadBlobLSBShort(image);
      (void) tag;
      length=(size_t) ReadBlobLSBLong(image);
      stream_info->offset_count=length >> 2;
      if (stream_info->offset_count != 0)
        {
          MagickOffsetType
            offset;

          stream_info->offsets=(ssize_t *) AcquireQuantumMemory(
            stream_info->offset_count,sizeof(*stream_info->offsets));
          if (stream_info->offsets == (ssize_t *) NULL)
            ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
          for (i=0; i < (ssize_t) stream_info->offset_count; i++)
            stream_info->offsets[i]=(int) ReadBlobLSBLong(image);
          offset=TellBlob(image);
          for (i=0; i < (ssize_t) stream_info->offset_count; i++)
            stream_info->offsets[i]+=offset;
        }
    }
  for (scene=0; scene < (ssize_t) number_scenes; scene++)
  {
    if (image_info->ping != MagickFalse)
      break;
    image->columns=(size_t) width;
    image->rows=(size_t) height;
    image->depth=depth;
    image->colorspace=RGBColorspace;
    if ((image->colormap == (PixelPacket *) NULL) && (samples_per_pixel == 1))
      {
        size_t
          one;

        one=1;
        if (colors == 0)
          colors=one << depth;
        if (AcquireImageColormap(image,one << depth) == MagickFalse)
          ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
        if (redmap != (int *) NULL)
          for (i=0; i < (ssize_t) colors; i++)
          {
            index=redmap[i];
            if ((scale != (Quantum *) NULL) && (index <= (int) max_value))
              index=(int) scale[index];
            image->colormap[i].red=index;
          }
        if (greenmap != (int *) NULL)
          for (i=0; i < (ssize_t) colors; i++)
          {
            index=greenmap[i];
            if ((scale != (Quantum *) NULL) && (index <= (int) max_value))
              index=(int) scale[index];
            image->colormap[i].green=index;
          }
        if (bluemap != (int *) NULL)
          for (i=0; i < (ssize_t) colors; i++)
          {
            index=bluemap[i];
            if ((scale != (Quantum *) NULL) && (index <= (int) max_value))
              index=(int) scale[index];
            image->colormap[i].blue=index;
          }
        if (graymap != (int *) NULL)
          for (i=0; i < (ssize_t) colors; i++)
          {
            index=graymap[i];
            if ((scale != (Quantum *) NULL) && (index <= (int) max_value))
              index=(int) scale[index];
            image->colormap[i].red=index;
            image->colormap[i].green=index;
            image->colormap[i].blue=index;
          }
      }
    if (image->compression == RLECompression)
      {
        unsigned int
          tag;

        /*
          Read RLE segment table.
        */
        for (i=0; i < (ssize_t) stream_info->remaining; i++)
          (void) ReadBlobByte(image);
        tag=(ReadBlobLSBShort(image) << 16) | ReadBlobLSBShort(image);
        stream_info->remaining=(size_t) ReadBlobLSBLong(image);
        if ((tag != 0xFFFEE000) || (stream_info->remaining <= 64) ||
            (EOFBlob(image) == MagickTrue))
          ThrowReaderException(CorruptImageError,"ImproperImageHeader");
        stream_info->count=0;
        stream_info->segment_count=ReadBlobLSBLong(image);
        if (stream_info->segment_count > 1)
          {
            bytes_per_pixel=1;
            depth=8;
          }
        for (i=0; i < 15; i++)
          stream_info->segments[i]=(int) ReadBlobLSBLong(image);
        stream_info->remaining-=64;
      }
    if ((samples_per_pixel > 1) && (image->interlace == PlaneInterlace))
      {
        /*
          Convert Planar RGB DCM Medical image to pixel packets.
        */
        for (i=0; i < (ssize_t) samples_per_pixel; i++)
        {
          for (y=0; y < (ssize_t) image->rows; y++)
          {
            q=GetAuthenticPixels(image,0,y,image->columns,1,exception);
            if (q == (PixelPacket *) NULL)
              break;
            for (x=0; x < (ssize_t) image->columns; x++)
            {
              switch ((int) i)
              {
                case 0:
                {
                  SetPixelRed(q,ScaleCharToQuantum((unsigned char)
                    ReadDCMByte(stream_info,image)));
                  break;
                }
                case 1:
                {
                  SetPixelGreen(q,ScaleCharToQuantum((unsigned char)
                    ReadDCMByte(stream_info,image)));
                  break;
                }
                case 2:
                {
                  SetPixelBlue(q,ScaleCharToQuantum((unsigned char)
                    ReadDCMByte(stream_info,image)));
                  break;
                }
                case 3:
                {
                  SetPixelAlpha(q,ScaleCharToQuantum((unsigned char)
                    ReadDCMByte(stream_info,image)));
                  break;
                }
                default:
                  break;
              }
              q++;
            }
            if (SyncAuthenticPixels(image,exception) == MagickFalse)
              break;
            if (image->previous == (Image *) NULL)
              {
                status=SetImageProgress(image,LoadImageTag,(MagickOffsetType) y,
                  image->rows);
                if (status == MagickFalse)
                  break;
              }
          }
        }
      }
    else
      {
        const char
          *option;

        int
          byte;

        LongPixelPacket
          pixel;

        /*
          Convert DCM Medical image to pixel packets.
        */
        byte=0;
        i=0;
        if ((window_center != 0) && (window_width == 0))
          window_width=(size_t) window_center;
        option=GetImageOption(image_info,"dcm:display-range");
        if (option != (const char *) NULL)
          {
            if (LocaleCompare(option,"reset") == 0)
              window_width=0;
          }
        (void) ResetMagickMemory(&pixel,0,sizeof(pixel));
        for (y=0; y < (ssize_t) image->rows; y++)
        {
          q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
          if (q == (PixelPacket *) NULL)
            break;
          indexes=GetAuthenticIndexQueue(image);
          for (x=0; x < (ssize_t) image->columns; x++)
          {
            if (samples_per_pixel == 1)
              {
                int
                  pixel_value;

                if (bytes_per_pixel == 1)
                  pixel_value=polarity != MagickFalse ?
                    ((int) max_value-ReadDCMByte(stream_info,image)) :
                    ReadDCMByte(stream_info,image);
                else
                  if ((bits_allocated != 12) || (significant_bits != 12))
                    {
                      if (image->endian == MSBEndian)
                        pixel_value=(int) (polarity != MagickFalse ? (max_value-
                          ReadDCMMSBShort(stream_info,image)) :
                          ReadDCMMSBShort(stream_info,image));
                      else
                        pixel_value=(int) (polarity != MagickFalse ? (max_value-
                          ReadDCMLSBShort(stream_info,image)) :
                          ReadDCMLSBShort(stream_info,image));
                    }
                  else
                    {
                      if ((i & 0x01) != 0)
                        pixel_value=(ReadDCMByte(stream_info,image) << 8) |
                          byte;
                      else
                        {
                          if (image->endian == MSBEndian)
                            pixel_value=(int) ReadDCMMSBShort(stream_info,
                              image);
                          else
                            pixel_value=(int) ReadDCMLSBShort(stream_info,
                              image);
                          byte=(int) (pixel_value & 0x0f);
                          pixel_value>>=4;
                        }
                      i++;
                    }
                index=pixel_value;
                if (window_width == 0)
                  {
                    if (signed_data == 1)
                      index=pixel_value-32767;
                  }
                else
                  {
                    ssize_t
                      window_max,
                      window_min;

                    window_min=(ssize_t) ceil(window_center-(window_width-1)/
                      2.0-0.5);
                    window_max=(ssize_t) floor(window_center+(window_width-1)/
                      2.0+0.5);
                    if ((ssize_t) pixel_value <= window_min)
                      index=0;
                    else
                      if ((ssize_t) pixel_value > window_max)
                        index=(int) max_value;
                      else
                        index=(int) (max_value*(((pixel_value-window_center-
                          0.5)/(window_width-1))+0.5));
                  }
                index&=mask;
                index=(int) ConstrainColormapIndex(image,(size_t) index);
                SetPixelIndex(indexes+x,index);
                pixel.red=1UL*image->colormap[index].red;
                pixel.green=1UL*image->colormap[index].green;
                pixel.blue=1UL*image->colormap[index].blue;
              }
            else
              {
                if (bytes_per_pixel == 1)
                  {
                    pixel.red=(size_t) ReadDCMByte(stream_info,image);
                    pixel.green=(size_t) ReadDCMByte(stream_info,image);
                    pixel.blue=(size_t) ReadDCMByte(stream_info,image);
                  }
                else
                  {
                    if (image->endian == MSBEndian)
                      {
                        pixel.red=ReadDCMMSBShort(stream_info,image);
                        pixel.green=ReadDCMMSBShort(stream_info,image);
                        pixel.blue=ReadDCMMSBShort(stream_info,image);
                      }
                    else
                      {
                        pixel.red=ReadDCMLSBShort(stream_info,image);
                        pixel.green=ReadDCMLSBShort(stream_info,image);
                        pixel.blue=ReadDCMLSBShort(stream_info,image);
                      }
                  }
                pixel.red&=mask;
                pixel.green&=mask;
                pixel.blue&=mask;
                if (scale != (Quantum *) NULL)
                  {
                    pixel.red=scale[pixel.red];
                    pixel.green=scale[pixel.green];
                    pixel.blue=scale[pixel.blue];
                  }
              }
            SetPixelRed(q,pixel.red);
            SetPixelGreen(q,pixel.green);
            SetPixelBlue(q,pixel.blue);
            q++;
          }
          if (SyncAuthenticPixels(image,exception) == MagickFalse)
            break;
          if (image->previous == (Image *) NULL)
            {
              status=SetImageProgress(image,LoadImageTag,(MagickOffsetType) y,
                image->rows);
              if (status == MagickFalse)
                break;
            }
        }
        if (stream_info->segment_count > 1)
          for (y=0; y < (ssize_t) image->rows; y++)
          {
            q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
            if (q == (PixelPacket *) NULL)
              break;
            indexes=GetAuthenticIndexQueue(image);
            for (x=0; x < (ssize_t) image->columns; x++)
            {
              if (samples_per_pixel == 1)
                {
                  int
                    pixel_value;

                  if (bytes_per_pixel == 1)
                    pixel_value=polarity != MagickFalse ?
                      ((int) max_value-ReadDCMByte(stream_info,image)) :
                      ReadDCMByte(stream_info,image);
                  else
                    if ((bits_allocated != 12) || (significant_bits != 12))
                      {
                        if (image->endian == MSBEndian)
                          pixel_value=(int) (polarity != MagickFalse ?
                            (max_value-ReadDCMMSBShort(stream_info,image)) :
                            ReadDCMMSBShort(stream_info,image));
                        else
                          pixel_value=(int) (polarity != MagickFalse ?
                            (max_value-ReadDCMLSBShort(stream_info,image)) :
                            ReadDCMLSBShort(stream_info,image));
                        if (signed_data == 1)
                          pixel_value=((signed short) pixel_value);
                      }
                    else
                      {
                        if ((i & 0x01) != 0)
                          pixel_value=(ReadDCMByte(stream_info,image) << 8) |
                            byte;
                        else
                          {
                            if (image->endian == MSBEndian)
                              pixel_value=(int) ReadDCMMSBShort(stream_info,
                                image);
                            else
                              pixel_value=(int) ReadDCMLSBShort(stream_info,
                                image);
                            byte=(int) (pixel_value & 0x0f);
                            pixel_value>>=4;
                          }
                        i++;
                      }
                  index=pixel_value;
                  if (window_width == 0)
                    {
                      if (signed_data == 1)
                        index=pixel_value-32767;
                    }
                  else
                    {
                      ssize_t
                        window_max,
                        window_min;

                      window_min=(ssize_t) ceil(window_center-(window_width-1)/
                        2.0-0.5);
                      window_max=(ssize_t) floor(window_center+(window_width-1)/
                        2.0+0.5);
                      if ((ssize_t) pixel_value <= window_min)
                        index=0;
                      else
                        if ((ssize_t) pixel_value > window_max)
                          index=(int) max_value;
                        else
                          index=(int) (max_value*(((pixel_value-window_center-
                            0.5)/(window_width-1))+0.5));
                    }
                  index&=mask;
                  index=(int) ConstrainColormapIndex(image,(size_t) index);
                  SetPixelIndex(indexes+x,(((size_t)
                    GetPixelIndex(indexes+x)) | (((size_t) index) <<
                    8)));
                  pixel.red=1UL*image->colormap[index].red;
                  pixel.green=1UL*image->colormap[index].green;
                  pixel.blue=1UL*image->colormap[index].blue;
                }
              else
                {
                  if (bytes_per_pixel == 1)
                    {
                      pixel.red=(size_t) ReadDCMByte(stream_info,image);
                      pixel.green=(size_t) ReadDCMByte(stream_info,image);
                      pixel.blue=(size_t) ReadDCMByte(stream_info,image);
                    }
                  else
                    {
                      if (image->endian == MSBEndian)
                        {
                          pixel.red=ReadDCMMSBShort(stream_info,image);
                          pixel.green=ReadDCMMSBShort(stream_info,image);
                          pixel.blue=ReadDCMMSBShort(stream_info,image);
                        }
                      else
                        {
                          pixel.red=ReadDCMLSBShort(stream_info,image);
                          pixel.green=ReadDCMLSBShort(stream_info,image);
                          pixel.blue=ReadDCMLSBShort(stream_info,image);
                        }
                    }
                  pixel.red&=mask;
                  pixel.green&=mask;
                  pixel.blue&=mask;
                  if (scale != (Quantum *) NULL)
                    {
                      pixel.red=scale[pixel.red];
                      pixel.green=scale[pixel.green];
                      pixel.blue=scale[pixel.blue];
                    }
                }
              SetPixelRed(q,(((size_t) GetPixelRed(q)) |
                (((size_t) pixel.red) << 8)));
              SetPixelGreen(q,(((size_t) GetPixelGreen(q)) |
                (((size_t) pixel.green) << 8)));
              SetPixelBlue(q,(((size_t) GetPixelBlue(q)) |
                (((size_t) pixel.blue) << 8)));
              q++;
            }
            if (SyncAuthenticPixels(image,exception) == MagickFalse)
              break;
            if (image->previous == (Image *) NULL)
              {
                status=SetImageProgress(image,LoadImageTag,(MagickOffsetType) y,
                  image->rows);
                if (status == MagickFalse)
                  break;
              }
          }
      }
    if (IsGrayImage(image,exception) != MagickFalse)
      (void) SetImageColorspace(image,GRAYColorspace);
    if (EOFBlob(image) != MagickFalse)
      {
        ThrowFileException(exception,CorruptImageError,"UnexpectedEndOfFile",
          image->filename);
        break;
      }
    /*
      Proceed to next image.
    */
    if (image_info->number_scenes != 0)
      if (image->scene >= (image_info->scene+image_info->number_scenes-1))
        break;
    if (scene < (ssize_t) (number_scenes-1))
      {
        /*
          Allocate next image structure.
        */
        AcquireNextImage(image_info,image);
        if (GetNextImageInList(image) == (Image *) NULL)
          {
            image=DestroyImageList(image);
            return((Image *) NULL);
          }
        image=SyncNextImageInList(image);
        status=SetImageProgress(image,LoadImagesTag,TellBlob(image),
          GetBlobSize(image));
        if (status == MagickFalse)
          break;
      }
  }
  /*
    Free resources.
  */
  if (stream_info->offsets != (ssize_t *) NULL)
    stream_info->offsets=(ssize_t *)
      RelinquishMagickMemory(stream_info->offsets);
  stream_info=(DCMStreamInfo *) RelinquishMagickMemory(stream_info);
  if (scale != (Quantum *) NULL)
    scale=(Quantum *) RelinquishMagickMemory(scale);
  if (graymap != (int *) NULL)
    graymap=(int *) RelinquishMagickMemory(graymap);
  if (bluemap != (int *) NULL)
    bluemap=(int *) RelinquishMagickMemory(bluemap);
  if (greenmap != (int *) NULL)
    greenmap=(int *) RelinquishMagickMemory(greenmap);
  if (redmap != (int *) NULL)
    redmap=(int *) RelinquishMagickMemory(redmap);
  (void) CloseBlob(image);
  return(GetFirstImageInList(image));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r D C M I m a g e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterDCMImage() adds attributes for the DCM image format to
%  the list of supported formats.  The attributes include the image format
%  tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterDCMImage method is:
%
%      size_t RegisterDCMImage(void)
%
*/
ModuleExport size_t RegisterDCMImage(void)
{
  MagickInfo
    *entry;

  static const char
    *DCMNote=
    {
      "DICOM is used by the medical community for images like X-rays.  The\n"
      "specification, \"Digital Imaging and Communications in Medicine\n"
      "(DICOM)\", is available at http://medical.nema.org/.  In particular,\n"
      "see part 5 which describes the image encoding (RLE, JPEG, JPEG-LS),\n"
      "and supplement 61 which adds JPEG-2000 encoding."
    };

  entry=SetMagickInfo("DCM");
  entry->decoder=(DecodeImageHandler *) ReadDCMImage;
  entry->magick=(IsImageFormatHandler *) IsDCM;
  entry->adjoin=MagickFalse;
  entry->seekable_stream=MagickTrue;
  entry->description=ConstantString(
    "Digital Imaging and Communications in Medicine image");
  entry->note=ConstantString(DCMNote);
  entry->module=ConstantString("DCM");
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r D C M I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterDCMImage() removes format registrations made by the
%  DCM module from the list of supported formats.
%
%  The format of the UnregisterDCMImage method is:
%
%      UnregisterDCMImage(void)
%
*/
ModuleExport void UnregisterDCMImage(void)
{
  (void) UnregisterMagickInfo("DCM");
}
