/*
 * Copyright (c) 2011, Intel Corporation.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <fcntl.h>
#include <errno.h>
#include <vector>
#include <unistd.h>
#include "tnvmeParsers.h"
#include "Cmds/identify.h"

struct AttribXML {
    string name;
    string value;
};


/**
 * A function to specifically handle parsing cmd lines of the form
 * "--skiptest <filename>".
 * @param skipTest Pass a structure to populate with parsing results
 * @param optarg Pass the 'optarg' argument from the getopt_long() API.
 * @return true upon successful parsing, otherwise false.
 */
bool
ParseSkipTestCmdLine(vector<TestRef> &skipTest, const char *optarg)
{
    int fd;
    ssize_t numRead;
    char buffer[80];
    string contents;
    string line;
    TestTarget tmp;


    if ((fd = open(optarg, O_RDWR)) == -1) {
        LOG_ERR("File=%s: %s", optarg, strerror(errno));
        return false;
    }
    while ((numRead = read(fd, buffer, (sizeof(buffer)-1))) != -1) {
        if (numRead == 0)
            break;
        buffer[numRead] = '\0';
        contents += buffer;
    }
    if (numRead == -1) {
        LOG_ERR("File=%s: %s", optarg, strerror(errno));
        return false;
    }

    // Parse and make sense of the file's contents
    for (size_t i = 0; i < contents.size(); i++) {
        bool processThisLine = false;

        if (contents[i] == '#') {
            // Comment start; goes until the end of this line
            for (size_t j = i; j < contents.size(); j++) {
                if ((contents[j] == '\n') || (contents[j] == '\r')) {
                    i = j;
                    processThisLine = true;
                    break;
                }
            }
        } else if ((contents[i] == '\n') || (contents[i] == '\r')) {
            processThisLine = true;
        } else {
            if (isalnum(contents[i]) ||
                (contents[i] == '.') ||
                (contents[i] == ':')) {
                line += contents[i];
            }
        }

        if (processThisLine) {
            if (line.size()) {
                if (ParseTargetCmdLine(tmp, line.c_str()) == false) {
                    close(fd);
                    return false;
                }

                TestRef skipThis = tmp.t;
                skipTest.push_back(skipThis);
                line = "";
            }
        }
    }

    // Report what tests will be skipped
    string output = "Control will skip test case(s): ";
    char work[20];
    for (size_t i = 0; i < skipTest.size(); i++ ) {
        if ((skipTest[i].xLev == UINT_MAX) ||
            (skipTest[i].yLev == UINT_MAX) ||
            (skipTest[i].zLev == UINT_MAX)) {

            snprintf(work, sizeof(work), "%ld:ALL.ALL.ALL, ",
                skipTest[i].group);
        } else {
            snprintf(work, sizeof(work), "%ld:%ld.%ld.%ld, ",
                skipTest[i].group, skipTest[i].xLev, skipTest[i].yLev,
                skipTest[i].zLev);
        }
        output += work;
    }
    LOG_NRM("%s", output.c_str());

    close(fd);
    return true;
}


/**
 * A function to seek for a specified node, i.e. starting element, in a
 * provided XML file.
 * @param xmlFile Pass the file to seek within
 * @param nodeName Pass the the name of the node to find
 * @param nodeDepth Pass the depth that the node should be placed
 * @param nodeVal Returns the value of the sought after node
 * @param attr Returns all the attributesof the sought after node
 * @return true upon successful seeking, otherwise false.
 */
bool
SeekSpecificXMLNode(xmlpp::TextReader &xmlFile, string nodeName, int nodeDepth,
    string &nodeVal, vector<AttribXML> &attr)
{
    try {
        nodeVal = "";
        attr.clear();


        LOG_DBG("New XML search for XML node: \"%s\"", nodeName.c_str());
        while(xmlFile.read()) {
            LOG_DBG("XML compare: \"%s\" with \"%s\"", nodeName.c_str(),
                xmlFile.get_name().c_str());
            if (strcmp(nodeName.c_str(), xmlFile.get_name().c_str()) == 0) {

                LOG_DBG("Found \"%s\" @ XML depth: %d", nodeName.c_str(),
                    xmlFile.get_depth());

                if (xmlFile.get_node_type() == xmlpp::TextReader::EndElement) {
                    LOG_DBG("Node is end element, continuing");
                    continue;
                }

                if (xmlFile.get_depth() == nodeDepth) {

                    if(xmlFile.has_value()) {
                        nodeVal = xmlFile.get_value();
                        LOG_DBG("XML value: \"%s\"", nodeVal.c_str());
                    }

                    if(xmlFile.has_attributes()) {
                        xmlFile.move_to_first_attribute();
                        do {
                            AttribXML xmlAttrib;
                            xmlAttrib.name = xmlFile.get_name();
                            xmlAttrib.value = xmlFile.get_value();
                            attr.push_back(xmlAttrib);
                            LOG_DBG("XML attrib: \"%s\"=\"%s\"",
                                xmlAttrib.name.c_str(),
                                xmlAttrib.value.c_str());

                        } while(xmlFile.move_to_next_attribute());
                        xmlFile.move_to_element();
                    }
                    return true;  // Found the node
                }
            }
        }
    }
    catch(const std::exception& e)
    {
        LOG_ERR("While seeking XML node %s: %s", nodeName.c_str(), e.what());
        return false;
    }

    LOG_DBG("XML node \"%s\' not found", nodeName.c_str());
    return false;
}


