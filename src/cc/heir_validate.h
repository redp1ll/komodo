#ifndef HEIR_VALIDATE_H
#define HEIR_VALIDATE_H

#include "CCinclude.h"
#include "CCHeir.h"

#define HEIR_NOTFOUND (-1)
#define HEIR_COINS 1
#define HEIR_TOKENS 2
#define IS_CHARINSTR(c, str) (std::string(str).find((char)(c)) != std::string::npos)


// makes coin initial tx opret
CScript EncodeHeirCreateOpRet(uint8_t eval, uint8_t funcid, CPubKey ownerPubkey, CPubKey heirPubkey, int64_t inactivityTimeSec, std::string heirName);

// makes coin additional tx opret
CScript EncodeHeirOpRet(uint8_t eval, uint8_t funcid, CPubKey ownerPubkey, CPubKey heirPubkey, int64_t inactivityTimeSec, uint256 fundingtxid);
CScript EncodeHeirAssetsCreateOpRet(uint8_t eval, uint8_t funcid, uint256 assetid, CPubKey ownerPubkey, CPubKey heirPubkey, int64_t inactivityTimeSec, std::string hearName);
CScript EncodeHeirAssetsOpRet(uint8_t eval, uint8_t funcid, uint256 assetid, CPubKey ownerPubkey, CPubKey heirPubkey, int64_t inactivityTimeSec, uint256 fundingtxid);
//CScript EncodeHeirConvertedAssetsOpRet(uint8_t eval, uint8_t funcid, uint256 assetid, CPubKey ownerPubkey, CPubKey heirPubkey, int64_t inactivityTimeSec, uint256 fundingtxid);

template <class Helper> uint256 FindLatestFundingTx(uint256 fundingtxid, uint256 &assetid, CScript& opRetScript, bool &isHeirSpendingBegan);
template <class Helper> uint8_t DecodeHeirOpRet(CScript scriptPubKey, uint256 &assetid, uint256& fundingtxid, bool noLogging = false);
template <class Helper> uint8_t DecodeHeirOpRet(CScript scriptPubKey, uint256 &assetid, CPubKey& ownerPubkey, CPubKey& heirPubkey, int64_t& inactivityTime, std::string& heirName, bool noLogging = false);
template <class Helper> uint8_t DecodeHeirOpRet(CScript scriptPubKey, uint256 &assetid, CPubKey& ownerPubkey, CPubKey& heirPubkey, int64_t& inactivityTime, std::string& heirName, uint256& txidInOpret, bool noLogging = false);

//int64_t AddHeirTokenInputs(struct CCcontract_info *cp, CMutableTransaction &mtx, CPubKey pk, uint256 refassetid, int64_t total, int32_t maxinputs);


// helper class to allow polymorphic behaviour for HeirXXX() functions in case of coins
class CoinHelper {
public:

	static bool isMyFuncId(uint8_t funcid) { return IS_CHARINSTR(funcid, "FAC"); }
	static uint8_t getMyEval() { return EVAL_HEIR; }
	static int64_t addOwnerInputs(struct CCcontract_info* cp, uint256 dummyid, CMutableTransaction& mtx, CPubKey ownerPubkey, int64_t total, int32_t maxinputs) {
		return AddNormalinputs(mtx, ownerPubkey, total, maxinputs);
	}

	static CScript makeCreateOpRet(uint256 dummyid, CPubKey ownerPubkey, CPubKey heirPubkey, int64_t inactivityTimeSec, std::string heirName) {
		return EncodeHeirCreateOpRet((uint8_t)EVAL_HEIR, (uint8_t)'F', ownerPubkey, heirPubkey, inactivityTimeSec, heirName);
	}
	static CScript makeAddOpRet(uint256 dummyid, CPubKey ownerPubkey, CPubKey heirPubkey, int64_t inactivityTimeSec, uint256 fundingtxid) {
		return EncodeHeirOpRet((uint8_t)EVAL_HEIR, (uint8_t)'A', ownerPubkey, heirPubkey, inactivityTimeSec, fundingtxid);
	}
	static CScript makeClaimOpRet(uint256 dummyid, CPubKey ownerPubkey, CPubKey heirPubkey, int64_t inactivityTimeSec, uint256 fundingtxid) {
		return EncodeHeirOpRet((uint8_t)EVAL_HEIR, (uint8_t)'C', ownerPubkey, heirPubkey, inactivityTimeSec, fundingtxid);
	}

