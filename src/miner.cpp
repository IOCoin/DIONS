// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2012 The Bitcoin developers
// Copyright (c) 2013 The NovaCoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "txdb.h"
#include "miner.h"
#include "wallet.h"
#include "kernel.h"
#include "json/json_spirit_reader_template.h"
#include "json/json_spirit_writer_template.h"
#include "json/json_spirit_utils.h"
//#include <CLI/CLI.hpp>
#include <dvmone/dvmone.h>
#include <dvmc/mocked_host.hpp>
#include <dvmc/dvmc.h>
#include <dvmc/dvmc.hpp>
#include <dvmc/hex.hpp>
#include <dvmc/loader.h>
#include <dvmc/tooling.hpp>
#include <fstream>

using namespace std;
using dvmc::operator""_address;
using namespace json_spirit;

//////////////////////////////////////////////////////////////////////////////
//
// BitcoinMiner
//

extern unsigned int nMinerSleep;

extern void ListTransactions(const __wx__Tx& wtx, const string& strAccount, int nMinDepth, bool fLong, Array& ret);
extern bool collisionReference(const string& s, uint256& wtxInHash);
extern bool validate_serial_n(const string&, const string&,__wx__Tx&);
extern bool read_serial_n(const string&, const string&, __wx__Tx&);
extern __wx__Tx generateUpdate(const string& origin);
static const string code_hex_str = "608060405234801561001057600080fd5b5061017c806100206000396000f3fe608060405234801561001057600080fd5b506004361061002b5760003560e01c8063f15fad2314610030575b600080fd5b61003861004e565b60405161004591906100c4565b60405180910390f35b60606040518060400160405280600c81526020017f74657374206d6573736167650000000000000000000000000000000000000000815250905090565b6000610096826100e6565b6100a081856100f1565b93506100b0818560208601610102565b6100b981610135565b840191505092915050565b600060208201905081810360008301526100de818461008b565b905092915050565b600081519050919050565b600082825260208201905092915050565b60005b83811015610120578082015181840152602081019050610105565b8381111561012f576000848401525b50505050565b6000601f19601f830116905091905056fea264697066735822122077ae7640f98ab2783e9eaf216c0440651b7b59a7061342c8aba40b91d4def4b964736f6c63430008070033";
static const string ADDRESS    = "address";
static const string DESCRIPTOR = "descriptor";
int static FormatHashBlocks(void* pbuffer, unsigned int len)
{
    unsigned char* pdata = (unsigned char*)pbuffer;
    unsigned int blocks = 1 + ((len + 8) / 64);
    unsigned char* pend = pdata + 64 * blocks;
    memset(pdata + len, 0, 64 * blocks - len);
    pdata[len] = 0x80;
    unsigned int bits = len * 8;
    pend[-1] = (bits >> 0) & 0xff;
    pend[-2] = (bits >> 8) & 0xff;
    pend[-3] = (bits >> 16) & 0xff;
    pend[-4] = (bits >> 24) & 0xff;
    return blocks;
}

static const unsigned int pSHA256InitState[8] =
{0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19};

void SHA256Transform(void* pstate, void* pinput, const void* pinit)
{
    SHA256_CTX ctx;
    unsigned char data[64];

    SHA256_Init(&ctx);

    for (int i = 0; i < 16; i++)
        ((uint32_t*)data)[i] = ByteReverse(((uint32_t*)pinput)[i]);

    for (int i = 0; i < 8; i++)
        ctx.h[i] = ((uint32_t*)pinit)[i];

    SHA256_Update(&ctx, data, sizeof(data));
    for (int i = 0; i < 8; i++)
        ((uint32_t*)pstate)[i] = ctx.h[i];
}

// Some explaining would be appreciated
class COrphan
{
public:
    CTransaction* ptx;
    set<uint256> setDependsOn;
    double dPriority;
    double dFeePerKb;

    COrphan(CTransaction* ptxIn)
    {
        ptx = ptxIn;
        dPriority = dFeePerKb = 0;
    }

    void print() const
    {
        printf("COrphan(hash=%s, dPriority=%.1f, dFeePerKb=%.1f)\n",
               ptx->GetHash().ToString().substr(0,10).c_str(), dPriority, dFeePerKb);
        BOOST_FOREACH(uint256 hash, setDependsOn)
        printf("   setDependsOn %s\n", hash.ToString().substr(0,10).c_str());
    }
};


