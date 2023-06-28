#include <windows.h>
#include <commctrl.h>
#include <iostream>
#include <filesystem>
#include <string>
#include <vector>
#include <utility>
#include <cmath>
#include <algorithm>
using namespace std;
namespace fs = filesystem;
const string desktopPath = "C:/Users/user/Desktop/";

#define ystart 2.0
#define yinter 88.0
#define xstart 36.0
#define xinter 105.0
#define xoffset 2.0

HWND hd;
HANDLE he;
DWORD Pi;

// Program-specific function to duplicate a file (don't use this exact code)
void duplicateFile(string sfn, string fn){
	fs::copy(sfn, desktopPath+fn);
}

vector<vector<string>> names = {
	// {"rd1", "nd1", "bd1", "qd1", "kd1", "bd2", "nd2", "rd2"},
	{"rd1", "", "", "", "kd1", "", "", "rd2"},
	{"pd1", "pd2", "pd3", "pd4", "pd5", "pd6", "pd7", "pd8"},
	{"",    "",    "",    "",    "",    "",     "",   ""   },
	{"",    "",    "",    "",    "",    "",     "",   ""   },
	{"",    "",    "",    "",    "",    "",     "",   ""   },
	{"",    "",    "",    "",    "",    "",     "",   ""   },
	{"pl1", "pl2", "pl3", "pl4", "pl5", "pl6", "pl7", "pl8"},
	{"rl1", "nl1", "bl1", "ql1", "kl1", "bl2", "nl2", "rl2"}
};

vector<int> rookIndex = {};

vector<pair<int,int>> pos = {};
vector<string> name = {};
vector<bool> valid = {}, pjc = {}, moved = {};
bool whiteTurn = true, stopped = false;

int ncount, pcount;
pair<int, int> nextPos = {0,0}, queens = {3,3};

void capture(int index){
	cout << name[index] << " has been CAPTURED!" << "\n";
	valid[index] = false;
	pos[index] = {8, 8};
	ListView_SetItemPosition(hd, pcount+index, xstart+xinter*(12-nextPos.first), ystart+yinter*nextPos.second);
	nextPos.second++;
	if (nextPos.second >= 8){
		nextPos.second = 0;
		nextPos.first++;
	}
	if (name[index][0]=='k'){
		stopped = true;
	}
}