	static void UnmarshalOpret(std::vector<uint8_t> vopret, uint8_t &e, uint8_t &funcId, uint256 &dummyAssetid, CPubKey& ownerPubkey, CPubKey& heirPubkey, int64_t& inactivityTime, std::string& heirName, uint256& txidInOpret) {
		E_UNMARSHAL(vopret, { ss >> e; ss >> funcId; ss >> ownerPubkey; ss >> heirPubkey; ss >> inactivityTime; if (IS_CHARINSTR(funcId, "F")) { ss >> heirName; } if (IS_CHARINSTR(funcId, "AC")) { ss >> txidInOpret; } });
	}

	static bool isSpendingTx(uint8_t funcid) { return (funcid == 'C'); }

	static CTxOut makeUserVout(int64_t amount, CPubKey myPubkey) {
		return CTxOut(amount, CScript() << ParseHex(HexStr(myPubkey)) << OP_CHECKSIG);
	}
	static CTxOut makeClaimerVout(int64_t amount, CPubKey myPubkey) {
		return CTxOut(amount, CScript() << ParseHex(HexStr(myPubkey)) << OP_CHECKSIG);
	}
};

// helper class to allow polymorphic behaviour for HeirXXX() functions in case of tokens
class TokenHelper {
public:

	static bool isMyFuncId(uint8_t funcid) { return IS_CHARINSTR(funcid, "FAC"); }   
	static uint8_t getMyEval() { return EVAL_ASSETS; }
	static int64_t addOwnerInputs(struct CCcontract_info* cp, uint256 assetid, CMutableTransaction& mtx, CPubKey ownerPubkey, int64_t total, int32_t maxinputs) {
		return AddAssetInputs(cp, mtx, ownerPubkey, assetid, total, maxinputs);
	}

	static CScript makeCreateOpRet(uint256 assetid, CPubKey ownerPubkey, CPubKey heirPubkey, int64_t inactivityTimeSec, std::string heirName) {
		return EncodeHeirAssetsCreateOpRet((uint8_t)EVAL_HEIR, (uint8_t)'F', assetid, ownerPubkey, heirPubkey, inactivityTimeSec, heirName);
	}
	static CScript makeAddOpRet(uint256 assetid, CPubKey ownerPubkey, CPubKey heirPubkey, int64_t inactivityTimeSec, uint256 fundingtxid) {
		return EncodeHeirAssetsOpRet((uint8_t)EVAL_HEIR, (uint8_t)'A', assetid, ownerPubkey, heirPubkey, inactivityTimeSec, fundingtxid);
	}
	static CScript makeClaimOpRet(uint256 assetid, CPubKey ownerPubkey, CPubKey heirPubkey, int64_t inactivityTimeSec, uint256 fundingtxid) {
		return EncodeHeirAssetsOpRet((uint8_t)EVAL_HEIR, (uint8_t)'C', assetid, ownerPubkey, heirPubkey, inactivityTimeSec, fundingtxid);
	}

	static void UnmarshalOpret(std::vector<uint8_t> vopret, uint8_t &e, uint8_t &funcId, uint256 &assetid, CPubKey& ownerPubkey, CPubKey& heirPubkey, int64_t& inactivityTime, std::string& heirName, uint256& txidInOpret) {
		uint8_t assetFuncId = '\0';
		bool result = E_UNMARSHAL(vopret, { ss >> e; ss >> assetFuncId; ss >> assetid; ss >> funcId; ss >> ownerPubkey; ss >> heirPubkey; ss >> inactivityTime; if (IS_CHARINSTR(funcId, "F")) { ss >> heirName; } if (IS_CHARINSTR(funcId, "AC")) { ss >> txidInOpret; } });
		if (!result /*|| assetFuncId != 't' -- any tx is ok*/) 
			funcId = 0;
	}
	static bool isSpendingTx(uint8_t funcid) { return (funcid == 'C'); }

	static CTxOut makeUserVout(int64_t amount, CPubKey myPubkey) {
		return MakeCC1vout(EVAL_ASSETS, amount, myPubkey);
	}
	static CTxOut makeClaimerVout(int64_t amount, CPubKey myPubkey) {
		return MakeCC1vout(EVAL_ASSETS, amount, myPubkey);
	}
};

