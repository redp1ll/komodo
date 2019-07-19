/******************************************************************************
 * Copyright � 2014-2019 The SuperNET Developers.                             *
 *                                                                            *
 * See the AUTHORS, DEVELOPER-AGREEMENT and LICENSE files at                  *
 * the top-level directory of this distribution for the individual copyright  *
 * holder information and the developer policies on copyright and licensing.  *
 *                                                                            *
 * Unless otherwise agreed in a custom licensing agreement, no part of the    *
 * SuperNET software, including this file may be copied, modified, propagated *
 * or distributed except according to the terms contained in the LICENSE file *
 *                                                                            *
 * Removal or modification of this copyright notice is prohibited.            *
 *                                                                            *
 ******************************************************************************/

#include <stdint.h>
#include <string.h>
#include <numeric>
//#include <boost/assign/list_of.hpp>

#include "univalue.h"
#include "amount.h"
//#include "consensus/upgrades.h"
//#include "core_io.h"
//#include "init.h"
//#include "key_io.h"
//#include "main.h"
//#include "komodo_defs.h"  //should be included after main.h
//#include "net.h"
//#include "netbase.h"
#include "rpc/server.h"
#include "rpc/protocol.h"
//#include "timedata.h"
//#include "transaction_builder.h"
//#include "util.h"
//#include "utilmoneystr.h"
//#include "primitives/transaction.h"
//#include "zcbenchmarks.h"
//#include "script/interpreter.h"
//#include "zcash/zip32.h"
//#include "notaries_staked.h"

#include "../wallet/crypter.h"

#include "cc/CCKogs.h"
#include "cc/CCinclude.h"

using namespace std;

static UniValue KogsCreateGameObjects(const UniValue& params, bool isKogs)
{
    UniValue result(UniValue::VOBJ), jsonParams(UniValue::VOBJ);
    CCerror.clear();

    if (params[1].getType() == UniValue::VOBJ)
        jsonParams = params[1].get_obj();
    else if (params[1].getType() == UniValue::VSTR)  // json in quoted string '{...}'
        jsonParams.read(params[1].get_str().c_str());
    if (jsonParams.getType() != UniValue::VOBJ || jsonParams.empty())
        throw runtime_error("parameter 1 must be object\n");
    std::cerr << __func__ << " test output jsonParams=" << jsonParams.write(0, 0) << std::endl;

    std::vector<KogsMatchObject> gameobjects;
    std::vector<std::string> paramkeys = jsonParams.getKeys();
    std::vector<std::string>::const_iterator iter;

    iter = std::find(paramkeys.begin(), paramkeys.end(), isKogs ? "kogs" : "slammers");
    if (iter != paramkeys.end()) {
        UniValue jsonArray = jsonParams[iter - paramkeys.begin()].get_array();
        if (!jsonArray.isArray())
            throw runtime_error("'kogs' or 'slammers' parameter value is not an array\n");
        if (jsonArray.size() == 0)
            throw runtime_error("'kogs' or 'slammers' array is empty\n");


        for (int i = 0; i < jsonArray.size(); i++)
        {
            std::vector<std::string> ikeys = jsonArray[i].getKeys();
            
            struct KogsMatchObject gameobj;
            gameobj.InitGameObject(isKogs ? KOGSID_KOG : KOGSID_SLAMMER); // set basic ids

            int paramcount = 0;
            // parse json array item with kog data:

            iter = std::find(ikeys.begin(), ikeys.end(), "nameId");
            if (iter != ikeys.end()) {
                gameobj.nameId = jsonArray[i][iter - ikeys.begin()].get_str();
                std::cerr << __func__ << " test output gameobj.nameId=" << gameobj.nameId << std::endl;
                paramcount++;
            }
            iter = std::find(ikeys.begin(), ikeys.end(), "descriptionId");
            if (iter != ikeys.end()) {
                gameobj.descriptionId = jsonArray[i][iter - ikeys.begin()].get_str();
                std::cerr << __func__ << " test output gameobj.descriptionId=" << gameobj.descriptionId << std::endl;
                paramcount++;
            }
            iter = std::find(ikeys.begin(), ikeys.end(), "imageId");
            if (iter != ikeys.end()) {
                gameobj.imageId = jsonArray[i][iter - ikeys.begin()].get_str();
                std::cerr << __func__ << " test output gameobj.imageId=" << gameobj.imageId << std::endl;
                paramcount++;
            }

            if (paramcount < 3)
                throw runtime_error("not all required game object data passed\n");

            gameobjects.push_back(gameobj);
        }
    }

    std::vector<std::string> hextxns = KogsCreateGameObjectNFTs(gameobjects);
    if (CCerror.empty())
        RETURN_IF_ERROR(CCerror);

    UniValue resarray(UniValue::VARR);
    for (int i = 0; i < hextxns.size(); i++)
    {
        resarray.push_back(hextxns[i]);
    }
    result.push_back(std::make_pair("rawtxns", resarray));
    return result;
}