/**
 * A function to extract the expected nodes for a --golden type file
 * @param xmlFile Pass the file to seek within
 * @param cmd Returns the filled in part of the structure
 * @param nodeName Pass the name of the node being parsed.
 * @return true upon successful parsing, otherwise false.
 */
bool ExtractIdentifyXMLValue(xmlpp::TextReader &xmlFile, IdentifyDUT &cmd,
    string nodeName)
{
    if (strcmp(nodeName.c_str(), "nsid") == 0) {
        if (xmlFile.get_node_type() == xmlpp::TextReader::Element) {
            LOG_DBG("Found <%s>: processing", nodeName.c_str());
            xmlFile.read();
            if (xmlFile.get_node_type() == xmlpp::TextReader::Text) {
                cmd.nsid = strtoul(xmlFile.get_value().c_str(), NULL, 10);
                LOG_DBG("Identify.nsid = 0x%02X", cmd.nsid);
                return true;
            }
        }
    } else if (strcmp(nodeName.c_str(), "cns") == 0) {
        if (xmlFile.get_node_type() == xmlpp::TextReader::Element) {
            LOG_DBG("Found <%s>: processing", nodeName.c_str());
            xmlFile.read();
            if (xmlFile.get_node_type() == xmlpp::TextReader::Text) {
                cmd.cns = strtoul(xmlFile.get_value().c_str(), NULL, 10);
                LOG_DBG("Identify.cns = 0x%02X", cmd.cns);
                return true;
            }
        }
    }

    LOG_DBG("Found unsupported node type: %d", xmlFile.get_node_type());
    return false;
}


/**
 * A function to extract the expected nodes for a --format type file
 * @param xmlFile Pass the file to seek within
 * @param cmd Returns the filled in part of the structure
 * @param nodeName Pass the name of the node being parsed.
 * @return true upon successful parsing, otherwise false.
 */
bool
ExtractFormatXMLValue(xmlpp::TextReader &xmlFile, FormatDUT &cmd, string nodeName)
{
    if (strcmp(nodeName.c_str(), "ses") == 0) {
        if (xmlFile.get_node_type() == xmlpp::TextReader::Element) {
            LOG_DBG("Found <%s>: processing", nodeName.c_str());
            xmlFile.read();
            if (xmlFile.get_node_type() == xmlpp::TextReader::Text) {
                cmd.ses = strtoul(xmlFile.get_value().c_str(), NULL, 16);
                LOG_DBG("Format.ses = 0x%02X", cmd.ses);
                return true;
            }
        }
    } else if (strcmp(nodeName.c_str(), "pil") == 0) {
        if (xmlFile.get_node_type() == xmlpp::TextReader::Element) {
            LOG_DBG("Found <%s>: processing", nodeName.c_str());
            xmlFile.read();
            if (xmlFile.get_node_type() == xmlpp::TextReader::Text) {
                cmd.pil = strtoul(xmlFile.get_value().c_str(), NULL, 16);
                LOG_DBG("Format.pil = 0x%02X", cmd.pil);
                return true;
            }
        }
    } else if (strcmp(nodeName.c_str(), "pi") == 0) {
        if (xmlFile.get_node_type() == xmlpp::TextReader::Element) {
            LOG_DBG("Found <%s>: processing", nodeName.c_str());
            xmlFile.read();
            if (xmlFile.get_node_type() == xmlpp::TextReader::Text) {
                cmd.pi = strtoul(xmlFile.get_value().c_str(), NULL, 16);
                LOG_DBG("Format.pi = 0x%02X", cmd.pi);
                return true;
            }
        }
    } else if (strcmp(nodeName.c_str(), "ms") == 0) {
        if (xmlFile.get_node_type() == xmlpp::TextReader::Element) {
            LOG_DBG("Found <%s>: processing", nodeName.c_str());
            xmlFile.read();
            if (xmlFile.get_node_type() == xmlpp::TextReader::Text) {
                cmd.ms = strtoul(xmlFile.get_value().c_str(), NULL, 16);
                LOG_DBG("Format.ms = 0x%02X", cmd.ms);
                return true;
            }
        }
    } else if (strcmp(nodeName.c_str(), "lbaf") == 0) {
        if (xmlFile.get_node_type() == xmlpp::TextReader::Element) {
            LOG_DBG("Found <%s>: processing", nodeName.c_str());
            xmlFile.read();
            if (xmlFile.get_node_type() == xmlpp::TextReader::Text) {
                cmd.lbaf = strtoul(xmlFile.get_value().c_str(), NULL, 16);
                LOG_DBG("Format.lbaf = 0x%02X", cmd.lbaf);
                return true;
            }
        }
    }

    LOG_DBG("Found unsupported node type: %d", xmlFile.get_node_type());
    return false;
}