//#define OPTIONAL_VOUT 0			// if vout is optional then in a validation plan it will be skipped without error, if all validators return false 



/**
* Small framework for vins and vouts validation implementing a variation of 'chain of responsibility' pattern:
* It consists of two classes CInputValidationPlan and COutputValidationPlan which both are configured with an array of vectors of validators
* (These validators are derived from the class CValidatorBase).
*
* A example of a validator may verify for a vout if its public key corresponds to the public key which is stored in opreturn.
* Or, vin validator may check if this vin depicts correctly to the CC contract's address.
*
* For validating vins CInputValidator additionally is provided with an instance of a class derived from the CInputIdentifierBase class.
* this identifier class allows to select identical vins (for example, normal vins or cc input vins) and apply validators from the corresponding vector to it.
* Note: CInputValidator treats that at least one identified vin should be present, otherwise it returns eval->invalid() and false.
*
* For validating vouts COutputValidator is configured for each vector of validators with the vout index to which these validators are applied
* (see constructors of both CInputValidator and COutputValidator)
*
*
* Base class for all validators
*/
/**
 * base class for all validators
 */
class CValidatorBase
{
public:
	CValidatorBase(CCcontract_info* cp) : m_cp(cp) {}
	virtual bool isVinValidator() const = 0;
	virtual bool validateVin(CTxIn vin, CTxOut prevVout, std::string& message) const = 0;
	virtual bool validateVout(CTxOut vout, std::string& message) const = 0;

protected:
	CCcontract_info * m_cp;
};


/**
 * Base class for classes which identify vins as normal or cc inputs
 */
class CInputIdentifierBase
{
public:
	CInputIdentifierBase(CCcontract_info* cp) : m_cp(cp) {}
	virtual std::string inputName() const = 0;
	virtual bool identifyInput(CTxIn vin) const = 0;
protected:
	CCcontract_info * m_cp;
};




/**
* Encapsulates an array containing rows of validators
* Each row is a vector of validators (zero is possible) for validating vins or prev tx's vouts
* this validation plan is used for validating tx inputs
*/
template <typename TValidatorBase>
class CInputValidationPlan
{
	using ValidatorsRow = std::vector<TValidatorBase*>;

public:

	// Pushes a row of validators for validating a vin or vout
	// @param CInputIdentifierBase* pointer to class-identifier which determines several identical adjacent vins (like in schema "vin.0+: normal inputs")
	// @param pargs parameter pack of zero or more pointer to validator objects
	// Why pointers? because we store the base class in validators' row and then call its virtual functions
	template <typename TValidatorBaseX, typename... ARGS>
	void pushValidators(CInputIdentifierBase *identifier, ARGS*... pargs) // validators row passed as variadic arguments CValidatorX *val1, CValidatorY *val2 ...
	{
		ValidatorsRow vValidators({ (TValidatorBase*)pargs... });
		m_arrayValidators.push_back(std::make_pair(identifier, vValidators));
	}