UniValue kogscreatekogs(const UniValue& params, bool fHelp)
{
    UniValue result(UniValue::VOBJ), jsonParams(UniValue::VOBJ);

    if (fHelp || (params.size() != 1))
    {
        throw runtime_error(
            "kogscreatekogs '{\"kogs\":[{\"nameId\":\"string\", \"descriptionId\":\"string\",\"imageId\":\"string\",\"setId\":\"string\",\"subsetId\":\"string\"}]}'\n"
            "creates array of kog NFT creation transactions to be sent via sendrawtransaction rpc\n" "\n");
    }
    return KogsCreateGameObjects(params, true);
}

UniValue kogscreateslammers(const UniValue& params, bool fHelp)
{
    UniValue result(UniValue::VOBJ), jsonParams(UniValue::VOBJ);

    if (fHelp || (params.size() != 1))
    {
        throw runtime_error(
            "kogscreateslammers '{\"slammers\":[{\"nameId\":\"string\", \"descriptionId\":\"string\",\"imageId\":\"string\",\"setId\":\"string\",\"subsetId\":\"string\"}]}'\n"
            "creates array of slammer NFT creation transactions to be sent via sendrawtransaction rpc\n" "\n");
    }
    return KogsCreateGameObjects(params, false);
}

UniValue kogscreatepack(const UniValue& params, bool fHelp)
{
    UniValue result(UniValue::VOBJ), jsonParams(UniValue::VOBJ);
    CCerror.clear();

    if (fHelp || (params.size() != 3))
    {
        throw runtime_error(
            "kogscreatepack packsize encryptkey initvector\n"
            "creates a pack with the 'number' of randomly selected kogs. The pack content is encrypted (to decrypt it later after purchasing)\n" "\n");
    }

    int32_t packsize = atoi(params[0].get_str());
    if (packsize <= 0)
        throw runtime_error("packsize should be positive number\n");

    std::string enckeystr = params[1].get_str();
    if (enckeystr.length() != WALLET_CRYPTO_KEY_SIZE)
        throw runtime_error(std::string("encryption key length should be ") + std::to_string(WALLET_CRYPTO_KEY_SIZE) + std::string("\n"));
    vuint8_t enckey(enckeystr.begin(), enckeystr.end());

    std::string ivstr = params[1].get_str();
    if (ivstr.length() != WALLET_CRYPTO_SALT_SIZE)
        throw runtime_error(std::string("ini vector length should be ") + std::to_string(WALLET_CRYPTO_SALT_SIZE) + std::string("\n"));
    vuint8_t iv(ivstr.begin(), ivstr.end());

    std::string hextx = KogsCreatePack(packsize, enckey, iv);
    RETURN_IF_ERROR(CCerror);

    return hextx;
}

UniValue kogsunsealpack(const UniValue& params, bool fHelp)
{
    UniValue result(UniValue::VARR), jsonParams(UniValue::VOBJ);
    CCerror.clear();

    if (fHelp || (params.size() != 3))
    {
        throw runtime_error(
            "kogsunsealpack packid encryptkey initvector\n"
            "unseals pack (decrypts its content) and sends kog tokens to the pack owner\n" "\n");
    }

    uint256 packid = Parseuint256(params[0].get_str().c_str());
    if (packid.IsNull())
        throw runtime_error("packid incorrect\n");

    std::string enckeystr = params[1].get_str();
    if (enckeystr.length() != WALLET_CRYPTO_KEY_SIZE)
        throw runtime_error(std::string("encryption key length should be ") + std::to_string(WALLET_CRYPTO_KEY_SIZE) + std::string("\n"));
    vuint8_t enckey(enckeystr.begin(), enckeystr.end());

    std::string ivstr = params[1].get_str();
    if (ivstr.length() != WALLET_CRYPTO_SALT_SIZE)
        throw runtime_error(std::string("init vector length should be ") + std::to_string(WALLET_CRYPTO_SALT_SIZE) + std::string("\n"));
    vuint8_t iv(ivstr.begin(), ivstr.end());

    std::vector<std::string> hextxns = KogsUnsealPackToOwner(packid, enckey, iv);
    RETURN_IF_ERROR(CCerror);

    for (auto hextx : hextxns)
    {
        result.push_back(std::make_pair("hextx", hextx));
    }
    return result;
}

static const CRPCCommand commands[] =
{ //  category              name                      actor (function)         okSafeMode
  //  --------------------- ------------------------  -----------------------  ----------
    { "kogs",         "kogscreatekogs",         &kogscreatekogs,          true },
    { "kogs",         "kogscreateslammers",     &kogscreateslammers,      true },
    { "kogs",         "kogscreatepack",         &kogscreatepack,          true },
    { "kogs",         "kogsunsealpack",         &kogsunsealpack,          true }
};

void RegisterCCRPCCommands(CRPCTable &tableRPC)
{
    for (unsigned int vcidx = 0; vcidx < ARRAYLEN(commands); vcidx++)
        tableRPC.appendCommand(commands[vcidx].name, &commands[vcidx]);
}