/**
 * A function to specifically handle parsing cmd lines of the form
 * "--golden <filename>".
 * @param golden Pass a structure to populate with parsing results
 * @param optarg Pass the 'optarg' argument from the getopt_long() API.
 * @return true upon successful parsing, otherwise false.
 */
bool
ParseGoldenCmdLine(Golden &golden, const char *optarg)
{
    bool allOK = false;
    string nodeVal, nodeName;
    vector<AttribXML> attr;
    IdentifyDUT cmd;
    size_t colLoc = 0;
    string inFileName = optarg;

    if ((int)(colLoc = inFileName.find_first_of(':')) > 0){
        golden.outputFile = inFileName.substr(colLoc + 1, inFileName.length());
        inFileName = inFileName.substr(0, colLoc);
    }

    try
    {
        golden.req = false;
        golden.cmds.clear();
        xmlpp::TextReader xmlFile(inFileName.c_str());

        if (SeekSpecificXMLNode(xmlFile, "identify", 0, nodeVal, attr) == false)
            return false;

        while (SeekSpecificXMLNode(xmlFile, "cmd", 1, nodeVal, attr)) {
            memset(&cmd, 0, sizeof(cmd));

            if (SeekSpecificXMLNode(xmlFile, "dw1", 2, nodeVal, attr)) {
                // Seek for value elements defining the cmd's params
                while(xmlFile.read()) {
                    nodeName = xmlFile.get_name();
                    LOG_DBG("XML parser found: \"%s\" of type %d",
                        nodeName.c_str(), xmlFile.get_node_type());

                    switch (xmlFile.get_node_type()) {
                    case xmlpp::TextReader::Comment:
                        LOG_DBG("Node is a comment, continuing");
                        continue;

                    case xmlpp::TextReader::SignificantWhitespace:
                        LOG_DBG("Node is a whitespace, continuing");
                        continue;

                    case xmlpp::TextReader::Element:
                        if (ExtractIdentifyXMLValue(xmlFile, cmd, nodeName)
                            == false) {
                            return false;
                        }
                        break;

                    case xmlpp::TextReader::EndElement:
                        if (strcmp("dw1", nodeName.c_str()) == 0) {
                            goto EXIT_DW1_SEARCH;
                        } else {
                            LOG_DBG(
                                "</%s> decoded as end element: skipping",
                                nodeName.c_str());
                            continue;
                        }
                        break;

                    default:
                        LOG_ERR("Found unsupported node type");
                        return false;
                    }
                }
            }
EXIT_DW1_SEARCH:

            if (SeekSpecificXMLNode(xmlFile, "dw10", 2, nodeVal, attr)) {
                // Seek for value elements defining the cmd's params
                while(xmlFile.read()) {
                    nodeName = xmlFile.get_name();
                    LOG_DBG("XML parser found: \"%s\" of type %d",
                        nodeName.c_str(), xmlFile.get_node_type());

                    switch (xmlFile.get_node_type()) {
                    case xmlpp::TextReader::Comment:
                        LOG_DBG("Node is a comment, continuing");
                        continue;

                    case xmlpp::TextReader::SignificantWhitespace:
                        LOG_DBG("Node is a whitespace, continuing");
                        continue;

                    case xmlpp::TextReader::Element:
                        if (ExtractIdentifyXMLValue(xmlFile, cmd, nodeName)
                            == false) {
                            return false;
                        }
                        break;

                    case xmlpp::TextReader::EndElement:
                        if (strcmp("dw10", nodeName.c_str()) == 0) {
                            goto EXIT_DW10_SEARCH;
                        } else {
                            LOG_DBG(
                                "</%s> decoded as end element: skipping",
                                nodeName.c_str());
                            continue;
                        }
                        break;

                    default:
                        LOG_ERR("Found unsupported node type");
                        return false;
                    }
                }
            }
EXIT_DW10_SEARCH:


            if (SeekSpecificXMLNode(xmlFile, "prp", 2, nodeVal, attr)) {
                // Seek for value elements defining the cmd's params
                while(xmlFile.read()) {
                    nodeName = xmlFile.get_name();
                    LOG_DBG("XML parser found: \"%s\" of type %d",
                        nodeName.c_str(), xmlFile.get_node_type());

                    switch (xmlFile.get_node_type()) {
                    case xmlpp::TextReader::Comment:
                        LOG_DBG("Node is a comment, continuing");
                        continue;

                    case xmlpp::TextReader::SignificantWhitespace:
                        LOG_DBG("Node is a whitespace, continuing");
                        continue;

                    case xmlpp::TextReader::Element:
                        LOG_ERR("Node is a element, unexpected");
                        return false;

                    case xmlpp::TextReader::EndElement:
                        if (strcmp("prp", nodeName.c_str()) == 0) {
                            goto EXIT_PRP_SEARCH;
                        } else {
                            LOG_DBG(
                                "</%s> decoded as end element: skipping",
                                nodeName.c_str());
                            continue;
                        }
                        break;

                    case xmlpp::TextReader::Text:
                        {
                            uint8_t work;
                            char *endPtr = NULL;
                            char *startPtr = NULL;
                            work = (uint8_t)strtoul(xmlFile.get_value().c_str(),
                                &endPtr, 16);
                            while ((*endPtr != '\0') && (startPtr != endPtr)) {
                                cmd.raw.push_back(work);
                                startPtr = endPtr;
                                work = (uint8_t)strtoul(startPtr, &endPtr, 16);
                            }
                            if (cmd.raw.size() != Identify::IDEAL_DATA_SIZE) {
                                LOG_ERR("Identify payload must be 4KB: %ld",
                                    cmd.raw.size());
                                return false;
                            }
                        }
                        break;

                    default:
                        LOG_ERR("Found unsupported node type");
                        return false;
                    }
                }
            }
EXIT_PRP_SEARCH:


            if (SeekSpecificXMLNode(xmlFile, "mask", 2, nodeVal, attr)) {
                // Seek for value elements defining the cmd's params
                while(xmlFile.read()) {
                    nodeName = xmlFile.get_name();
                    LOG_DBG("XML parser found: \"%s\" of type %d",
                        nodeName.c_str(), xmlFile.get_node_type());

                    switch (xmlFile.get_node_type()) {
                    case xmlpp::TextReader::Comment:
                        LOG_DBG("Node is a comment, continuing");
                        continue;

                    case xmlpp::TextReader::SignificantWhitespace:
                        LOG_DBG("Node is a whitespace, continuing");
                        continue;

                    case xmlpp::TextReader::Element:
                        LOG_ERR("Node is a element, unexpected");
                        return false;

                    case xmlpp::TextReader::EndElement:
                        if (strcmp("mask", nodeName.c_str()) == 0) {
                            goto EXIT_MASK_SEARCH;
                        } else {
                            LOG_DBG(
                                "</%s> decoded as end element: skipping",
                                nodeName.c_str());
                            continue;
                        }
                        break;

                    case xmlpp::TextReader::Text:
                        {
                            uint8_t work;
                            char *endPtr = NULL;
                            char *startPtr = NULL;
                            work = (uint8_t)strtoul(xmlFile.get_value().c_str(),
                                &endPtr, 16);
                            while ((*endPtr != '\0') && (startPtr != endPtr)) {
                                cmd.mask.push_back(work);
                                startPtr = endPtr;
                                work = (uint8_t)strtoul(startPtr, &endPtr, 16);
                            }
                            if (cmd.mask.size() != Identify::IDEAL_DATA_SIZE) {
                                LOG_ERR("Identify mask must be 4KB: %ld",
                                    cmd.mask.size());
                                return false;
                            }
                        }
                        break;

                    default:
                        LOG_ERR("Found unsupported node type");
                        return false;
                    }
                }
            }
EXIT_MASK_SEARCH:

            // Finalize this cmd, possible among many more
            golden.cmds.push_back(cmd);
            allOK = true;
        }
    }
    catch(const std::exception& e)
    {
        LOG_ERR("While processing file %s: %s", inFileName.c_str(), e.what());
        return false;
    }

    if (allOK == false) {
        LOG_ERR("Unable to completely process file %s", inFileName.c_str());
        return false;
    }

#ifdef DEBUG
    for (size_t i = 0; i < golden.cmds.size(); i++) {
        LOG_DBG("Identify cmd #%ld", i);
        LOG_DBG("  Identify:DW1.nsid = 0x%02x", golden.cmds[i].nsid);
        LOG_DBG("  Identify.DW10.cns = %c", golden.cmds[i].cns ? 'T' : 'F');
        LOG_DBG("  sizeof(Identify.raw) = %ld", golden.cmds[i].raw.size());
        LOG_DBG("  sizeof(Identify.mask) = %ld", golden.cmds[i].mask.size());
    }
#endif
    golden.req = true;
    return true;
}