// n1: rook, bishop...
// n2: (l)ight, (d)ark...
bool checkMove(char n1, char n2, pair<int,int> p1, pair<int,int> p2, int selfIndex) {
	auto ipos = find(pos.begin(), pos.end(), p2);
	int index = ipos - pos.begin(), enpassantCapture = -1;
	bool found = ipos != pos.end(), setPjc = false;
	
	// Check 0: If it's the piece's turn
	if (whiteTurn != (n2=='l')){
		return false;
	}
	
	// Check 1: If the piece has moved at all
	if (p1.first == p2.first && p1.second == p2.second){
		return false;
	}

	// Check 2: If the piece is out of the map
	if (p1.first < 0 || p1.first > 7 || p1.second < 0 || p1.second > 7 || p2.first < 0 || p2.first > 7 || p2.second < 0 || p2.second > 7){
		return false;
	}

	// Check 3: If the piece is moving where it is supposed to make and if there are pieces obstructing the way
	switch (n1){
		case 'b':
			if (p1.first-p2.first == p1.second-p2.second) {
				pair<int,int> s = {min(p1.first, p2.first), min(p1.second, p2.second)};
				for (int i = 1; i < abs(p1.first-p2.first); i++){
					if(find(pos.begin(), pos.end(), pair<int, int>{s.first+i, s.second+i}) != pos.end()) {
						return false;
					}
				}
			} else if (p1.first-p2.first == -p1.second+p2.second) {
				pair<int,int> s = {min(p1.first, p2.first), max(p1.second, p2.second)};
				for (int i = 1; i < abs(p1.first-p2.first); i++){
					if(find(pos.begin(), pos.end(), pair<int, int>{s.first+i, s.second-i}) != pos.end()) {
						return false;
					}
				}
			} else {
				return false;
			}
			break;
		
		case 'k':
			if (p1.second == p2.second && !moved[selfIndex]){
				if (p2.first-p1.first == -2 && find(pos.begin(), pos.end(), pair<int, int>{p1.first-1, p1.second}) == pos.end() && find(pos.begin(), pos.end(), pair<int, int>{p1.first-2, p1.second}) == pos.end() && find(pos.begin(), pos.end(), pair<int, int>{p1.first-3, p1.second}) == pos.end() && !moved[rookIndex[n2=='l'? 2 : 0]]){
					pos[rookIndex[n2=='l'? 2 : 0]].first = p2.first+1;
					break;
				} else if (p2.first-p1.first == 2 && find(pos.begin(), pos.end(), pair<int, int>{p1.first+1, p1.second}) == pos.end() && find(pos.begin(), pos.end(), pair<int, int>{p1.first+2, p1.second}) == pos.end() && !moved[rookIndex[n2=='l'? 3 : 1]]) {
					pos[rookIndex[n2=='l'? 3 : 1]].first = p2.first-1;
					break;
				}
			}

			if (!(abs(p1.first-p2.first)<=1 && abs(p1.second-p2.second)<=1)){
				return false;
			}
			break;
		
		case 'n':
			if (!((abs(p1.first-p2.first)==2 && abs(p1.second-p2.second)==1) || (abs(p1.first-p2.first)==1 && abs(p1.second-p2.second)==2))){
				return false;
			}
			break;
		
		case 'p':
		{
			bool psuccess = false;

			if (n2 == 'l'){
				if (found){
					if (
						// Side differs
						p2.second-p1.second==-1 && 
						// Side differs
						name[index][1] == 'd' && 
						(p2.first-p1.first==1 || p2.first-p1.first==-1)
					){
						// Capture
						psuccess = true;
					}
				} else if (p1.first==p2.first) {
					// Side differs
					if (p1.second == 6){
						// Side differs
						if (p2.second-p1.second==-2){
							psuccess = true;
						}

						// Side differs
						if (p2.second-p1.second==-1){
							psuccess = true;
							setPjc = true;
						}
					} else {
						// Side differs
						if (p2.second-p1.second==-1){
							psuccess = true;
						}
					}
				}


				// Side differs
				auto ipos2 = find(pos.begin(), pos.end(), pair<int,int>{p2.first, p2.second+1});
				int index2 = ipos2 - pos.begin();
				bool found2 = ipos2 != pos.end();

				if (found2){
					if (
						// Side differs
						p2.second-p1.second==-1 && 
						// Side differs
						name[index2][1] == 'd' && 
						pjc[index2] &&
						(p2.first-p1.first==1 || p2.first-p1.first==-1)
					){
						enpassantCapture = index2;
						psuccess = true;
					}
				}
			} else {
				if (found){
					if (
						// Side differs
						p2.second-p1.second==1 && 
						// Side differs
						name[index][1] == 'l' && 
						(p2.first-p1.first==1 || p2.first-p1.first==-1)
					){
						// Capture
						psuccess = true;
					}
				} else if (p1.first==p2.first) {
					// Side differs
					if (p1.second == 1){
						// Side differs
						if (p2.second-p1.second==2){
							psuccess = true;
						}

						// Side differs
						if (p2.second-p1.second==1){
							psuccess = true;
							setPjc = true;
						}
					} else {
						// Side differs
						if (p2.second-p1.second==1){
							psuccess = true;
						}
					}
				}

				// Side differs
				auto ipos2 = find(pos.begin(), pos.end(), pair<int,int>{p2.first, p2.second-1});
				int index2 = ipos2 - pos.begin();
				bool found2 = ipos2 != pos.end();

				if (found2){
					if (
						// Side differs
						p2.second-p1.second==1 && 
						// Side differs
						name[index2][1] == 'l' && 
						pjc[index2] &&
						(p2.first-p1.first==1 || p2.first-p1.first==-1)
					){
						enpassantCapture = index2;
						psuccess = true;
					}
				}
			} 

			if (!psuccess){
				return false;
			}

			// Promotion:
			break;
		}

		case 'q':
			if (p1.first-p2.first == p1.second-p2.second) {
				pair<int,int> s = {min(p1.first, p2.first), min(p1.second, p2.second)};
				for (int i = 1; i < abs(p1.first-p2.first); i++){
					if(find(pos.begin(), pos.end(), pair<int, int>{s.first+i, s.second+i}) != pos.end()) {
						return false;
					}
				}
			} else if (p1.first-p2.first == -p1.second+p2.second) {
				pair<int,int> s = {min(p1.first, p2.first), max(p1.second, p2.second)};
				for (int i = 1; i < abs(p1.first-p2.first); i++){
					if(find(pos.begin(), pos.end(), pair<int, int>{s.first+i, s.second-i}) != pos.end()) {
						return false;
					}
				}
			} else if (p1.first==p2.first){
				for (int i = min(p1.second,p2.second)+1; i < max(p1.second,p2.second); i++){
					if(find(pos.begin(), pos.end(), pair<int, int>{p1.first, i}) != pos.end()) {
						return false;
					}
				}
			} else if (p1.second==p2.second){
				for (int i = min(p1.first,p2.first)+1; i < max(p1.first,p2.first); i++){
					if(find(pos.begin(), pos.end(), pair<int, int>{i, p1.second}) != pos.end()) {
						return false;
					}
				}
			} else {
				return false;
			}
			break;

		case 'r':
			if (p1.first==p2.first){
				for (int i = min(p1.second,p2.second)+1; i < max(p1.second,p2.second); i++){
					if (find(pos.begin(), pos.end(), pair<int, int>{p1.first, i}) != pos.end()) {
						return false;
					}
				}
			} else if (p1.second==p2.second){
				for (int i = min(p1.first,p2.first)+1; i < max(p1.first,p2.first); i++){
					if(find(pos.begin(), pos.end(), pair<int, int>{i, p1.second}) != pos.end()) {
						return false;
					}
				}
			} else {
				return false;
			}
			break;
		
		default:
			break;
	}

	// Check 4: Check if destination piece is from team
	if(found) {
		if (name[index][1] == n2){
			return false;
		}

		capture(index);
	}

	moved[selfIndex] = true;

	if (enpassantCapture!=-1){
		capture(enpassantCapture);
	}

	pjc[selfIndex] = setPjc;

	return true;
}