	// validate tx inputs and corresponding prev tx vouts
	bool validate(const CTransaction& tx, Eval* eval)
	{
		std::string message = "<empty>";
		//std::cerr << "CInputValidationPlan::validate() starting vins validation..." << std::endl;

		int32_t ival = 0;
		int32_t iv = 0;
		int32_t numv = tx.vin.size();
		int32_t numValidators = m_arrayValidators.size();
		
		// run over vins:
		while (iv < numv && ival < numValidators) {

			int32_t identifiedCount = 0;
			CInputIdentifierBase *identifier = m_arrayValidators[ival].first;
			//					check if this is 'our' input:
			while (iv < numv && identifier->identifyInput(tx.vin[iv])) {
				
				// get prev tx:
				CTransaction prevTx, *pPrevTxOrNull = NULL;
				uint256 hashBlock;

				if (!eval->GetTxUnconfirmed(tx.vin[iv].prevout.hash, prevTx, hashBlock)) {
					std::ostringstream stream;
					stream << "can't find vinTx for vin=" << iv << ".";
					return eval->Invalid(stream.str().c_str());
				}
				pPrevTxOrNull = &prevTx;  // TODO: get prev tx only if it required (i.e. if vout validators are present)

				// exec 'validators' from validator row of ival index, for tx.vin[iv]
				if (!execValidatorsInRow(&tx, pPrevTxOrNull, iv, ival, message)) {
					std::ostringstream stream;
					stream << "invalid tx vin[" << iv << "]:" << message;
					return eval->Invalid(stream.str().c_str());				// ... if not, return 'invalid'
				}

				identifiedCount++;		// how many vins we identified
				iv++;					// advance to the next vin
			}

			// CInputValidationPlan treats that there must be at least one identified vin for configured validators' row
			// like in 'vin.0: normal input'
			if (identifiedCount == 0) {
				std::ostringstream stream;
				stream << "can't find required vins for " << identifier->inputName() << ".";
				return eval->Invalid(stream.str().c_str());
			}

			ival++;   // advance to the next validator row
					  // and it will try the same vin with the new CInputIdentifierBase and validators row
		}

		// validation is successful if all validators have been used (i.e. ival = numValidators)
		if (ival < numValidators) {
			std::cerr << "CInputValidationPlan::validate() incorrect tx" << " ival=" << ival << " numValidators=" << numValidators << std::endl;
			return eval->Invalid("incorrect tx structure: not all required vins are present.");
		}

		std::cerr << "CInputValidationPlan::validate() returns with true" << std::endl;
		return true;
	}

private:
	// Executes validators from the requested row of validators (selected by iValidators) for selected vin or vout (selected by iv)
	bool execValidatorsInRow(const CTransaction* pTx, const CTransaction* pPrevTx, int32_t iv, int32_t ival, std::string& refMessage) const
	{
		// check boundaries:
		if (ival < 0 || ival >= m_arrayValidators.size()) {
			std::cerr << "CInputValidationPlan::execValidatorsInRow() internal error: incorrect param ival=" << ival << " size=" << m_arrayValidators.size();
			refMessage = "internal error: incorrect param ival index";
			return false;
		}

		if (iv < 0 || iv >= pTx->vin.size()) {
			std::cerr << "CInputValidationPlan::execValidatorsInRow() internal error: incorrect param iv=" << iv << " size=" << m_arrayValidators.size();
			refMessage = "internal error: incorrect param iv index";
			return false;
		}

		// get requested row of validators:
		ValidatorsRow vValidators = m_arrayValidators[ival].second;

		std::cerr << "CInputValidationPlan::execValidatorsInRow() calling validators" << " for vin iv=" << iv << " ival=" << ival << std::endl;

		for (auto v : vValidators) {
			bool result;

			if (v->isVinValidator())
				// validate this vin and previous vout:
				result = v->validateVin(pTx->vin[iv], pPrevTx->vout[pTx->vin[iv].prevout.n], refMessage);
			else
				// if it is vout validator pass the previous tx vout:
				result = v->validateVout( pPrevTx->vout[pTx->vin[iv].prevout.n], refMessage);
			if (!result) {
				return result;
			}
		}
		return true; // validation OK
	}


private:
	//std::map<CInputIdentifierBase*, ValidatorsRow> m_arrayValidators;
	std::vector< std::pair<CInputIdentifierBase*, ValidatorsRow> > m_arrayValidators;

};


/**
* Encapsulates an array containing rows of validators
* Each row is a vector of validators (zero is possible) for validating vouts
* this validation plan is used for validating tx outputs
*/
template <typename TValidatorBase>
class COutputValidationPlan
{
	using ValidatorsRow = std::vector<TValidatorBase*>;

public:
	// Pushes a row of validators for validating a vout
	// @param ivout index to vout to validate
	// @param pargs parameter pack of zero or more pointer to validator objects
	// Why pointers? because we store base class and call its virtual functions

	template <typename TValidatorBaseX, typename... ARGS>
	void pushValidators(int32_t ivout, ARGS*... pargs) // validators row passed as variadic arguments CValidatorX *val1, CValidatorY *val2 ...
	{
		ValidatorsRow vValidators({ (TValidatorBase*)pargs... });
		m_arrayValidators.push_back(std::make_pair(ivout, vValidators));
	}