/**
 * A function to specifically handle parsing cmd lines of the form
 * "--fwimage <filename>".
 * @param fwimage Pass a structure to populate with parsing results
 * @param optarg Pass the 'optarg' argument from the getopt_long() API.
 * @return true upon successful parsing, otherwise false.
 */
bool
ParseFWImageCmdLine(FWImage &fwimage, const char *optarg)
{
    int fd;
    ssize_t numRead;
    uint8_t buffer[1];


    fwimage.data.empty();
    if ((fd = open(optarg, O_RDWR)) == -1) {
        LOG_ERR("File=%s: %s", optarg, strerror(errno));
        return false;
    }
    while ((numRead = read(fd, buffer, sizeof(buffer))) != -1) {
        if (numRead == 0)
            break;
        fwimage.data.push_back(buffer[0]);
    }
    if (numRead == -1) {
        LOG_ERR("File=%s: %s", optarg, strerror(errno));
        return false;
    }

    close(fd);
    fwimage.req = true;
    return true;
}


/**
 * A function to specifically handle parsing cmd lines of the form
 * "--format <filename>".
 * @param format Pass a structure to populate with parsing results
 * @param optarg Pass the 'optarg' argument from the getopt_long() API.
 * @return true upon successful parsing, otherwise false.
 */
bool
ParseFormatCmdLine(Format &format, const char *optarg)
{
    bool allOK = false;
    string nodeVal, nodeName;
    vector<AttribXML> attr;
    FormatDUT cmd;

    try
    {
        format.req = false;
        format.cmds.clear();
        xmlpp::TextReader xmlFile(optarg);

        if (SeekSpecificXMLNode(xmlFile, "format", 0, nodeVal, attr) == false)
            return false;

        while (SeekSpecificXMLNode(xmlFile, "namespace", 1, nodeVal, attr)) {
            memset(&cmd, 0, sizeof(cmd));

            // Retrieve the namespace ID to issue a cmd at
            if (attr.size() != 1) {
                LOG_ERR("\"namespace\" node is required to have an attribute");
                return false;
            } else if (strcmp("id", attr[0].name.c_str()) != 0) {
                LOG_ERR("\"namespace\" node must have one \"id\" attribute");
                return false;
            }
            cmd.nsid = strtoul(attr[0].value.c_str(), NULL, 10);

            if (SeekSpecificXMLNode(xmlFile, "cmd", 2, nodeVal, attr)) {
                // This node shouldn't have any value of attributes, ignoring
                if (SeekSpecificXMLNode(xmlFile, "dw10", 3, nodeVal, attr)) {

                    // Seek for value elements defining the cmd's params
                    while(xmlFile.read()) {
                        nodeName = xmlFile.get_name();
                        LOG_DBG("XML parser found: \"%s\" of type %d",
                            nodeName.c_str(), xmlFile.get_node_type());

                        switch (xmlFile.get_node_type()) {
                        case xmlpp::TextReader::Comment:
                            LOG_DBG("Node is a comment, continuing");
                            continue;

                        case xmlpp::TextReader::SignificantWhitespace:
                            LOG_DBG("Node is a whitespace, continuing");
                            continue;

                        case xmlpp::TextReader::Element:
                            if (ExtractFormatXMLValue(xmlFile, cmd, nodeName)
                                == false) {
                                return false;
                            }
                            break;

                        case xmlpp::TextReader::EndElement:
                            if (strcmp("dw10", nodeName.c_str()) == 0) {
                                goto EXIT_VALUE_SEARCH;
                            } else {
                                LOG_DBG(
                                    "</%s> decoded as end element: skipping",
                                    nodeName.c_str());
                                continue;
                            }
                            break;

                        default:
                            LOG_ERR("Found unsupported node type");
                            return false;
                        }
                    }
EXIT_VALUE_SEARCH:
                    format.cmds.push_back(cmd);
                    allOK = true;
                }
            }
        }
    }
    catch(const std::exception& e)
    {
        LOG_ERR("While processing file %s: %s", optarg, e.what());
        return false;
    }

    if (allOK == false) {
        LOG_ERR("Unable to completely process file %s", optarg);
        return false;
    }

#ifdef DEBUG
    for (size_t i = 0; i < format.cmds.size(); i++) {
        LOG_DBG("Namespace: %d", format.cmds[i].nsid);
        LOG_DBG("  FormatNVM:DW10.ses = 0x%02x", format.cmds[i].ses);
        LOG_DBG("  FormatNVM:DW10.pil = %c", format.cmds[i].pil ? 'T' : 'F');
        LOG_DBG("  FormatNVM:DW10.pi = 0x%02x", format.cmds[i].pi);
        LOG_DBG("  FormatNVM:DW10.ms = %c", format.cmds[i].ms ? 'T' : 'F');
        LOG_DBG("  FormatNVM:DW10.lbaf = 0x%02x", format.cmds[i].lbaf);
    }
#endif
    format.req = true;
    return true;
}