int main(){
	hd = FindWindowA("Progman", NULL);
	hd = FindWindowEx(hd, 0, "SHELLDLL_DefView", NULL);
	hd = FindWindowEx(hd, 0, "SysListView32", NULL);

	pcount = SendMessage(hd, LVM_GETITEMCOUNT, 0, 0);

	int index = pcount;
	for (int i = 0; i < 8; i++){
		for (int j = 0; j < 8; j++){
			if (names[i][j]=="") {
				continue;
			}
			string oname = names[i][j];

			if (oname[0] == 'r'){
				rookIndex.push_back(index-pcount);
			}
			
			string rname = oname.substr(0,2);
			duplicateFile("./assets/"+rname+".ico", oname+".ico");
			Sleep(1500);
			ListView_SetItemPosition(hd, index, xstart+(j+xoffset)*xinter, ystart+i*yinter);
			name.push_back(oname);
			pos.push_back({j, i});
			valid.push_back(true);
			pjc.push_back(false);
			moved.push_back(false);
			index++;
		}
	}

	GetWindowThreadProcessId(hd, &Pi);
	he = OpenProcess(PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ, false, Pi);
	
	POINT *pC = (POINT*) VirtualAllocEx(he, NULL, sizeof(POINT), MEM_COMMIT, PAGE_READWRITE);
	WriteProcessMemory(he, pC, &pC, sizeof(POINT), NULL);

	// cursor:
	// GetCursorPos(&pt);
	
	// Game loop
	while (true) {
		if (stopped){
			break;
		}
		

		Sleep(1000);
		ncount = SendMessage(hd, LVM_GETITEMCOUNT, 0, 0);
		for (int i = pcount; i < ncount; i++){
			// real index
			const int r = i-pcount;

			if (!valid[r]){
				continue;
			}
			

			POINT pt;
			ListView_GetItemPosition(hd, i, pC);
			ReadProcessMemory(he, pC, &pt, sizeof(POINT), NULL);

			pair<int,int> oldPt = pos[r];
			pair<int,int> newPt = {round((pt.x-xstart)/xinter)-xoffset, round((pt.y-ystart)/yinter)};
			
			if (pt.x != (pos[r].first+xoffset)*xinter+xstart || pt.y != pos[r].second*yinter+ystart) {
				bool success = checkMove(name[r][0], name[r][1], oldPt, newPt, r);

				if (success){
					pos[r] = {newPt.first, newPt.second};
					ListView_SetItemPosition(hd, i, (newPt.first+xoffset)*xinter+xstart, newPt.second*yinter+ystart);
					cout << name[r] << " made a valid move\n";
					
					
					// Pawn promotion
					if (name[r][0] == 'p'){
						int promotePos = name[r][1]=='l'? 0 : 7, newIndex = name[r][1]=='l'? queens.first++ : queens.second++;
						if (newPt.second == promotePos){
							fs::rename(desktopPath+name[r]+".ico", desktopPath+"q"+name[r][1]+name[r][2]+".ico");
							name[r][0] = 'q';
						}
					}
					
					whiteTurn = !whiteTurn;

				} else {
					ListView_SetItemPosition(hd, i, (pos[r].first+xoffset)*xinter+xstart, pos[r].second*yinter+ystart);
					cout << name[r] << " made an invalid move\n";
				}
			}
		}
	}

	VirtualFreeEx(he, pC, 0, MEM_RELEASE);

	return 0;
}