uint64_t nLastBlockTx = 0;
uint64_t nLastBlockSize = 0;
int64_t nLastCoinStakeSearchInterval = 0;

// We want to sort transactions by priority and fee, so:
typedef boost::tuple<double, double, CTransaction*> TxPriority;
class TxPriorityCompare
{
    bool byFee;
public:
    TxPriorityCompare(bool _byFee) : byFee(_byFee) { }
    bool operator()(const TxPriority& a, const TxPriority& b)
    {
        if (byFee)
        {
            if (a.get<1>() == b.get<1>())
                return a.get<0>() < b.get<0>();
            return a.get<1>() < b.get<1>();
        }
        else
        {
            if (a.get<0>() == b.get<0>())
                return a.get<1>() < b.get<1>();
            return a.get<0>() < b.get<0>();
        }
    }
};
/*
int main(int argc, const char** argv) noexcept
{
    using namespace dvmc;

    try
    {
        HexValidator Hex;

        std::string vm_config;
        std::string code_arg;
        int64_t track = 1000000;
        auto rev = DVMC_LATEST_STABLE_REVISION;
        std::string input_arg;
        auto create = false;
        auto bench = false;

        CLI::App app{"DVMC tool"};
        const auto& version_flag = *app.add_flag("--version", "Print version information and exit");
        const auto& vm_option =
            *app.add_option("--vm", vm_config, "DVMC VM module")->envname("DVMC_VM");

        auto& run_cmd = *app.add_subcommand("run", "Execute DVM pos_read")->fallthrough();
        run_cmd.add_option("code", code_arg, "Bytecode")
            ->required()
            ->check(Hex | CLI::ExistingFile);
        run_cmd.add_option("--track", track, "Execution track limit", true)
            ->check(CLI::Range(0, 1000000000));
        run_cmd.add_option("--rev", rev, "DVM revision", true);
        run_cmd.add_option("--input", input_arg, "Input bytes")->check(Hex | CLI::ExistingFile);
        run_cmd.add_flag(
            "--create", create,
            "Create new contract out of the code and then retrieve_desc_vx this contract with the input");
        run_cmd.add_flag(
            "--bench", bench,
            "Benchmark execution time (state modification may result in unexpected behaviour)");

        try
        {
            app.parse(argc, argv);

            dvmc::VM vm;
            if (vm_option.count() != 0)
            {
                dvmc_loader_error_code ec = DVMC_LOADER_UNSPECIFIED_ERROR;
                vm = VM{dvmc_load_and_configure(vm_config.c_str(), &ec)};
                if (ec != DVMC_LOADER_SUCCESS)
                {
                    const auto error = dvmc_last_error_msg();
                    if (error != nullptr)
                        std::cerr << error << "\n";
                    else
                        std::cerr << "Loading error " << ec << "\n";
                    return static_cast<int>(ec);
                }
            }

            // Handle the --version flag first and exit when present.
            if (version_flag)
            {
                if (vm)
                    std::cout << vm.name() << " " << vm.version() << " (" << vm_config << ")\n";

                std::cout << "DVMC " PROJECT_VERSION;
                if (argc >= 1)
                    std::cout << " (" << argv[0] << ")";
                std::cout << "\n";
                return 0;
            }

            if (run_cmd)
            {
                // For run command the --vm is required.
                if (vm_option.count() == 0)
                    throw CLI::RequiredError{vm_option.get_name()};

                std::cout << "Config: " << vm_config << "\n";

                const auto code_hex = load_hex(code_arg);
                const auto input_hex = load_hex(input_arg);
                // If code_hex or input_hex is not valid hex string an exception is thrown.
                return tooling::run(vm, rev, track, code_hex, input_hex, create, bench, std::cout);
            }

            return 0;
        }
        catch (const CLI::ParseError& e)
        {
            return app.exit(e);
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << "\n";
        return -1;
    }
    catch (...)
    {
        return -2;
    }
} */
/*
#include <dvmc/dvmc.hpp>
#include <dvmc/hex.hpp>
#include <dvmc/mocked_host.hpp>
#include <dvmc/tooling.hpp>
#include <chrono>
#include <ostream>

namespace dvmc::tooling
{
namespace
{
/// The address where a new contract is created with --create option.
constexpr auto create_address = 0xc9ea7ed000000000000000000000000000000001_address;

/// The track limit for contract creation.
constexpr auto create_track = 10'000'000;

auto bench(MockedHost& host,
           dvmc::VM& vm,
           dvmc_revision rev,
           const dvmc_message& msg,
           bytes_view code,
           const dvmc::result& expected_result,
           std::ostream& out)
{
    {
        using clock = std::chrono::steady_clock;
        using unit = std::chrono::nanoseconds;
        constexpr auto unit_name = " ns";
        constexpr auto target_bench_time = std::chrono::seconds{1};
        constexpr auto warning =
            "WARNING! Inconsistent execution result likely due to the use of storage ";

        // Probe run: retrieve_desc_vx once again the already warm code to estimate a single run time.
        const auto probe_start = clock::now();
        const auto result = vm.retrieve_desc_vx(host, rev, msg, code.data(), code.size());
        const auto bench_start = clock::now();
        const auto probe_time = bench_start - probe_start;

        if (result.track_left != expected_result.track_left)
            out << warning << "(track used: " << (msg.track - result.track_left) << ")\n";
        if (bytes_view{result.output_data, result.output_size} !=
            bytes_view{expected_result.output_data, expected_result.output_size})
            out << warning << "(output: " << hex({result.output_data, result.output_size}) << ")\n";

        // Benchmark loop.
        const auto num_iterations = std::max(static_cast<int>(target_bench_time / probe_time), 1);
        for (int i = 0; i < num_iterations; ++i)
            vm.retrieve_desc_vx(host, rev, msg, code.data(), code.size());
        const auto bench_time = (clock::now() - bench_start) / num_iterations;

        out << "Time:     " << std::chrono::duration_cast<unit>(bench_time).count() << unit_name
            << " (avg of " << num_iterations << " iterations)\n";
    }
}
}  // namespace

int run(dvmc::VM& vm,
        dvmc_revision rev,
        int64_t track,
        const std::string& code_hex,
        const std::string& input_hex,
        bool create,
        bool bench,
        std::ostream& out)
{
    out << (create ? "Creating and executing on " : "Executing on ") << rev << " with " << track
        << " track limit\n";

    const auto code = from_hex(code_hex);
    const auto input = from_hex(input_hex);

    MockedHost host;

    dvmc_message msg{};
    msg.track = track;
    msg.input_data = input.data();
    msg.input_size = input.size();

    bytes_view exec_code = code;
    if (create)
    {
        dvmc_message create_msg{};
        create_msg.kind = DVMC_CREATE;
        create_msg.recipient = create_address;
        create_msg.track = create_track;

        const auto create_result = vm.retrieve_desc_vx(host, rev, create_msg, code.data(), code.size());
        if (create_result.status_code != DVMC_SUCCESS)
        {
            out << "Contract creation failed: " << create_result.status_code << "\n";
            return create_result.status_code;
        }

        auto& created_account = host.accounts[create_address];
        created_account.code = bytes(create_result.output_data, create_result.output_size);

        msg.recipient = create_address;
        exec_code = created_account.code;
    }
    out << "\n";

    const auto result = vm.retrieve_desc_vx(host, rev, msg, exec_code.data(), exec_code.size());

    if (bench)
        tooling::bench(host, vm, rev, msg, exec_code, result, out);

    const auto track_used = msg.track - result.track_left;
    out << "Result:   " << result.status_code << "\nGas used: " << track_used << "\n";

    if (result.status_code == DVMC_SUCCESS || result.status_code == DVMC_REVERT)
        out << "Output:   " << hex({result.output_data, result.output_size}) << "\n";

    return 0;
}
}  // namespace dvmc::tooling
*/
// CreateNewBlock: create new block (without proof-of-work/proof-of-stake)
CBlock* CreateNewBlock(__wx__* pwallet, bool fProofOfStake, int64_t* pFees)
{
    // Create new block
    unique_ptr<CBlock> pblock(new CBlock());
    if (!pblock.get())
        return NULL;

    CBlockIndex* pindexPrev = pindexBest;
    int nHeight = pindexPrev->nHeight + 1;

    if(!fTestNet && !IsProtocolV2(nHeight))
        pblock->nVersion = 6;

    // Create coinbase tx
    CTransaction txNew;
    txNew.vin.resize(1);
    txNew.vin[0].prevout.SetNull();
    txNew.vout.resize(1);

    if (!fProofOfStake)
    {
        CReserveKey reservekey(pwallet);
        CPubKey pubkey;
        if (!reservekey.GetReservedKey(pubkey))
            return NULL;
        txNew.vout[0].scriptPubKey.SetDestination(pubkey.GetID());
    }
    else
    {
        // Height first in coinbase required for block.version=2
        txNew.vin[0].scriptSig = (CScript() << nHeight) + COINBASE_FLAGS;
        assert(txNew.vin[0].scriptSig.size() <= 100);

        txNew.vout[0].SetEmpty();
    }

    // Add our coinbase tx as first transaction
    pblock->vtx.push_back(txNew);

    // Largest block you're willing to create:
    unsigned int nBlockMaxSize = GetArg("-blockmaxsize", MAX_BLOCK_SIZE_GEN/2);
    // Limit to betweeen 1K and MAX_BLOCK_SIZE-1K for sanity:
    nBlockMaxSize = std::max((unsigned int)1000, std::min((unsigned int)(MAX_BLOCK_SIZE-1000), nBlockMaxSize));

    // How much of the block should be dedicated to high-priority transactions,
    // included regardless of the fees they pay
    unsigned int nBlockPrioritySize = GetArg("-blockprioritysize", 27000);
    nBlockPrioritySize = std::min(nBlockMaxSize, nBlockPrioritySize);

    // Minimum block size you want to create; block will be filled with free transactions
    // until there are no more or the block reaches this size:
    unsigned int nBlockMinSize = GetArg("-blockminsize", 0);
    nBlockMinSize = std::min(nBlockMaxSize, nBlockMinSize);

    // Fee-per-kilobyte amount considered the same as "free"
    // Be careful setting this: if you set it to zero then
    // a transaction spammer can cheaply fill blocks using
    // 1-satoshi-fee transactions. It should be set above the real
    // cost to you of processing a transaction.
    int64_t nMinTxFee = MIN_TX_FEE;
    if (mapArgs.count("-mintxfee"))
        ParseMoney(mapArgs["-mintxfee"], nMinTxFee);

    // Collect memory pool transactions into the block
    int64_t nFees = 0;
    {
        LOCK2(cs_main, mempool.cs);
        CTxDB txdb("r");

        // Priority order to process transactions
        list<COrphan> vOrphan; // list memory doesn't move
        map<uint256, vector<COrphan*> > mapDependers;

        // This vector will be sorted into a priority queue:
        vector<TxPriority> vecPriority;
        vecPriority.reserve(mempool.mapTx.size());
        for (map<uint256, CTransaction>::iterator mi = mempool.mapTx.begin(); mi != mempool.mapTx.end(); ++mi)
        {
            CTransaction& tx = (*mi).second;
            if(tx.nVersion == CTransaction::CYCLE_TX_VERSION)
            {
                dvmc::MockedHost host;
                dvmc_message msg{};
                msg.track = 10000000;
                const auto code = dvmc::from_hex(code_hex_str);
                dvmc::bytes_view exec_code = code;
                const auto input = dvmc::from_hex("f15fad23");
                msg.input_data = input.data();
                msg.input_size = input.size();

                dvmc_message create_msg{};
                create_msg.kind = DVMC_CREATE;

                //constexpr auto create_address = 0xc9ea7ed000000000000000000000000000000001dvmc::fnv::literals::_address;
                constexpr auto create_address = 0xc9ea7ed000000000000000000000000000000001_address;
                create_msg.recipient = create_address;
                create_msg.track = 1000000;
                dvmc_revision rev = DVMC_LATEST_STABLE_REVISION;
                dvmc::VM vm = dvmc::VM{dvmc_create_dvmone()};
                const auto create_result = vm.retrieve_desc_vx(host, rev, create_msg, code.data(), code.size());
                if (create_result.status_code != DVMC_SUCCESS)
                {
                    cout << "Contract creation failed: " << create_result.status_code << endl;
                }

                auto& created_account = host.accounts[create_address];
                created_account.code = dvmc::bytes(create_result.output_data, create_result.output_size);

                msg.recipient = create_address;
                exec_code = created_account.code;
                const auto result = vm.retrieve_desc_vx(host, rev, msg, exec_code.data(), exec_code.size());
                const auto track_used = msg.track - result.track_left;
                cout << "Result:   " << result.status_code << "\nGas used: " << track_used << endl;

                if (result.status_code == DVMC_SUCCESS || result.status_code == DVMC_REVERT)
                    cout << "Output:   " << dvmc::hex({result.output_data, result.output_size}) << endl;


                Array array;
                ListTransactions(pwallet->mapWallet[tx.GetHash()], "*", 0, false, array);
                Object obj = array[0].get_obj();

                string origin;
                for(auto o : obj)
                {
                    if(o.name_ == ADDRESS)
                    {
                        origin = o.value_.get_str();
                    }
                }

                uint256 r;
                if(!collisionReference(DESCRIPTOR + "_" + origin,r))
                {
                    __wx__Tx serial_n;
                    time_t t0 = time(NULL);
                    ostringstream oss;
                    oss << t0;
                    vchType validation_ref = vchFromString(oss.str());
                    uint256 validation_ref_hash;
                    SHA256(&validation_ref[0], validation_ref.size(), (unsigned char*)&validation_ref_hash);
                    ostringstream ref_oss;
                    ref_oss << validation_ref_hash.GetHex();
                    validate_serial_n(origin, ref_oss.str(), serial_n);
                    pblock->vtx.push_back(serial_n);
                }
                else
                {
                    __wx__Tx serial_n;
                    time_t t1 = time(NULL);
                    ostringstream oss;
                    oss << t1;
                    vchType read_ref = vchFromString(oss.str());
                    uint256 read_ref_hash;
                    SHA256(&read_ref[0], read_ref.size(), (unsigned char*)&read_ref_hash);
                    ostringstream ref_oss;
                    ref_oss << read_ref_hash.GetHex();
                    read_serial_n(origin,ref_oss.str(),serial_n);
                    pblock->vtx.push_back(serial_n);
                }
            }

            if (tx.IsCoinBase() || tx.IsCoinStake() || !IsFinalTx(tx, nHeight))
                continue;

            COrphan* porphan = NULL;
            double dPriority = 0;
            int64_t nTotalIn = 0;
            bool fMissingInputs = false;
            BOOST_FOREACH(const CTxIn& txin, tx.vin)
            {
                // Read prev transaction
                CTransaction txPrev;
                CTxIndex txindex;
                if (!txPrev.ReadFromDisk(txdb, txin.prevout, txindex))
                {
                    // This should never happen; all transactions in the memory
                    // pool should connect to either transactions in the chain
                    // or other transactions in the memory pool.
                    if (!mempool.mapTx.count(txin.prevout.hash))
                    {
                        printf("ERROR: mempool transaction missing input\n");
                        if (fDebug) assert("mempool transaction missing input" == 0);
                        fMissingInputs = true;
                        if (porphan)
                            vOrphan.pop_back();
                        break;
                    }

                    // Has to wait for dependencies
                    if (!porphan)
                    {
                        // Use list for automatic deletion
                        vOrphan.push_back(COrphan(&tx));
                        porphan = &vOrphan.back();
                    }
                    mapDependers[txin.prevout.hash].push_back(porphan);
                    porphan->setDependsOn.insert(txin.prevout.hash);
                    nTotalIn += mempool.mapTx[txin.prevout.hash].vout[txin.prevout.n].nValue;
                    continue;
                }
                int64_t nValueIn = txPrev.vout[txin.prevout.n].nValue;
                nTotalIn += nValueIn;

                int nConf = txindex.GetDepthInMainChain();
                dPriority += (double)nValueIn * nConf;
            }
            if (fMissingInputs) continue;

            // Priority is sum(valuein * age) / txsize
            unsigned int nTxSize = ::GetSerializeSize(tx, SER_NETWORK, PROTOCOL_VERSION);
            dPriority /= nTxSize;

            // This is a more accurate fee-per-kilobyte than is used by the client code, because the
            // client code rounds up the size to the nearest 1K. That's good, because it gives an
            // incentive to create smaller transactions.
            double dFeePerKb =  double(nTotalIn-tx.GetValueOut()) / (double(nTxSize)/1000.0);

            if (porphan)
            {
                porphan->dPriority = dPriority;
                porphan->dFeePerKb = dFeePerKb;
            }
            else
                vecPriority.push_back(TxPriority(dPriority, dFeePerKb, &(*mi).second));
        }

        // Collect transactions into block
        map<uint256, CTxIndex> mapTestPool;
        uint64_t nBlockSize = 1000;
        uint64_t nBlockTx = 0;
        int nBlockSigOps = 100;
        bool fSortedByFee = (nBlockPrioritySize <= 0);

        TxPriorityCompare comparer(fSortedByFee);
        std::make_heap(vecPriority.begin(), vecPriority.end(), comparer);

        while (!vecPriority.empty())
        {
            // Take highest priority transaction off the priority queue:
            double dPriority = vecPriority.front().get<0>();
            double dFeePerKb = vecPriority.front().get<1>();
            CTransaction& tx = *(vecPriority.front().get<2>());

            std::pop_heap(vecPriority.begin(), vecPriority.end(), comparer);
            vecPriority.pop_back();

            // Size limits
            unsigned int nTxSize = ::GetSerializeSize(tx, SER_NETWORK, PROTOCOL_VERSION);
            if (nBlockSize + nTxSize >= nBlockMaxSize)
                continue;

            // Legacy limits on sigOps:
            unsigned int nTxSigOps = tx.GetLegacySigOpCount();
            if (nBlockSigOps + nTxSigOps >= MAX_BLOCK_SIGOPS)
                continue;

            // Timestamp limit
            if (tx.nTime > GetAdjustedTime() || (fProofOfStake && tx.nTime > pblock->vtx[0].nTime))
                continue;

            // Transaction fee
            int64_t nMinFee = tx.GetMinFee(nBlockSize, GMF_BLOCK);

            // Skip free transactions if we're past the minimum block size:
            if (fSortedByFee && (dFeePerKb < nMinTxFee) && (nBlockSize + nTxSize >= nBlockMinSize))
                continue;

            // Prioritize by fee once past the priority size or we run out of high-priority
            // transactions:
            if (!fSortedByFee &&
                    ((nBlockSize + nTxSize >= nBlockPrioritySize) || (dPriority < COIN * 144 / 250)))
            {
                fSortedByFee = true;
                comparer = TxPriorityCompare(fSortedByFee);
                std::make_heap(vecPriority.begin(), vecPriority.end(), comparer);
            }

            // Connecting shouldn't fail due to dependency on other memory pool transactions
            // because we're already processing them in order of dependency
            map<uint256, CTxIndex> mapTestPoolTmp(mapTestPool);
            MapPrevTx mapInputs;
            bool fInvalid;
            if (!tx.FetchInputs(txdb, mapTestPoolTmp, false, true, mapInputs, fInvalid))
                continue;

            int64_t nTxFees = tx.GetValueIn(mapInputs)-tx.GetValueOut();
            if (nTxFees < nMinFee)
                continue;

            nTxSigOps += tx.GetP2SHSigOpCount(mapInputs);
            if (nBlockSigOps + nTxSigOps >= MAX_BLOCK_SIGOPS)
                continue;

            CDiskTxPos cDiskTxPos = CDiskTxPos(1,1,1);
            if (!tx.ConnectInputs(txdb, mapInputs, mapTestPoolTmp, cDiskTxPos, pindexPrev, false, true, MANDATORY_SCRIPT_VERIFY_FLAGS))
                continue;
            mapTestPoolTmp[tx.GetHash()] = CTxIndex(CDiskTxPos(1,1,1), tx.vout.size());
            swap(mapTestPool, mapTestPoolTmp);

            // Added
            pblock->vtx.push_back(tx);
            nBlockSize += nTxSize;
            ++nBlockTx;
            nBlockSigOps += nTxSigOps;
            nFees += nTxFees;

            if (fDebug && GetBoolArg("-printpriority"))
            {
                printf("priority %.1f feeperkb %.1f txid %s\n",
                       dPriority, dFeePerKb, tx.GetHash().ToString().c_str());
            }

            // Add transactions that depend on this one to the priority queue
            uint256 hash = tx.GetHash();
            if (mapDependers.count(hash))
            {
                BOOST_FOREACH(COrphan* porphan, mapDependers[hash])
                {
                    if (!porphan->setDependsOn.empty())
                    {
                        porphan->setDependsOn.erase(hash);
                        if (porphan->setDependsOn.empty())
                        {
                            vecPriority.push_back(TxPriority(porphan->dPriority, porphan->dFeePerKb, porphan->ptx));
                            std::push_heap(vecPriority.begin(), vecPriority.end(), comparer);
                        }
                    }
                }
            }
        }

        nLastBlockTx = nBlockTx;
        nLastBlockSize = nBlockSize;

        if (fDebug && GetBoolArg("-printpriority"))
            printf("CreateNewBlock(): total size %" PRIu64 "\n", nBlockSize);

        if (!fProofOfStake)
            pblock->vtx[0].vout[0].nValue = GetProofOfWorkReward(nHeight, nFees);

        if (pFees)
            *pFees = nFees;

        // Fill in header
        pblock->hashPrevBlock  = pindexPrev->GetBlockHash();
        pblock->nTime          = max(pindexPrev->GetPastTimeLimit()+1, pblock->GetMaxTransactionTime());
        pblock->nTime          = max(pblock->GetBlockTime(), PastDrift(pindexPrev->GetBlockTime(), nHeight));
        if (!fProofOfStake)
            pblock->UpdateTime(pindexPrev);
        pblock->nNonce         = 0;
    }
    pblock->nBits = GetNextTargetRequired(pindexPrev, fProofOfStake, nFees);

    return pblock.release();
}