/**
 * A function to specifically handle parsing cmd lines of the form
 * "[<grp> | <grp>:<test>]" where the absent of the optional parameters means
 * a user is specifying "all" things.
 * @param target Pass a structure to populate with parsing results
 * @param optarg Pass the 'optarg' argument from the getopt_long() API.
 * @return true upon successful parsing, otherwise false.
 */
bool
ParseTargetCmdLine(TestTarget &target, const char *optarg)
{
    size_t ulwork;
    string swork;
    char *endptr;
    long tmp;

    target.req = true;
    target.t.group = UINT_MAX;
    target.t.xLev = UINT_MAX;
    target.t.yLev = UINT_MAX;
    target.t.zLev = UINT_MAX;

    if (optarg == NULL) {
        // User specified to run all test within all all groups
        return true;
    }

    swork = optarg;
    if ((ulwork = swork.find(":", 0)) == string::npos) {
        // User specified format <grp> only
        tmp = strtol(swork.c_str(), &endptr, 10);
        if (*endptr != '\0') {
            LOG_ERR("Unrecognized format <grp>=%s", optarg);
            return false;
        } else if (tmp < 0) {
            LOG_ERR("<grp> values < 0 are not supported");
            return false;
        }
        target.t.group = tmp;

    } else {
        // Specified format <grp>:<test>
        tmp = strtol(swork.substr(0, swork.size()).c_str(), &endptr, 10);
        if (*endptr != ':') {
            LOG_ERR("Missing ':' character in format string");
            return false;
        } else if (tmp < 0) {
            LOG_ERR("<grp> values < 0 are not supported");
            return false;
        }
        target.t.group = tmp;

        // Find xLev component of <test>
        swork = swork.substr(swork.find_first_of(':') + 1, swork.length());
        if (swork.length() == 0) {
            LOG_ERR("Missing <test> format string");
            return false;
        }

        tmp = strtol(swork.substr(0, swork.size()).c_str(), &endptr, 10);
        if (*endptr != '.') {
            LOG_ERR("Missing 1st '.' character in format string");
            return false;
        } else if (tmp < 0) {
            LOG_ERR("<x> values < 0 are not supported");
            return false;
        }
        target.t.xLev = tmp;

        // Find yLev component of <test>
        swork = swork.substr(swork.find_first_of('.') + 1, swork.length());
        if (swork.length() == 0) {
            LOG_ERR("Unrecognized format <grp>:<x>.<y>=%s", optarg);
            return false;
        }

        tmp = strtol(swork.substr(0, swork.size()).c_str(), &endptr, 10);
        if (*endptr != '.') {
            LOG_ERR("Missing 2nd '.' character in format string");
            return false;
        } else if (tmp < 0) {
            LOG_ERR("<y> values < 0 are not supported");
            return false;
        }
        target.t.yLev = tmp;

        // Find zLev component of <test>
        swork = swork.substr(swork.find_first_of('.') + 1, swork.length());
        if (swork.length() == 0) {
            LOG_ERR("Unrecognized format <grp>:<x>.<y>.<z>=%s", optarg);
            return false;
        }

        tmp = strtol(swork.substr(0, swork.size()).c_str(), &endptr, 10);
        if (*endptr != '\0') {
            LOG_ERR("Unrecognized format <grp>:<x>.<y>.<z>=%s", optarg);
            return false;
        } else if (tmp < 0) {
            LOG_ERR("<z> values < 0 are not supported");
            return false;
        }
        target.t.zLev = tmp;
    }
;
    if (target.t.group == UINT_MAX) {
        LOG_ERR("Unrecognized format <grp>=%s\n", optarg);
        return false;
    } else if (!(((target.t.xLev == UINT_MAX)  &&
                  (target.t.yLev == UINT_MAX)  &&
                  (target.t.zLev == UINT_MAX)) ||
                 ((target.t.xLev != UINT_MAX)  &&
                  (target.t.yLev != UINT_MAX)  &&
                  (target.t.zLev != UINT_MAX)))) {
        LOG_ERR("Unrecognized format <grp>:<x>.<y>.<z>=%s", optarg);
        LOG_ERR("Parsed and decoded: <%ld>:<%ld>.<%ld>.<%ld>",
            target.t.group, target.t.xLev, target.t.yLev, target.t.zLev);
        return false;
    }

    return true;
}


