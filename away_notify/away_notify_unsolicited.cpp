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

const char* m_sCapability = "away-notify";

class CAwayNotify : public CModule {
public:
	MODCONSTRUCTOR(CAwayNotify) {}

	virtual ~CAwayNotify() {}

	// Advertise that we can handle away-notify to the server
	virtual bool OnServerCapAvailable(const CString& sCap) {
		return sCap.Equals(m_sCapability);
	}

	// Later with cap-notify this may be relevant to us, but for now we can't do anything with it.
	virtual void OnServerCapResult(const CString& sCap, bool bSuccess) {}

	// Do not offer the cap, we will send an unsolicited LS later to enable it if applicable.
	virtual void OnClientCapLs(CClient* pClient, SCString &ssCaps) {}

	// Since we later check on every incoming away-notify message if the clients connected will get it, we don't care about this.
	virtual void OnClientCapRequest(CClient* pClient, const CString& sCap, bool bState) {}

	// We sell away-notify and away-notify accessories
	// More specifically, after our unsolicited LS we need znc to accept we can use it.
	virtual bool IsClientCapSupported(CClient* pClient, const CString& sCap, bool bState) {
		return sCap.Equals(m_sCapability);
	}

	// If the module was just loaded, check each network for away-notify and if a client doesn't have that's connected to it, offer it to the client.
	virtual bool OnLoad(const CString& sArgsi, CString& sMessage) {
		// TODO: There are at present too many hoops to jump through to do this.  So I don't.
		return true;
	}

	// If we have clients already connected, the server has away-notify, and they do not have away-notify, offer it to them.
	virtual void OnIRCConnected() {
		// TODO: Do this
		if(m_pNetwork && m_pNetwork->GetIRCSock() && m_pNetwork->GetIRCSock()->IsCapAccepted(m_sCapability) && !m_pNetwork->GetClients().empty()) {
			for(CClient* client : m_pNetwork->GetClients()) {
				if(!client->IsCapEnabled(m_sCapability)) {
					client->PutClient(CString(":irc.znc.in CAP unknown-nick LS :") + m_sCapability);
				}
			}
		}
	}

	// Once a client logs in, we now know their network.  If it has away-notify, offer it to the client.
	virtual void OnClientLogin() {
		if(m_pNetwork && m_pNetwork->GetIRCSock() && m_pNetwork->GetIRCSock()->IsCapAccepted(m_sCapability) && m_pClient) {
			//CString sCapOffer = CString(" ").Join(m_pClient->m_ssAcceptedCaps.begin(), m_pClient->m_ssAcceptedCaps.end());
			//sCapOffer += " " + m_sCapability;
			m_pClient->PutClient(CString(":irc.znc.in CAP unknown-nick LS :") + m_sCapability);
		}
	}

	virtual EModRet OnRaw(CString& sLine) {
		CString cmd = sLine.Token(1, false);
		if (cmd.Equals("AWAY")) {
			if (m_pNetwork && m_pNetwork->GetIRCSock() && m_pNetwork->GetIRCSock()->IsCapAccepted(m_sCapability)) {
				vector<CClient*> vClients = m_pNetwork->GetClients();
				for (std::vector<CClient*>::size_type i = 0; i != vClients.size(); i++) {
					if (vClients[i] != m_pClient && vClients[i]->IsCapEnabled(m_sCapability)) {
						m_pNetwork->PutUser(sLine, vClients[i], NULL);
					}
				}
			}
			return HALTCORE;
		}
		return CONTINUE;
	}
};

template<> void TModInfo<CAwayNotify>(CModInfo& Info) {
	Info.SetWikiPage("away_notify");
	Info.SetHasArgs(false);
}

GLOBALMODULEDEFS(CAwayNotify, "Provides IRCv3.1 away-notify, using unsolicited CAP LS")
