/*
 * Copyright (C) 2004-2014 ZNC, see the NOTICE file for details.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <znc/Client.h>
#include <znc/Modules.h>
#include <znc/IRCNetwork.h>

#include <boost/regex.hpp>

using std::vector;

/*

Tested:
WeeChat 0.3.8 (Dec 17 2012)                                          - Yes, tested
Purple IRC                                                           - No, tested using finch (not Pidgin)
HexChat 2.10.0 [x64] / Windows 7 SP1 [3.69GHz]                       - Sometimes, requires a script
irssi v0.8.15                                                        - No, tested
Android IRC v2.1.15                                                  - Yes, tested on Kindle Fire
Yaaic - Yet another Android IRC client - http://www.yaaic.org        - Yes, tested with port on BB10 OS (no version info?)
Communi 2.2.0                                                        - No, tested with TinCanIRC on BB10 OS

Informed:
IRCCloud irccloud.com                                                - Yes, from wiki
xchat 2.8.6 Ubuntu                                                   - Sometimes, requires a script, from wiki
Textual IRC Client: www.textualapp.com — v5.0.0                      - Yes, from wiki
mIRC v7.29 Khaled Mardam-Bey                                         - Sometimes, requires a script, from wiki
Using libcommuni 3.0.1 - http://communi.github.com                   - Yes, as informed by the author
Colloquy 1.5.2 (5977) - iPhone OS 7.1.2 (ARM) - http://colloquy.mobi - Yes, from wiki

No information:
Android IRC Client (3.3.4 - Build 54a08b6 (dirty)-)                  - Not tested
AndChat 1.4.3.2 http://www.andchat.net                               - Not tested
*/

class CPrivmsgAuto : public CModule {
	void StatusCommand(const CString& sLine) {
		if (m_sNoPrefixClients.find(m_pClient) != m_sNoPrefixClients.end()) {
			PutModule("This client receives non-prefixed privmsg");
		} else {
			PutModule("This client receives prefixed privmsg");
		}
	}

	void ToggleCommand(const CString& sLine) {
		if (m_sNoPrefixClients.find(m_pClient) != m_sNoPrefixClients.end()) {
			m_sNoPrefixClients.erase(m_sNoPrefixClients.find(m_pClient));
			PutModule("This client will receive prefixed privmsg");
		} else {
			m_sNoPrefixClients.insert(m_pClient);
			PutModule("This client will receive non-prefixed privmsg");
		}
	}

public:
	MODCONSTRUCTOR(CPrivmsgAuto) {
		// Replace this with a command system to let people set their own, ship with known defaults?
		m_vCapableVersions.push_back(boost::regex("Android IRC v")); // Version?
		m_vCapableVersions.push_back(boost::regex("WeeChat")); // Version?
		m_vCapableVersions.push_back(boost::regex("Yaaic")); // Version?
		m_vCapableVersions.push_back(boost::regex("IRCCloud")); // Version?
		m_vCapableVersions.push_back(boost::regex("Colloquy")); // Version?
		m_vCapableVersions.push_back(boost::regex("Using libcommuni 3\\.(?:[1-9]+|0\\.[1-9]+)"));
		m_vCapableVersions.push_back(boost::regex("Textual .*(?:2\\.[1-9]{1}[0-9]*|[3-9]{1}[0-9]*)\\."));

		// The following require scripts ---
		m_vCapableVersions.push_back(boost::regex("HexChat 2\\.(?:9\\.[6-9]{1}|1(?:[0-9])+)"));
		m_vCapableVersions.push_back(boost::regex("xchat 2\\.(?:8\\.[6-9]{1}|1(?:[0-9])+)"));
		m_vCapableVersions.push_back(boost::regex("mIRC v(?:7\\.(?:(?:2[7-9])|(?:[3-9]|[0-9]{3,}))|(?:[8-9]|[0-9]{2,}))"));

		// Commands
		AddHelpCommand();
		AddCommand("Status", static_cast<CModCommand::ModCmdFunc>(&CPrivmsgAuto::StatusCommand));
		AddCommand("Toggle", static_cast<CModCommand::ModCmdFunc>(&CPrivmsgAuto::ToggleCommand));
	}

