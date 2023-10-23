#ifndef __STATE_H__
#define __STATE_H__

class DosHeaderState;
class NTHeaderState;
class FileHeaderState;
class OptionalHeaderState;
class DataDirectoryState;
class ExportDirectoryState;
class ImportDirectoryState;
class SectionHeaderState;
class SectionState;

class State
{
public :
	virtual void Enter() {}
	virtual void Escape();

	virtual ~State(){}

	static DosHeaderState* dosheader;
	static NTHeaderState* ntheader;
	static FileHeaderState* fileheader;
	static OptionalHeaderState* optionalheader;
	static DataDirectoryState* datadirectory;
	static ExportDirectoryState* exportdirectory;
	static ImportDirectoryState* importdirectory;
	static SectionHeaderState* sectionheader;
	static SectionState* section;
};

class DosHeaderState : public State
{
public :
	virtual void Enter() override;

private :

};

class NTHeaderState : public State
{
public :
	virtual void Enter() override;
	virtual void Escape() override;

private :

};

class FileHeaderState : public State
{
public :
	virtual void Enter() override;

private :

};

class OptionalHeaderState : public State
{
public :
	virtual void Enter() override;

private :

};

class DataDirectoryState : public State
{
public :
	virtual void Enter() override;
	virtual void Escape() override;

private :

};

class ExportDirectoryState : public State
{
public :
	virtual void Enter() override;
	virtual void Escape() override;
	void SetnFunc(int nfunc) {nFunc = nfunc;}
	int GetnFunc() {return nFunc;}

private :
	int nFunc;
};

class ImportDirectoryState : public State
{
public :
	virtual void Enter() override;
	virtual void Escape() override;
	void SetSize(int size) {sizeofIID = size;}
	void SetIdxTable() {IdxTable = new int[sizeofIID];}
	void SetFuncTable();

	int SearchIdxTable(int idx) 
	{
		for(int i = 0; i < sizeofIID; i++)
			if(IdxTable[i] == idx)
				return i;

		return -1;
	}

	int SearchFuncTable(int idx) {return FuncTable[idx];}
	int* GetIdxTable() {return IdxTable;}
	int* GetFuncTable() {return FuncTable;}

private :
	int sizeofIID;
	int* IdxTable;
	int* FuncTable;
};

class SectionHeaderState : public State
{
public :
	virtual void Enter() override;
	virtual void Escape() override;

private :

};

class SectionState : public State
{
public :
	virtual void Enter() override;
	void SetIdx(int idx) {SectionIdx = idx;}

private :
	int SectionIdx;
};

#endif