/**
 * A function to specifically handle parsing cmd lines of the form
 * "<space:offset:num:acc>".
 * @param rmmap Pass a structure to populate with parsing results
 * @param optarg Pass the 'optarg' argument from the getopt_long() API.
 * @return true upon successful parsing, otherwise false.
 */
bool
ParseRmmapCmdLine(RmmapIo &rmmap, const char *optarg)
{
    size_t ulwork;
    char *endptr;
    string swork;
    size_t tmp;
    string sacc;


    rmmap.req = true;
    rmmap.space = NVMEIO_FENCE;
    rmmap.offset = 0;
    rmmap.size = 0;
    rmmap.acc = ACC_FENCE;

    LOG_DBG("Option selected = %s", optarg);

    // Parsing <space:offset:size>
    swork = optarg;
    if ((ulwork = swork.find(":", 0)) == string::npos) {
        LOG_ERR("Unrecognized format <space:off:size:acc>=%s", optarg);
        return false;
    }
    if (strcmp("PCI", swork.substr(0, ulwork).c_str()) == 0) {
        rmmap.space = NVMEIO_PCI_HDR;
    } else if (strcmp("BAR01", swork.substr(0, ulwork).c_str()) == 0) {
        rmmap.space = NVMEIO_BAR01;
    } else {
        LOG_ERR("Unrecognized identifier <space>=%s", optarg);
        return false;
    }

    // Parsing <off:size:acc>
    swork = swork.substr(ulwork+1, swork.size());
    tmp = strtoul(swork.substr(0, swork.size()).c_str(), &endptr, 16);
    if (*endptr != ':') {
        LOG_ERR("Unrecognized format <off:size:acc>=%s", optarg);
        return false;
    }
    rmmap.offset = tmp;

    // Parsing <size:acc>
    swork = swork.substr(swork.find_first_of(':') + 1, swork.length());
    if (swork.length() == 0) {
        LOG_ERR("Missing <size> format string");
        return false;
    }
    tmp = strtoul(swork.substr(0, swork.size()).c_str(), &endptr, 16);
    if (*endptr != ':') {
        LOG_ERR("Unrecognized format <size:acc>=%s", optarg);
        return false;
    }
    rmmap.size = tmp;

    // Parsing <acc>
    swork = swork.substr(swork.find_first_of(':') + 1, swork.length());
    if (swork.length() == 0) {
        LOG_ERR("Missing <acc> format string");
        return false;
    }

    sacc = swork.substr(0, swork.size()).c_str();
    tmp = strtoul(swork.substr(1, swork.size()).c_str(), &endptr, 16);
    if (*endptr != '\0') {
    	LOG_ERR("Unrecognized format <acc>=%s", optarg);
        return false;
    }
    // Detect the access width passed.
    if (sacc.compare("b") == 0) {
    	rmmap.acc = BYTE_LEN;
    } else if (sacc.compare("w") == 0) {
    	rmmap.acc = WORD_LEN;
    } else if (sacc.compare("l") == 0) {
    	rmmap.acc = DWORD_LEN;
    } else if (sacc.compare("q") == 0) {
    	rmmap.acc = QUAD_LEN;
    } else {
    	LOG_ERR("Unrecognized access width to read.");
    	return false;
    }

    return true;
}