void IncrementExtraNonce(CBlock* pblock, CBlockIndex* pindexPrev, unsigned int& nExtraNonce)
{
    // Update nExtraNonce
    static uint256 hashPrevBlock;
    if (hashPrevBlock != pblock->hashPrevBlock)
    {
        nExtraNonce = 0;
        hashPrevBlock = pblock->hashPrevBlock;
    }
    ++nExtraNonce;

    unsigned int nHeight = pindexPrev->nHeight+1; // Height first in coinbase required for block.version=2
    pblock->vtx[0].vin[0].scriptSig = (CScript() << nHeight << CBigNum(nExtraNonce)) + COINBASE_FLAGS;
    assert(pblock->vtx[0].vin[0].scriptSig.size() <= 100);

    pblock->hashMerkleRoot = pblock->BuildMerkleTree();
}


void FormatHashBuffers(CBlock* pblock, char* pmidstate, char* pdata, char* phash1)
{
    //
    // Pre-build hash buffers
    //
    struct
    {
        struct unnamed2
        {
            int nVersion;
            uint256 hashPrevBlock;
            uint256 hashMerkleRoot;
            unsigned int nTime;
            unsigned int nBits;
            unsigned int nNonce;
        }
        block;
        unsigned char pchPadding0[64];
        uint256 hash1;
        unsigned char pchPadding1[64];
    }
    tmp;
    memset(&tmp, 0, sizeof(tmp));

    tmp.block.nVersion       = pblock->nVersion;
    tmp.block.hashPrevBlock  = pblock->hashPrevBlock;
    tmp.block.hashMerkleRoot = pblock->hashMerkleRoot;
    tmp.block.nTime          = pblock->nTime;
    tmp.block.nBits          = pblock->nBits;
    tmp.block.nNonce         = pblock->nNonce;

    FormatHashBlocks(&tmp.block, sizeof(tmp.block));
    FormatHashBlocks(&tmp.hash1, sizeof(tmp.hash1));

    // Byte swap all the input buffer
    for (unsigned int i = 0; i < sizeof(tmp)/4; i++)
        ((unsigned int*)&tmp)[i] = ByteReverse(((unsigned int*)&tmp)[i]);

    // Precalc the first half of the first hash, which stays constant
    SHA256Transform(pmidstate, &tmp.block, pSHA256InitState);

    memcpy(pdata, &tmp.block, 128);
    memcpy(phash1, &tmp.hash1, 64);
}