	// validate tx outputs 
	bool validate(const CTransaction& tx, Eval* eval)
	{
		std::string message = "<empty>";
		//std::cerr << "COutputValidationPlan::validateOutputs() starting vouts validation..." << std::endl;

		int32_t ival = 0;
		int32_t numVouts = tx.vout.size();
		int32_t numValidators = m_arrayValidators.size();

		// run over vouts:
		while (ival < numValidators) {

			int32_t ivout = m_arrayValidators[ival].first;
			if (ivout >= numVouts) {
				std::cerr << "COutputValidationPlan::validate() incorrect tx" << "for ival=" << ival << " in tx.vout no such ivout=" << ivout << std::endl;
				return eval->Invalid("incorrect tx structure: not all required vouts are present.");
			}
			else
			{
				// exec 'validators' from validator row of ival index, for tx.vout[ivout]
				if (!execValidatorsInRow(&tx, ivout, ival, message)) {
					std::ostringstream stream;
					stream << "invalid tx vout[" << ivout << "]:" << message;
					return eval->Invalid(stream.str().c_str());				// ... if not, return 'invalid'
				}
			}

			ival++;					// advance to the next vout

		}

		std::cerr << "COutputValidationPlan::validate() returns with true" << std::endl;
		return true;
	}

private:
	// Executes validators from the requested row of validators (selected by iValidators) for selected vin or vout (selected by iv)
	bool execValidatorsInRow(const CTransaction* pTx, int32_t iv, int32_t ival, std::string& refMessage) const
	{
		// check boundaries:
		if (ival < 0 || ival >= m_arrayValidators.size()) {
			std::cerr << "COutputValidationPlan::execValidatorsInRow() internal error: incorrect param ival=" << ival << " size=" << m_arrayValidators.size();
			refMessage = "internal error: incorrect param ival index";
			return false;
		}

		if (iv < 0 || iv >= pTx->vout.size()) {
			std::cerr << "COutputValidationPlan::execValidatorsInRow() internal error: incorrect param iv=" << iv << " size=" << m_arrayValidators.size();
			refMessage = "internal error: incorrect param iv index";
			return false;
		}

		// get requested row of validators:
		ValidatorsRow vValidators = m_arrayValidators[ival].second;

		std::cerr << "COutputValidationPlan::execRow() calling validators" << " for vout iv=" << iv << " ival=" << ival << std::endl;

		for (auto v : vValidators) {

			if (!v->isVinValidator())	{
				// if this is a 'in' validation plan then pass the previous tx vout:
				bool result = v->validateVout(pTx->vout[iv], refMessage);
				if (!result) 
					return result;
			}
		}
		return true; // validation OK
	}


private:
	//std::map<int32_t, ValidatorsRow> m_mapValidators;
	std::vector< std::pair<int32_t, ValidatorsRow> > m_arrayValidators;

};


class CNormalInputIdentifier : CInputIdentifierBase  {
public:
	CNormalInputIdentifier(CCcontract_info* cp) : CInputIdentifierBase(cp) {}
	virtual std::string inputName() const { return std::string("normal input"); }
	virtual bool identifyInput(CTxIn vin) const {
		return !IsCCInput(vin.scriptSig);
	}
};

class CCCInputIdentifier : CInputIdentifierBase {
public:
	CCCInputIdentifier(CCcontract_info* cp) : CInputIdentifierBase(cp) {}
	virtual std::string inputName() const { return std::string("CC input"); }
	virtual bool identifyInput(CTxIn vin) const {
		return IsCCInput(vin.scriptSig);
	}
};