/**
 * A function to specifically handle parsing cmd lines of the form
 * "<space:offset:num:val:acc>".
 * @param wmmap Pass a structure to populate with parsing results
 * @param optarg Pass the 'optarg' argument from the getopt_long() API.
 * @return true upon successful parsing, otherwise false.
 */
bool
ParseWmmapCmdLine(WmmapIo &wmmap, const char *optarg)
{
    size_t ulwork;
    char *endptr;
    string swork;
    size_t tmp;
    uint64_t tmpVal;
    string sacc;

    wmmap.req = true;
    wmmap.space = NVMEIO_FENCE;
    wmmap.offset = 0;
    wmmap.size = 0;
    wmmap.value = 0;
    wmmap.acc = ACC_FENCE;

    // Parsing <space:off:size:val:acc>
    swork = optarg;
    if ((ulwork = swork.find(":", 0)) == string::npos) {
        LOG_ERR("Unrecognized format <space:off:siz:val:acc>=%s", optarg);
        return false;
    }
    if (strcmp("PCI", swork.substr(0, ulwork).c_str()) == 0) {
        wmmap.space = NVMEIO_PCI_HDR;
    } else if (strcmp("BAR01", swork.substr(0, ulwork).c_str()) == 0) {
        wmmap.space = NVMEIO_BAR01;
    } else {
        LOG_ERR("Unrecognized identifier <space>=%s", optarg);
        return false;
    }

    // Parsing <off:size:val:acc>
    swork = swork.substr(ulwork+1, swork.size());
    tmp = strtoul(swork.substr(0, swork.size()).c_str(), &endptr, 16);
    if (*endptr != ':') {
        LOG_ERR("Unrecognized format <off:siz:val:acc>=%s", optarg);
        return false;
    }
    wmmap.offset = tmp;

    // Parsing <size:val:acc>
    swork = swork.substr(swork.find_first_of(':') + 1, swork.length());
    tmp = strtoul(swork.substr(0, swork.size()).c_str(), &endptr, 16);
    if (*endptr != ':') {
        LOG_ERR("Unrecognized format <siz:val:acc>=%s", optarg);
        return false;
    } else if (tmp > MAX_SUPPORTED_REG_SIZE) {
        LOG_ERR("<size> > allowed value of max of %d bytes",
            MAX_SUPPORTED_REG_SIZE);
        return false;
    }
    wmmap.size = tmp;

    // Parsing <val:acc>
    swork = swork.substr(swork.find_first_of(':') + 1, swork.length());
    if (swork.length() == 0) {
        LOG_ERR("Missing <val> format string");
        return false;
    }
    tmpVal = strtoull(swork.substr(0, swork.size()).c_str(), &endptr, 16);
    if (*endptr != ':') {
        LOG_ERR("Unrecognized format <val:acc>=%s", optarg);
        return false;
    }
    wmmap.value = tmpVal;

    // Parsing <acc>
    swork = swork.substr(swork.find_first_of(':') + 1, swork.length());
    if (swork.length() == 0) {
        LOG_ERR("Missing <acc> format string");
        return false;
    }

    sacc = swork.substr(0, swork.size()).c_str();
    tmp = strtoul(swork.substr(1, swork.size()).c_str(), &endptr, 16);
    if (*endptr != '\0') {
        LOG_ERR("Unrecognized format <acc>=%s", optarg);
        return false;
    }

    if (sacc.compare("b") == 0) {
    	wmmap.acc = BYTE_LEN;
    } else if (sacc.compare("w") == 0) {
    	wmmap.acc = WORD_LEN;
    } else if (sacc.compare("l") == 0) {
    	wmmap.acc = DWORD_LEN;
    } else if (sacc.compare("q") == 0) {
    	wmmap.acc = QUAD_LEN;
    } else {
    	LOG_ERR("Unrecognized access width for writing.");
    	return false;
    }

    return true;
}