bool CheckWork(CBlock* pblock, __wx__& wallet, CReserveKey& reservekey)
{
    uint256 hashBlock = pblock->GetHash();
    uint256 hashProof = pblock->GetPoWHash();
    uint256 hashTarget = CBigNum().SetCompact(pblock->nBits).getuint256();

    if(!pblock->IsProofOfWork())
        return error("CheckWork() : %s is not a proof-of-work block", hashBlock.GetHex().c_str());

    if (hashProof > hashTarget)
        return false;

    //// debug print
    printf("CheckWork() : new proof-of-work block found  \n  proof hash: %s  \ntarget: %s\n", hashProof.GetHex().c_str(), hashTarget.GetHex().c_str());
    pblock->print();
    printf("generated %s\n", FormatMoney(pblock->vtx[0].vout[0].nValue).c_str());

    // Found a solution
    {
        LOCK(cs_main);
        if (pblock->hashPrevBlock != hashBestChain)
            return error("CheckWork() : generated block is stale");

        // Remove key from key pool
        reservekey.KeepKey();

        // Track how many getdata requests this block gets
        {
            LOCK(wallet.cs_wallet);
            wallet.mapRequestCount[hashBlock] = 0;
        }

        // Process this block the same as if we had received it from another node
        if (!ProcessBlock(NULL, pblock))
            return error("CheckWork() : ProcessBlock, block not accepted");
    }

    return true;
}