/**
* Validates 1of2address for vout (may be used for either this or prev tx)
*/
template <class Helper> class CCC1of2AddressValidator : CValidatorBase
{
public:
	CCC1of2AddressValidator(CCcontract_info* cp, CScript opRetScript, std::string customMessage = "") :
		m_opRetScript(opRetScript), m_customMessage(customMessage), CValidatorBase(cp) {}

	virtual bool isVinValidator() const { return false; }
	virtual bool validateVout(CTxOut vout, std::string& message) const
	{
		//std::cerr << "CCC1of2AddressValidator::validateVout() entered" << std::endl;
		uint8_t funcId;
		CPubKey ownerPubkey, heirPubkey;
		int64_t inactivityTime;
		std::string heirName;
		uint256 assetid;

		if ((funcId = DecodeHeirOpRet<Helper>(m_opRetScript, assetid, ownerPubkey, heirPubkey, inactivityTime, heirName)) == 0) {
			message = m_customMessage + std::string(" invalid opreturn format");
			std::cerr << "CCC1of2AddressValidator::validateVout() exits with false: " << message << std::endl;
			return false;
		}

		char shouldBeAddr[65], ccAddr[65];

		GetCCaddress1of2(m_cp, shouldBeAddr, ownerPubkey, heirPubkey);
		if (vout.scriptPubKey.IsPayToCryptoCondition()) {
			if (Getscriptaddress(ccAddr, vout.scriptPubKey) && strcmp(shouldBeAddr, ccAddr) == 0) {
				std::cerr << "CCC1of2AddressValidator::validateVout() exits with true" << std::endl;
				return true;
			}
			else {
				message = m_customMessage + std::string(" incorrect heir funding address: incorrect pubkey(s)");
			}
		}
		else {
			message = m_customMessage + std::string(" incorrect heir funding address: not a 1of2addr");
		}

		std::cerr << "CCC1of2AddressValidator::validateVout() exits with false: " << message << std::endl;
		return false;
	}
	virtual bool validateVin(CTxIn vin, CTxOut prevVout, std::string& message) const { return false; }

private:
	CScript m_opRetScript;
	std::string m_customMessage;
};


/**
* Validates if this is vout to owner or heir from opret (funding or change)
*/
template <class Helper> class CMyPubkeyVoutValidator : CValidatorBase
{
public:
	CMyPubkeyVoutValidator(CCcontract_info* cp, CScript opRetScript, bool checkNormals)
		: m_opRetScript(opRetScript), m_checkNormals(checkNormals), CValidatorBase(cp) {	}

	virtual bool isVinValidator() const { return false; }
	virtual bool validateVout(CTxOut vout, std::string& message) const
	{
		//std::cerr << "CMyPubkeyVoutValidator::validateVout() entered" << std::endl;

		uint8_t funcId;
		CPubKey ownerPubkey, heirPubkey;
		int64_t inactivityTime;
		std::string heirName;
		uint256 assetid;

		///std::cerr << "CMyPubkeyVoutValidator::validateVout() m_opRetScript=" << m_opRetScript.ToString() << std::endl;
		// get both pubkeys:
		if ((funcId = DecodeHeirOpRet<Helper>(m_opRetScript, assetid, ownerPubkey, heirPubkey, inactivityTime, heirName)) == 0) {
			message = std::string("invalid opreturn format");
			return false;
		}

		CScript ownerScript;
		CScript heirScript;
		if (m_checkNormals)  {
			ownerScript = CoinHelper::makeUserVout(vout.nValue, ownerPubkey).scriptPubKey;
			heirScript = CoinHelper::makeUserVout(vout.nValue, heirPubkey).scriptPubKey;
		}
		else  {
			ownerScript = Helper::makeUserVout(vout.nValue, ownerPubkey).scriptPubKey;
			heirScript = Helper::makeUserVout(vout.nValue, heirPubkey).scriptPubKey;
		}

		//std::cerr << "CMyPubkeyVoutValidator::validateVout() vout.scriptPubKey=" << vout.scriptPubKey.ToString() <<  " makeUserVout=" << Helper::makeUserVout(vout.nValue, ownerPubkey).scriptPubKey.ToString() << std::endl;

		// recreate scriptPubKey for owner and heir and compare it with that of the vout to check:
		if (vout.scriptPubKey == ownerScript ||	vout.scriptPubKey == heirScript) {
			// this is vout to owner or heir addr:
			std::cerr << "CMyPubkeyVoutValidator::validateVout() exits with true" << std::endl;
			return true;

		}

		std::cerr << "CMyPubkeyVoutValidator::validateVout() exits with false (not the owner's or heir's addresses)" << std::endl;
		return false;
	}
	virtual bool validateVin(CTxIn vin, CTxOut prevVout, std::string& message) const { return true; }

private:
	CScript m_opRetScript;
	uint256 m_lasttxid;
	bool m_checkNormals;
};