/**
 * A function to specifically handle parsing cmd lines of the form
 * "<STS:PXDS:AERUCES:CSTS>".
 * @param errRegs Pass a structure to populate with parsing results
 * @param optarg Pass the 'optarg' argument from the getopt_long() API.
 * @return true upon successful parsing, otherwise false.
 */
bool
ParseErrorCmdLine(ErrorRegs &errRegs, const char *optarg)
{
    char *endptr;
    string swork;
    size_t tmp;
    string sacc;

    errRegs.sts = 0;
    errRegs.pxds = 0;
    errRegs.aeruces = 0;
    errRegs.csts = 0;

    // Parsing <STS:PXDS:AERUCES:CSTS>
    swork = optarg;
    tmp = strtoul(swork.substr(0, swork.size()).c_str(), &endptr, 16);
    if (*endptr != ':') {
        LOG_ERR("Unrecognized format <STS:PXDS:AERUCES:CSTS>=%s", optarg);
        return false;
    } else if (tmp > ((uint16_t)(-1))) {
        LOG_ERR("<STS> > allowed max value of 0x%04X", ((uint16_t)(-1)));
        return false;
    }
    errRegs.sts = (uint16_t)tmp;

    // Parsing <PXDS:AERUCES:CSTS>
    swork = swork.substr(swork.find_first_of(':') + 1, swork.length());
    tmp = strtoul(swork.substr(0, swork.size()).c_str(), &endptr, 16);
    if (*endptr != ':') {
        LOG_ERR("Unrecognized format <PXDS:AERUCES:CSTS>=%s", optarg);
        return false;
    } else if (tmp > ((uint16_t)(-1))) {
        LOG_ERR("<PXDS> > allowed max value of 0x%04X", ((uint16_t)(-1)));
        return false;
    }
    errRegs.pxds = (uint16_t)tmp;

    // Parsing <AERUCES:CSTS>
    swork = swork.substr(swork.find_first_of(':') + 1, swork.length());
    tmp = strtoul(swork.substr(0, swork.size()).c_str(), &endptr, 16);
    if (*endptr != ':') {
        LOG_ERR("Unrecognized format <AERUCES:CSTS>=%s", optarg);
        return false;
    } else if (tmp > ((uint16_t)(-1))) {
        LOG_ERR("<AERUCES> > allowed max value of 0x%04X", ((uint16_t)(-1)));
        return false;
    }
    errRegs.aeruces = (uint16_t)tmp;

    // Parsing <CSTS>
    swork = swork.substr(swork.find_first_of(':') + 1, swork.length());
    tmp = strtoul(swork.substr(0, swork.size()).c_str(), &endptr, 16);
    if (*endptr != '\0') {
        LOG_ERR("Unrecognized format <CSTS>=%s", optarg);
        return false;
    } else if (tmp > ((uint16_t)(-1))) {
        LOG_ERR("<CSTS> > allowed max value of 0x%08X", ((uint32_t)(-1)));
        return false;
    }
    errRegs.csts = (uint16_t)tmp;

    return true;
}


/**
 * A function to specifically handle parsing cmd lines of the form
 * "<ncqr:nsqr>".
 * @param numQueues Pass a structure to populate with parsing results
 * @param optarg Pass the 'optarg' argument from the getopt_long() API.
 * @return true upon successful parsing, otherwise false.
 */
bool
ParseQueuesCmdLine(NumQueues &numQueues, const char *optarg)
{
    char *endptr;
    string swork;
    size_t tmp;
    string sacc;

    numQueues.req = true;
    numQueues.ncqr = 0;
    numQueues.nsqr = 0;

    // Parsing <ncqr:nsqr>
    swork = optarg;
    tmp = strtoul(swork.substr(0, swork.size()).c_str(), &endptr, 16);
    if (*endptr != ':') {
        LOG_ERR("Unrecognized format <ncqr:nsqr>=%s", optarg);
        return false;
    } else if (tmp > ((uint16_t)(-1))) {
        LOG_ERR("<ncqr> > allowed max value of 0x%04X", ((uint16_t)(-1)));
        return false;
    }
    numQueues.ncqr = (uint16_t)tmp;

    // Parsing <nsqr>
    swork = swork.substr(swork.find_first_of(':') + 1, swork.length());
    if (swork.length() == 0) {
        LOG_ERR("Missing <nsqr> format string");
        return false;
    }
    tmp = strtoul(swork.substr(0, swork.size()).c_str(), &endptr, 16);
    if (*endptr != '\0') {
        LOG_ERR("Unrecognized format <nsqr>=%s", optarg);
        return false;
    } else if (tmp > ((uint16_t)(-1))) {
        LOG_ERR("<nsqr> > allowed max value of 0x%04X", ((uint16_t)(-1)));
        return false;
    }
    numQueues.nsqr = (uint16_t)tmp;

    return true;
}