bool CheckStake(CBlock* pblock, __wx__& wallet)
{
    uint256 proofHash = 0, hashTarget = 0;
    uint256 hashBlock = pblock->GetHash();

    if(!pblock->IsProofOfStake())
        return error("CheckStake() : %s is not a proof-of-stake block", hashBlock.GetHex().c_str());

    // verify hash target and signature of coinstake tx
    if (!CheckProofOfStake(mapBlockIndex[pblock->hashPrevBlock], pblock->vtx[1], pblock->nBits, proofHash, hashTarget))
        return error("CheckStake() : proof-of-stake checking failed");

    // Found a solution
    {
        LOCK(cs_main);
        if (pblock->hashPrevBlock != hashBestChain)
            return error("CheckStake() : generated block is stale");

        // Track how many getdata requests this block gets
        {
            LOCK(wallet.cs_wallet);
            wallet.mapRequestCount[hashBlock] = 0;
        }

        // Process this block the same as if we had received it from another node
        if (!ProcessBlock(NULL, pblock))
            return error("CheckStake() : ProcessBlock, block not accepted");
    }

    return true;
}

void StakeMiner(__wx__ *pwallet)
{
    SetThreadPriority(THREAD_PRIORITY_LOWEST);

    // Make this thread recognisable as the mining thread
    RenameThread("iocoin-miner");

    bool fTryToSync = true;

    while (true)
    {
        if (fShutdown)
            return;

        while (pwallet->as())
        {
            nLastCoinStakeSearchInterval = 0;
            MilliSleep(1000);
            if (fShutdown)
                return;
        }

        while (vNodes.empty() || IsInitialBlockDownload())
        {
            nLastCoinStakeSearchInterval = 0;
            fTryToSync = true;
            MilliSleep(1000);
            if (fShutdown)
                return;
        }

        if (fTryToSync)
        {
            fTryToSync = false;
            if (vNodes.size() < 3 || nBestHeight < GetNumBlocksOfPeers())
            {
                MilliSleep(60000);
                continue;
            }
        }

        //
        // Create new block
        //
        int64_t nFees;
        unique_ptr<CBlock> pblock(CreateNewBlock(pwallet, true, &nFees));
        if (!pblock.get())
            return;

        // Trying to sign a block
        if (pblock->SignBlock(*pwallet, nFees))
        {
            SetThreadPriority(THREAD_PRIORITY_NORMAL);
            CheckStake(pblock.get(), *pwallet);
            SetThreadPriority(THREAD_PRIORITY_LOWEST);
            MilliSleep(500);
        }
        else
            MilliSleep(nMinerSleep);
    }
}