/**
* Check if the user is the heir and the heir is allowed to spend (duration > inactivityTime)
*/
template <class Helper> class CHeirSpendValidator : CValidatorBase
{
public:
	CHeirSpendValidator(CCcontract_info* cp, CScript opRetScript, uint256 latesttxid, bool isHeirSpendingBegan)
		: m_opRetScript(opRetScript), m_latesttxid(latesttxid), m_isHeirSpendingBegan(isHeirSpendingBegan), CValidatorBase(cp) {}

	virtual bool isVinValidator() const { return false; }
	virtual bool validateVout(CTxOut vout, std::string& message) const
	{
		//std::cerr << "CHeirSpendValidator::validateVout() entered" << std::endl;

		uint8_t funcId;
		CPubKey ownerPubkey, heirPubkey;
		int64_t inactivityTime;
		std::string heirName;
		uint256 assetid;


		// get heir pubkey:
		if ((funcId = DecodeHeirOpRet<Helper>(m_opRetScript, assetid, ownerPubkey, heirPubkey, inactivityTime, heirName)) == 0) {
			message = std::string("invalid opreturn format");
			return false;
		}

		int32_t numblocks;
		int64_t durationSec = CCduration(numblocks, m_latesttxid);

		// recreate scriptPubKey for heir and compare it with that of the vout:
		if (vout.scriptPubKey == Helper::makeClaimerVout(vout.nValue, heirPubkey).scriptPubKey) {
			// this is the heir is trying to spend
			if (!m_isHeirSpendingBegan && durationSec <= inactivityTime) {
				message = "heir is not allowed yet to spend funds";
				std::cerr << "CHeirSpendValidator::validateVout() heir is not allowed yet to spend funds" << std::endl;
				return false;
			}
			else {
				// heir is allowed to spend
				return true;
			}
		}

		std::cerr << "CHeirSpendValidator::validateVout() exits with true" << std::endl;

		// this is not heir:
		return true;
	}
	virtual bool validateVin(CTxIn vin, CTxOut prevVout, std::string& message) const { return true; }

private:
	CScript m_opRetScript;
	uint256 m_latesttxid;
	bool m_isHeirSpendingBegan;
};

/**
* Validates this opreturn and compares it with the opreturn from the previous tx
*/
template <class Helper> class COpRetValidator : CValidatorBase
{
public:
	COpRetValidator(CCcontract_info* cp, CScript lastOpRetScript)
		: m_lastOpRetScript(lastOpRetScript), CValidatorBase(cp) {}

	virtual bool isVinValidator() const { return false; }
	virtual bool validateVout(CTxOut vout, std::string& message) const
	{
		//std::cerr << "COpRetValidator::validateVout() entered" << std::endl;

		uint8_t funcId, lastFuncId;
		CPubKey ownerPubkey, heirPubkey, lastOwnerPubkey, lastHeirPubkey;
		int64_t inactivityTime, lastInactivityTime, durationSec = 0;
		std::string dummyName1, dummyName2;			// do not check heir name
		uint256 txidInOpRet = zeroid, lastTxidInOpret = zeroid, assetid;

		if ((funcId = DecodeHeirOpRet<Helper>(vout.scriptPubKey, assetid, ownerPubkey, heirPubkey, inactivityTime, dummyName1, txidInOpRet)) == 0) {
			message = std::string("invalid opreturn format");
			return false;
		}
		if ((lastFuncId = DecodeHeirOpRet<Helper>(m_lastOpRetScript, assetid, lastOwnerPubkey, lastHeirPubkey, lastInactivityTime, dummyName2, lastTxidInOpret)) == 0) {
			message = std::string("invalid last tx opreturn format");
			return false;
		}

		if (ownerPubkey != lastOwnerPubkey || heirPubkey != lastHeirPubkey || inactivityTime != lastInactivityTime) {
			message = std::string("invalid data in opreturn: owner and heir pubkeys, inactivityTime must be equal to values in the last tx");
			return false;
		}

		if (/*funcId != 'F' && funcId !='G' && txidInOpRet != zeroid ||*/					// if it is 'F' or 'G' there should be no txidInOpRet 
																					// if lastFuncId == 'F' || 'G' but we have found last tx by txidInOpRet - no check
			!IS_CHARINSTR(lastFuncId, "F") && txidInOpRet != lastTxidInOpret) {   // possible only if last tx opRet is corrupted 
			message = std::string("txid in opreturn not equal to last tx opret (corruption possible)");
			return false;
		}

		std::cerr << "COpRetValidator::validateVout() exits with true" << std::endl;
		return true;
	}
	virtual bool validateVin(CTxIn vin, CTxOut prevVout, std::string& message) const { return true; }

private:
	CScript m_lastOpRetScript;
};



#endif