	// When loaded, send a ctcp version to all connected clients.
	virtual bool OnLoad(const CString& sArgsi, CString& sMessage) {
		if (m_pNetwork) {
			vector<CClient*> vClients = m_pNetwork->GetClients();
			for (std::vector<CClient*>::size_type i = 0; i != vClients.size(); i++) {
				if (vClients[i]->GetNetwork()) {
					if (m_pNetwork->IsIRCConnected ()) {
						vClients[i]->PutClient(":*privmsg_auto!znc@znc.in PRIVMSG " + m_pNetwork->GetIRCNick().GetNick() + " :\x01VERSION\x01");
					} else {
						vClients[i]->PutClient(":*privmsg_auto!znc@znc.in PRIVMSG " + m_pNetwork->GetNick() + " :\x01VERSION\x01");
					}
				}
			}
		}
		return true;
	}

	virtual void OnClientLogin() {
		m_pClient->PutClient(":*privmsg_auto!znc@znc.in PRIVMSG " + m_pNetwork->GetIRCNick().GetNick() + " :\001" + "VERSION" + "\001");
	}

	virtual void onClientDisconnect() {
		if (m_sNoPrefixClients.find(m_pClient) != m_sNoPrefixClients.end()) {
			m_sNoPrefixClients.erase(m_sNoPrefixClients.find(m_pClient));
		}
	}

	virtual void OnModNotice(const CString& sLine) {
		if (sLine.StartsWith("\001VERSION")) {
			CString line = CString(sLine);
			line.LeftChomp(9);
			line.RightChomp(1);
			bool found = false;

			for(std::vector<boost::regex>::size_type i = 0; i != m_vCapableVersions.size(); i++) {
				if (boost::regex_search(line, m_vCapableVersions[i])) {
					found = true;
					break;
				}
			}
//			PutModule(line);
			if (found) {
				m_sNoPrefixClients.insert(m_pClient);
//				PutModule("This client supports not-prefixed privmsg!");
//			} else {
//				PutModule("This client does not support non-prefixed privmsg");
			}
		}
	}

	virtual EModRet OnUserMsg(CString& sTarget, CString& sMessage) {
		if (m_pNetwork && m_pNetwork->GetIRCSock() && !m_pNetwork->IsChan(sTarget)) {
			vector<CClient*> vClients = m_pNetwork->GetClients();
			for (std::vector<CClient*>::size_type i = 0; i != vClients.size(); i++) {
				if (vClients[i] != m_pClient) {
					if (m_sNoPrefixClients.find(vClients[i]) != m_sNoPrefixClients.end()) {
						m_pNetwork->PutUser(":" + m_pNetwork->GetIRCNick().GetNickMask() + " PRIVMSG " + sTarget + " :" + sMessage, vClients[i], NULL);
					} else {
						m_pNetwork->PutUser(":" + sTarget + "!prefix@privmsg.znc.in PRIVMSG " + m_pNetwork->GetIRCNick().GetNick() + " :-> " + sMessage, vClients[i], NULL);
					}
				}
			}
		}

		return CONTINUE;
	}

	virtual EModRet OnUserAction(CString& sTarget, CString& sMessage) {
		if (m_pNetwork && m_pNetwork->GetIRCSock() && !m_pNetwork->IsChan(sTarget)) {
			vector<CClient*> vClients = m_pNetwork->GetClients();
			for (std::vector<CClient*>::size_type i = 0; i != vClients.size(); i++) {
				if (vClients[i] != m_pClient) {
					if (m_sNoPrefixClients.find(vClients[i]) != m_sNoPrefixClients.end()) {
						m_pNetwork->PutUser(":" + m_pNetwork->GetIRCNick().GetNickMask() + " PRIVMSG " + sTarget + " :\x01" + "ACTION " + sMessage + "\x01", vClients[i], NULL);
					} else {
						m_pNetwork->PutUser(":" + sTarget + "!prefix@privmsg.znc.in PRIVMSG " + m_pNetwork->GetIRCNick().GetNick() + " :\x01" + "ACTION -> " + sMessage + "\x01", vClients[i], NULL);
					}
				}
			}
		}

		return CONTINUE;
	}
private:
	std::set<CClient*> m_sNoPrefixClients;
	vector<boost::regex> m_vCapableVersions;
};

template<> void TModInfo<CPrivmsgAuto>(CModInfo& Info) {
	Info.SetWikiPage("privmsg_auto");
	Info.SetHasArgs(false);
}

NETWORKMODULEDEFS(CPrivmsgAuto, "Send outgoing PRIVMSGs and CTCP ACTIONs to other clients using ugly prefixes")
