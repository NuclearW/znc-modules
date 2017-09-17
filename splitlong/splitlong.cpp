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
/*
From irssi's splitlong-safe.pl

:mynick!user@host PRIVMSG nick :message
510 = 512 - length("\r\n") # 512 is mandated by rfc2812
497 = 510 - length(":" . "!" . " PRIVMSG " . " :");
422 = 497 - length("@" . $ownusername . $ownhostname) # maximum 64 for hostname, 10 for username
leaving just the nick length and the target nick/channel's length
$maxlength = 422 - length( $real_server->{nick} . $real_target );

9 = length( "\01ACTION " . $msg . "\01" ) - length( $msg );
$maxlength -= 9 if $action eq 'me'
*/

#include <znc/Nick.h>
#include <znc/Client.h>
#include <znc/IRCNetwork.h>
#include <znc/ZNCString.h>

using std::vector;

class CSplitlong : public CModule {
public:
	MODCONSTRUCTOR(CSplitlong) {}

	virtual EModRet OnUserMsg(CString& sTarget, CString& sMessage) {
		return processMessage(sTarget, sMessage, false);
	}

	virtual EModRet OnUserAction(CString& sTarget, CString& sMessage) {
		return processMessage(sTarget, sMessage, true);
	}

private:
	EModRet processMessage(const CString& sTarget, const CString& sMessage, bool action) {
		if (m_pNetwork) {
			const CNick& nNick = m_pNetwork->GetIRCNick();
			CString sNickMask = nNick.GetNickMask();
			size_t iMaxlength = 497;
			iMaxlength -= sTarget.length();
			iMaxlength -= sNickMask.length();
			if (action) {
				iMaxlength -= 9;
			}
			if (!sNickMask.WildCmp("*!*@*")) {
				// We don't have the full mask, so we have to assume the worst
				iMaxlength -= 65;
			}

			CString sLoopMessage = CString(sMessage);
			if (sLoopMessage.length() > iMaxlength) {
				do {
					CString sTmpMessage = sLoopMessage.substr(0, iMaxlength);
					sLoopMessage = sLoopMessage.substr(iMaxlength);
					if (action) {
						PutIRC("PRIVMSG " + sTarget + " :\x01" + "ACTION " + sTmpMessage + "\x01");
					} else {
						PutIRC("PRIVMSG " + sTarget + " :" + sTmpMessage);
					}
				} while (sLoopMessage.length() > iMaxlength);
				if (action) {
					PutIRC("PRIVMSG " + sTarget + " :\x01" + "ACTION " + sLoopMessage + "\x01");
				} else {
					PutIRC("PRIVMSG " + sTarget + " :" + sLoopMessage);
				}

				// We must relay to other clients since we HALTCORE
				if (m_pNetwork->IsChan(sTarget)) {
					const vector<CClient*>& vClients = m_pClient->GetClients();
					for (unsigned int a = 0; a < vClients.size(); a++) {
						CClient* pClient = vClients[a];
						if (pClient != m_pClient) {
							if (action) {
								pClient->PutClient(":" + sNickMask + " PRIVMSG " + sTarget + " :\x01" + "ACTION " + sMessage + "\x01");
							} else {
								pClient->PutClient(":" + sNickMask + " PRIVMSG " + sTarget + " :" + sMessage);
							}
						}
					}
				}

				return HALTCORE;
			}
		}
		return CONTINUE;
	}
};

template<> void TModInfo<CSplitlong>(CModInfo& Info) {
	Info.SetWikiPage("splitlong");
}

NETWORKMODULEDEFS(CSplitlong, "Break up messages too long to fit the 512 maximum that dumb clients send.")
