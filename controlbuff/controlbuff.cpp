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

#include <znc/IRCNetwork.h>
#include <znc/ZNCString.h>
#include <znc/Query.h>
#include <znc/Utils.h>

using std::vector;

// TODO: Persist?

class CControlBuff : public CModule {
	void ListCommand(const CString& sLine) {
		if(m_pNetwork) {
			vector<CQuery*> queries = m_pNetwork->GetQueries();
			if(queries.empty()) {
				PutModule("There are no queries to list.");
			} else {
				CTable table;
				table.AddColumn("Name");
				table.AddColumn("Size");
				table.AddColumn("Max Size");
				table.AddColumn("Pinned");

				for(CQuery* query : queries) {
					table.AddRow();
					table.SetCell("Name", query->GetName());
					table.SetCell("Size", CString(query->GetBuffer().Size()));
					table.SetCell("Max Size", CString(query->GetBufferCount()));
					table.SetCell("Pinned", IsPinned(query->GetName()) ? "*" : " ");
				}

				PutModule(table);
			}
		} else {
			PutModule("Error listing buffers.");
		}
	}

	void PinCommand(const CString& sLine) {
		if(m_pNetwork) {
			VCString queries;
			sLine.Split(" ", queries, false, "", "", false, true);
			if(!queries.empty()) queries.erase(queries.begin());
			for(CString query : queries) {
				if(m_pNetwork->FindQuery(query)) {
					if(IsPinned(query)) {
						PutModule(query + " is already pinned.");
					} else {
						Pin(query);
						PutModule("Pinned " + query);
					}
				} else {
					PutModule("Could not find buffer: " + query);
				}
			}
		} else {
			PutModule("Error pinning buffers.");
		}
		// TODO: Prevent it from being removed automatically?
	}

	void UnpinCommand(const CString& sLine) {
		if(m_pNetwork) {
			VCString queries;
			sLine.Split(" ", queries, false, "", "", false, true);
			if(!queries.empty()) queries.erase(queries.begin());
			for(CString query : queries) {
				if(m_pNetwork->FindQuery(query)) {
					if(IsPinned(query)) {
						Unpin(query);
						PutModule("Unpinned " + query);
					} else {
						PutModule(query + " is not pinned.");
					}
				} else {
					PutModule("Could not find buffer: " + query);
				}
			}
		} else {
			PutModule("Error unpinning buffers.");
		}
	}

	void CleanCommand(const CString& sLine) {
		if(m_pNetwork) {
			vector<CQuery*> queries = m_pNetwork->GetQueries();
			if(queries.empty()) {
				PutModule("There are no queries to clean.");
			} else {
				for(CQuery* query : queries) {
					if(!IsPinned(query->GetName())) {
						m_pNetwork->DelQuery(query->GetName());
					}
				}

				PutModule("Cleaned buffers.");
			}
		} else {
			PutModule("Error cleaning buffers.");
		}
	}

public:
	MODCONSTRUCTOR(CControlBuff) {		
		// Commands
		AddHelpCommand();
		AddCommand("List", static_cast<CModCommand::ModCmdFunc>(&CControlBuff::ListCommand), "", "List all query buffers.");
		AddCommand("Pin", static_cast<CModCommand::ModCmdFunc>(&CControlBuff::PinCommand), "<buffer> (<buffer> ...)", "Pin a query buffer.");
		AddCommand("Unpin", static_cast<CModCommand::ModCmdFunc>(&CControlBuff::UnpinCommand), "<buffer> (<buffer> ...)", "Unin a query buffer.");
		AddCommand("Clean", static_cast<CModCommand::ModCmdFunc>(&CControlBuff::CleanCommand), "", "Remove all unpinned query buffers.");
	}

private:
	bool IsPinned(const CString& sQuery) {
		return m_sPinned.find(sQuery.AsLower()) != m_sPinned.end();
	}

	void Pin(const CString& sQuery) {
		m_sPinned.insert(sQuery.AsLower());
	}

	void Unpin(const CString& sQuery) {
		if(IsPinned(sQuery)) m_sPinned.erase(sQuery.AsLower());
	}

	std::set<CString> m_sPinned;
};

template<> void TModInfo<CControlBuff>(CModInfo& Info) {
	Info.SetWikiPage("controlbuff");
}

NETWORKMODULEDEFS(CControlBuff, "Additional control of query buffers.")
