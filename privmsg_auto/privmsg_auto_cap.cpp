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
#include <znc/IRCSock.h>

using std::vector;

const char* m_sCapability = "self-message";

class CPrivmsgAutoCap : public CModule {
public:
	MODCONSTRUCTOR(CPrivmsgAutoCap) {}

	virtual ~CPrivmsgAutoCap() {}

	// Offer the capability
	virtual void OnClientCapLs(CClient* pClient, SCString &ssCaps) {
		ssCaps.insert(m_sCapability);
	}

	// Since we later check on messages as they go through, we need not do anything with this
	virtual void OnclientCapRequest(CClient* pClient, const CString& sCap, bool bState) {}

	// We sell self-message and self-message accessories
	virtual bool IsClientCapSupported(CClient* pClient, const CString& sCap, bool bState) {
		return sCap.Equals(m_sCapability);
	}

	virtual EModRet OnUserMsg(CString& sTarget, CString& sMessage) {
		if (m_pNetwork && m_pNetwork->GetIRCSock() && !m_pNetwork->IsChan(sTarget)) {
			vector<CClient*> vClients = m_pNetwork->GetClients();
			for (std::vector<CClient*>::size_type i = 0; i != vClients.size(); i++) {
				if (vClients[i] != m_pClient) {
					if (vClients[i]->IsCapEnabled(m_sCapability)) {
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
					if (vClients[i]->IsCapEnabled(m_sCapability)) {
						m_pNetwork->PutUser(":" + m_pNetwork->GetIRCNick().GetNickMask() + " PRIVMSG " + sTarget + " :\x01" + "ACTION " + sMessage + "\x01", vClients[i], NULL);
					} else {
						m_pNetwork->PutUser(":" + sTarget + "!prefix@privmsg.znc.in PRIVMSG " + m_pNetwork->GetIRCNick().GetNick() + " :\x01" + "ACTION -> " + sMessage + "\x01", vClients[i], NULL);
					}
				}
			}
		}

		return CONTINUE;
	}

};

template<> void TModInfo<CPrivmsgAutoCap>(CModInfo& Info) {
	Info.SetWikiPage("privmsg_auto_cap");
	Info.SetHasArgs(false);
}

GLOBALMODULEDEFS(CPrivmsgAutoCap, "Automatically switches between privmsg and privmsg_prefix behavior based on the self-message CAP")