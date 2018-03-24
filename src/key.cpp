// Copyright (c) 2009-2012 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <map>

#include <openssl/ecdsa.h>
#include <openssl/obj_mac.h>

#include "key.h"


struct __fbase__
{
  BIGNUM* _g1;
  EC_POINT* _g0;
  EC_POINT* _kvtx;
  EC_POINT* _o1;
  EC_POINT* _k;
  BIGNUM* _s;
  BIGNUM* _q1;
  BIGNUM* _l1;
  BIGNUM* _q;
  BIGNUM* _t;
  BN_CTX* ctx;
};

void __fbase__x(__fbase__* x)
{
  BN_free(x->_g1);
  EC_POINT_free(x->_g0);
  EC_POINT_free(x->_kvtx);
  EC_POINT_free(x->_o1);
  EC_POINT_free(x->_k);
  BN_free(x->_s);
  BN_free(x->_q1);
  BN_free(x->_l1);
  BN_free(x->_q);
  BN_free(x->_t);
  BN_CTX_free(x->ctx);
}

void __vtx_clean(__fbase__* x, EC_GROUP* g)
{
  __fbase__x(x);
  EC_GROUP_free(g);
}

struct __convol__77
{
  BN_CTX* ctx;
  BIGNUM* q1;
  EC_POINT* q2;
  BIGNUM* q3;
  EC_POINT* q4;
  EC_POINT* q5;
  BIGNUM* q6;
};

void __convol_x__(__convol__77* c)
{
  BN_CTX_free(c->ctx);
  BN_free(c->q1);
  EC_POINT_free(c->q2);
  BN_free(c->q3);
  EC_POINT_free(c->q4);
  EC_POINT_free(c->q5);
  BN_free(c->q6);
}

struct __conv__intern__
{
  BN_CTX* ctx;
  BIGNUM* s1;
  BIGNUM* s2;
  EC_POINT* s3;
  BIGNUM* s4;
  BIGNUM* s5;
  BIGNUM* s6;
  BIGNUM* s7;
};

void __conv_intern__x__(__conv__intern__* c)
{
  BN_CTX_free(c->ctx);
  BN_free(c->s1);
  BN_free(c->s2);
  EC_POINT_free(c->s3);
  BN_free(c->s4);
  BN_free(c->s5);
  BN_free(c->s6);
  BN_free(c->s7);
}

// Generate a private key from just the secret parameter
int EC_KEY_regenerate_key(EC_KEY *eckey, BIGNUM *priv_key)
{
    int ok = 0;
    BN_CTX *ctx = NULL;
    EC_POINT *pub_key = NULL;

    if (!eckey) return 0;

    const EC_GROUP *group = EC_KEY_get0_group(eckey);

    if ((ctx = BN_CTX_new()) == NULL)
        goto err;

    pub_key = EC_POINT_new(group);

    if (pub_key == NULL)
        goto err;

    if (!EC_POINT_mul(group, pub_key, priv_key, NULL, NULL, ctx))
        goto err;

    EC_KEY_set_private_key(eckey,priv_key);
    EC_KEY_set_public_key(eckey,pub_key);

    ok = 1;

err:

    if (pub_key)
        EC_POINT_free(pub_key);
    if (ctx != NULL)
        BN_CTX_free(ctx);

    return(ok);
}

// Perform ECDSA key recovery (see SEC1 4.1.6) for curves over (mod p)-fields
// recid selects which key is recovered
// if check is non-zero, additional checks are performed
int ECDSA_SIG_recover_key_GFp(EC_KEY *eckey, ECDSA_SIG *ecsig, const unsigned char *msg, int msglen, int recid, int check)
{
    if (!eckey) return 0;

    int ret = 0;
    BN_CTX *ctx = NULL;

    BIGNUM *x = NULL;
    BIGNUM *e = NULL;
    BIGNUM *order = NULL;
    BIGNUM *sor = NULL;
    BIGNUM *eor = NULL;
    BIGNUM *field = NULL;
    EC_POINT *R = NULL;
    EC_POINT *O = NULL;
    EC_POINT *Q = NULL;
    BIGNUM *rr = NULL;
    BIGNUM *zero = NULL;
    int n = 0;
    int i = recid / 2;

    const EC_GROUP *group = EC_KEY_get0_group(eckey);
    if ((ctx = BN_CTX_new()) == NULL) { ret = -1; goto err; }
    BN_CTX_start(ctx);
    order = BN_CTX_get(ctx);
    if (!EC_GROUP_get_order(group, order, ctx)) { ret = -2; goto err; }
    x = BN_CTX_get(ctx);
    if (!BN_copy(x, order)) { ret=-1; goto err; }
    if (!BN_mul_word(x, i)) { ret=-1; goto err; }
    if (!BN_add(x, x, ecsig->r)) { ret=-1; goto err; }
    field = BN_CTX_get(ctx);
    if (!EC_GROUP_get_curve_GFp(group, field, NULL, NULL, ctx)) { ret=-2; goto err; }
    if (BN_cmp(x, field) >= 0) { ret=0; goto err; }
    if ((R = EC_POINT_new(group)) == NULL) { ret = -2; goto err; }
    if (!EC_POINT_set_compressed_coordinates_GFp(group, R, x, recid % 2, ctx)) { ret=0; goto err; }
    if (check)
    {
        if ((O = EC_POINT_new(group)) == NULL) { ret = -2; goto err; }
        if (!EC_POINT_mul(group, O, NULL, R, order, ctx)) { ret=-2; goto err; }
        if (!EC_POINT_is_at_infinity(group, O)) { ret = 0; goto err; }
    }
    if ((Q = EC_POINT_new(group)) == NULL) { ret = -2; goto err; }
    n = EC_GROUP_get_degree(group);
    e = BN_CTX_get(ctx);
    if (!BN_bin2bn(msg, msglen, e)) { ret=-1; goto err; }
    if (8*msglen > n) BN_rshift(e, e, 8-(n & 7));
    zero = BN_CTX_get(ctx);
    if (!BN_zero(zero)) { ret=-1; goto err; }
    if (!BN_mod_sub(e, zero, e, order, ctx)) { ret=-1; goto err; }
    rr = BN_CTX_get(ctx);
    if (!BN_mod_inverse(rr, ecsig->r, order, ctx)) { ret=-1; goto err; }
    sor = BN_CTX_get(ctx);
    if (!BN_mod_mul(sor, ecsig->s, rr, order, ctx)) { ret=-1; goto err; }
    eor = BN_CTX_get(ctx);
    if (!BN_mod_mul(eor, e, rr, order, ctx)) { ret=-1; goto err; }
    if (!EC_POINT_mul(group, Q, eor, R, sor, ctx)) { ret=-2; goto err; }
    if (!EC_KEY_set_public_key(eckey, Q)) { ret=-2; goto err; }

    ret = 1;

err:
    if (ctx) {
        BN_CTX_end(ctx);
        BN_CTX_free(ctx);
    }
    if (R != NULL) EC_POINT_free(R);
    if (O != NULL) EC_POINT_free(O);
    if (Q != NULL) EC_POINT_free(Q);
    return ret;
}

void CKey::SetCompressedPubKey()
{
    EC_KEY_set_conv_form(pkey, POINT_CONVERSION_COMPRESSED);
    fCompressedPubKey = true;
}

void CKey::Reset()
{
    fCompressedPubKey = false;
    if (pkey != NULL)
        EC_KEY_free(pkey);
    pkey = EC_KEY_new_by_curve_name(NID_secp256k1);
    if (pkey == NULL)
        throw key_error("CKey::CKey() : EC_KEY_new_by_curve_name failed");
    fSet = false;
}

CKey::CKey()
{
    pkey = NULL;
    Reset();
}

CKey::CKey(const CKey& b)
{
    pkey = EC_KEY_dup(b.pkey);
    if (pkey == NULL)
        throw key_error("CKey::CKey(const CKey&) : EC_KEY_dup failed");
    fSet = b.fSet;
}

CKey& CKey::operator=(const CKey& b)
{
    if (!EC_KEY_copy(pkey, b.pkey))
        throw key_error("CKey::operator=(const CKey&) : EC_KEY_copy failed");
    fSet = b.fSet;
    return (*this);
}

CKey::~CKey()
{
    EC_KEY_free(pkey);
}

bool CKey::IsNull() const
{
    return !fSet;
}

bool CKey::IsCompressed() const
{
    return fCompressedPubKey;
}

int CompareBigEndian(const unsigned char *c1, size_t c1len, const unsigned char *c2, size_t c2len) {
    while (c1len > c2len) {
        if (*c1)
            return 1;
        c1++;
        c1len--;
    }
    while (c2len > c1len) {
        if (*c2)
            return -1;
        c2++;
        c2len--;
    }
    while (c1len > 0) {
        if (*c1 > *c2)
            return 1;
        if (*c2 > *c1)
            return -1;
        c1++;
        c2++;
        c1len--;
    }
    return 0;
}

// Order of secp256k1's generator minus 1.
const unsigned char vchMaxModOrder[32] = {
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFE,
    0xBA,0xAE,0xDC,0xE6,0xAF,0x48,0xA0,0x3B,
    0xBF,0xD2,0x5E,0x8C,0xD0,0x36,0x41,0x40
};

// Half of the order of secp256k1's generator minus 1.
const unsigned char vchMaxModHalfOrder[32] = {
    0x7F,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0x5D,0x57,0x6E,0x73,0x57,0xA4,0x50,0x1D,
    0xDF,0xE9,0x2F,0x46,0x68,0x1B,0x20,0xA0
};

const unsigned char vchZero[0] = {};

bool CKey::CheckSignatureElement(const unsigned char *vch, int len, bool half) {
    return CompareBigEndian(vch, len, vchZero, 0) > 0 &&
           CompareBigEndian(vch, len, half ? vchMaxModHalfOrder : vchMaxModOrder, 32) <= 0;
}

void CKey::MakeNewKey(bool fCompressed)
{
    if (!EC_KEY_generate_key(pkey))
        throw key_error("CKey::MakeNewKey() : EC_KEY_generate_key failed");
    if (fCompressed)
        SetCompressedPubKey();
    fSet = true;
}

bool CKey::SetPrivKey(const CPrivKey& vchPrivKey)
{
    const unsigned char* pbegin = &vchPrivKey[0];
    if (d2i_ECPrivateKey(&pkey, &pbegin, vchPrivKey.size()))
    {
        // In testing, d2i_ECPrivateKey can return true
        // but fill in pkey with a key that fails
        // EC_KEY_check_key, so:
        if (EC_KEY_check_key(pkey))
        {
            fSet = true;
            return true;
        }
    }
    // If vchPrivKey data is bad d2i_ECPrivateKey() can
    // leave pkey in a state where calling EC_KEY_free()
    // crashes. To avoid that, set pkey to NULL and
    // leak the memory (a leak is better than a crash)
    pkey = NULL;
    Reset();
    return false;
}

bool CKey::SetSecret(const CSecret& vchSecret, bool fCompressed)
{
    EC_KEY_free(pkey);
    pkey = EC_KEY_new_by_curve_name(NID_secp256k1);
    if (pkey == NULL)
        throw key_error("CKey::SetSecret() : EC_KEY_new_by_curve_name failed");
    if (vchSecret.size() != 32)
        throw key_error("CKey::SetSecret() : secret must be 32 bytes");
    BIGNUM *bn = BN_bin2bn(&vchSecret[0],32,BN_new());
    if (bn == NULL)
        throw key_error("CKey::SetSecret() : BN_bin2bn failed");
    if (!EC_KEY_regenerate_key(pkey,bn))
    {
        BN_clear_free(bn);
        throw key_error("CKey::SetSecret() : EC_KEY_regenerate_key failed");
    }
    BN_clear_free(bn);
    fSet = true;
    if (fCompressed || fCompressedPubKey)
        SetCompressedPubKey();
    return true;
}

CSecret CKey::GetSecret(bool &fCompressed) const
{
    CSecret vchRet;
    vchRet.resize(32);
    const BIGNUM *bn = EC_KEY_get0_private_key(pkey);
    int nBytes = BN_num_bytes(bn);
    if (bn == NULL)
        throw key_error("CKey::GetSecret() : EC_KEY_get0_private_key failed");
    int n=BN_bn2bin(bn,&vchRet[32 - nBytes]);
    if (n != nBytes)
        throw key_error("CKey::GetSecret(): BN_bn2bin failed");
    fCompressed = fCompressedPubKey;
    return vchRet;
}

CPrivKey CKey::GetPrivKey() const
{
    int nSize = i2d_ECPrivateKey(pkey, NULL);
    if (!nSize)
        throw key_error("CKey::GetPrivKey() : i2d_ECPrivateKey failed");
    CPrivKey vchPrivKey(nSize, 0);
    unsigned char* pbegin = &vchPrivKey[0];
    if (i2d_ECPrivateKey(pkey, &pbegin) != nSize)
        throw key_error("CKey::GetPrivKey() : i2d_ECPrivateKey returned unexpected size");
    return vchPrivKey;
}

bool CKey::SetPubKey(const CPubKey& vchPubKey)
{
    const unsigned char* pbegin = &vchPubKey.vchPubKey[0];
    if (o2i_ECPublicKey(&pkey, &pbegin, vchPubKey.vchPubKey.size()))
    {
        fSet = true;
        if (vchPubKey.vchPubKey.size() == 33)
            SetCompressedPubKey();
        return true;
    }
    pkey = NULL;
    Reset();
    return false;
}

CPubKey CKey::GetPubKey() const
{
    int nSize = i2o_ECPublicKey(pkey, NULL);
    if (!nSize)
        throw key_error("CKey::GetPubKey() : i2o_ECPublicKey failed");
    std::vector<unsigned char> vchPubKey(nSize, 0);
    unsigned char* pbegin = &vchPubKey[0];
    if (i2o_ECPublicKey(pkey, &pbegin) != nSize)
        throw key_error("CKey::GetPubKey() : i2o_ECPublicKey returned unexpected size");
    return CPubKey(vchPubKey);
}

bool CKey::Sign(uint256 hash, std::vector<unsigned char>& vchSig)
{
    vchSig.clear();


    ECDSA_SIG *sig = ECDSA_do_sign((unsigned char*)&hash, sizeof(hash), pkey);
    if (sig == NULL)
        return false;
    BN_CTX *ctx = BN_CTX_new();
    BN_CTX_start(ctx);
    const EC_GROUP *group = EC_KEY_get0_group(pkey);
    BIGNUM *order = BN_CTX_get(ctx);
    BIGNUM *halforder = BN_CTX_get(ctx);
    EC_GROUP_get_order(group, order, ctx);
    BN_rshift1(halforder, order);
    if (BN_cmp(sig->s, halforder) > 0) {
        // enforce low S values, by negating the value (modulo the order) if above order/2.
        BN_sub(sig->s, order, sig->s);
    }
    BN_CTX_end(ctx);
    BN_CTX_free(ctx);
    unsigned int nSize = ECDSA_size(pkey);
    vchSig.resize(nSize); // Make sure it is big enough
    unsigned char *pos = &vchSig[0];
    nSize = i2d_ECDSA_SIG(sig, &pos);
    ECDSA_SIG_free(sig);
    vchSig.resize(nSize); // Shrink to fit actual size

    return true;
}

// create a compact signature (65 bytes), which allows reconstructing the used public key
// The format is one header byte, followed by two times 32 bytes for the serialized r and s values.
// The header byte: 0x1B = first key with even y, 0x1C = first key with odd y,
//                  0x1D = second key with even y, 0x1E = second key with odd y
bool CKey::SignCompact(uint256 hash, std::vector<unsigned char>& vchSig)
{
    bool fOk = false;
    ECDSA_SIG *sig = ECDSA_do_sign((unsigned char*)&hash, sizeof(hash), pkey);
    if (sig==NULL)
        return false;
    vchSig.clear();
    vchSig.resize(65,0);
    int nBitsR = BN_num_bits(sig->r);
    int nBitsS = BN_num_bits(sig->s);
    if (nBitsR <= 256 && nBitsS <= 256)
    {
        int nRecId = -1;
        for (int i=0; i<4; i++)
        {
            CKey keyRec;
            keyRec.fSet = true;
            if (fCompressedPubKey)
                keyRec.SetCompressedPubKey();
            if (ECDSA_SIG_recover_key_GFp(keyRec.pkey, sig, (unsigned char*)&hash, sizeof(hash), i, 1) == 1)
                if (keyRec.GetPubKey() == this->GetPubKey())
                {
                    nRecId = i;
                    break;
                }
        }

        if (nRecId == -1)
        {
            ECDSA_SIG_free(sig);
            throw key_error("CKey::SignCompact() : unable to construct recoverable key");
        }

        vchSig[0] = nRecId+27+(fCompressedPubKey ? 4 : 0);
        BN_bn2bin(sig->r,&vchSig[33-(nBitsR+7)/8]);
        BN_bn2bin(sig->s,&vchSig[65-(nBitsS+7)/8]);
        fOk = true;
    }
    ECDSA_SIG_free(sig);
    return fOk;
}

// reconstruct public key from a compact signature
// This is only slightly more CPU intensive than just verifying it.
// If this function succeeds, the recovered public key is guaranteed to be valid
// (the signature is a valid signature of the given data for that key)
bool CKey::SetCompactSignature(uint256 hash, const std::vector<unsigned char>& vchSig)
{
    if (vchSig.size() != 65)
        return false;
    int nV = vchSig[0];
    if (nV<27 || nV>=35)
        return false;
    ECDSA_SIG *sig = ECDSA_SIG_new();
    BN_bin2bn(&vchSig[1],32,sig->r);
    BN_bin2bn(&vchSig[33],32,sig->s);

    EC_KEY_free(pkey);
    pkey = EC_KEY_new_by_curve_name(NID_secp256k1);
    if (nV >= 31)
    {
        SetCompressedPubKey();
        nV -= 4;
    }
    if (ECDSA_SIG_recover_key_GFp(pkey, sig, (unsigned char*)&hash, sizeof(hash), nV - 27, 0) == 1)
    {
        fSet = true;
        ECDSA_SIG_free(sig);
        return true;
    }
    ECDSA_SIG_free(sig);
    return false;
}

bool CKey::Verify(uint256 hash, const std::vector<unsigned char>& vchSig)
{
    // -1 = error, 0 = bad sig, 1 = good
    if (ECDSA_verify(0, (unsigned char*)&hash, sizeof(hash), &vchSig[0], vchSig.size(), pkey) != 1)
        return false;

    return true;
}

bool CKey::IsValid()
{
    if (!fSet)
        return false;

    if (!EC_KEY_check_key(pkey))
        return false;

    bool fCompr;
    CSecret secret = GetSecret(fCompr);
    CKey key2;
    key2.SetSecret(secret, fCompr);
    return GetPubKey() == key2.GetPubKey();
}

bool ECC_InitSanityCheck() {
    EC_KEY *pkey = EC_KEY_new_by_curve_name(NID_secp256k1);
    if(pkey == NULL)
        return false;
    EC_KEY_free(pkey);

    // TODO Is there more EC functionality that could be missing?
    return true;
}

int reflection(__pq__& v)
{
  std::vector<unsigned char> vt_;
  
  __fbase__ __fb; 
    
  EC_GROUP* group = EC_GROUP_new_by_curve_name(NID_secp256k1);
    
  if (!group)
  {
    __vtx_clean(&__fb, group);
    return -1;
  }
    
  if (!(__fb.ctx = BN_CTX_new()))
  {
    __vtx_clean(&__fb, group);
    return -1;
  };
    
  if (!(__fb._t = BN_bin2bn(&v.__fq1[0], 0x20, BN_new())))
  {
    __vtx_clean(&__fb, group);
    return -1;
  };
    
  if (!(__fb._s = BN_bin2bn(&v.__fq9[0], v.__fq9.size(), BN_new())))
  {
    __vtx_clean(&__fb, group);
    return -1;
  };
    
  if (!(__fb._k = EC_POINT_bn2point(group, __fb._s, NULL, __fb.ctx)))
  {
    __vtx_clean(&__fb, group);
    return -1;
  };
    
  if (!EC_POINT_mul(group, __fb._k, NULL, __fb._k, __fb._t, __fb.ctx))
  {
    __vtx_clean(&__fb, group);
    return -1;
  };
    
  if (!(__fb._q = EC_POINT_point2bn(group, __fb._k, POINT_CONVERSION_COMPRESSED, BN_new(), __fb.ctx)))
  {
    __vtx_clean(&__fb, group);
    return -1;
  };
    
    
  vt_.resize(0x21);
  if (BN_num_bytes(__fb._q) != 0x21
    || BN_bn2bin(__fb._q, &vt_[0]) != 0x21)
  {
    __vtx_clean(&__fb, group);
    return -1;
  };
    
  SHA256(&vt_[0], vt_.size(), &v.__fq2[0]);
    
  if (!(__fb._l1 = BN_bin2bn(&v.__fq2[0], 0x20, BN_new())))
  {
    __vtx_clean(&__fb, group);
    return -1;
  };
    
  if (!(__fb._o1 = EC_POINT_new(group)))
  {
    __vtx_clean(&__fb, group);
    return -1;
  };
    
  if (!EC_POINT_mul(group, __fb._o1, __fb._l1, NULL, NULL, __fb.ctx))
  {
    __vtx_clean(&__fb, group);
    return -1;
  };
    
  if (!(__fb._q1 = BN_bin2bn(&v.__fq0[0], v.__fq0.size(), BN_new())))
  {
    __vtx_clean(&__fb, group);
    return -1;
  };
    
    
  if (!(__fb._kvtx = EC_POINT_bn2point(group, __fb._q1, NULL, __fb.ctx)))
  {
    __vtx_clean(&__fb, group);
    return -1;
  };
    
  if (!EC_POINT_mul(group, __fb._o1, __fb._l1, NULL, NULL, __fb.ctx))
  {
    __vtx_clean(&__fb, group);
    return -1;
  };
    
  if (!(__fb._g0 = EC_POINT_new(group)))
  {
    __vtx_clean(&__fb, group);
    return -1;
  };
    
  if (!EC_POINT_add(group, __fb._g0, __fb._kvtx, __fb._o1, __fb.ctx))
  {
    __vtx_clean(&__fb, group);
    return -1;
  };
    
  if (!(__fb._g1 = EC_POINT_point2bn(group, __fb._g0, POINT_CONVERSION_COMPRESSED, BN_new(), __fb.ctx)))
  {
    __vtx_clean(&__fb, group);
    return -1;
  };
    
    
  v.__fq5.resize(0x21);
  if (BN_num_bytes(__fb._g1) != 0x21
    || BN_bn2bin(__fb._g1, &v.__fq5[0]) != 0x21)
  {
    __vtx_clean(&__fb, group);
    return -1;
  };
    
  __vtx_clean(&__fb, group);

  return 0;
}

int invert(__inv__& inv)
{
  EC_GROUP* group = EC_GROUP_new_by_curve_name(NID_secp256k1);
  if (!group)
  {
    EC_GROUP_free(group);
    return -1;
  }
   

  BIGNUM* i7 = BN_bin2bn(&inv.__inv7[0], 0x20, BN_new());
  if(!i7)
  {
    BN_free(i7);
    EC_GROUP_free(group);
    return -1;
  }

  EC_POINT* __i1 = EC_POINT_new(group);
  EC_POINT_mul(group, __i1, i7, NULL, NULL, NULL);
  BIGNUM* img = EC_POINT_point2bn(group, __i1, POINT_CONVERSION_COMPRESSED, BN_new(), NULL);

  if(!img || BN_num_bytes(img) != 0x21 || BN_bn2bin(img, &inv.__inv1[0]) != 0x21)
  {
    BN_free(img);
    EC_POINT_free(__i1);
    BN_free(i7);
    EC_GROUP_free(group);
    return -1;
  }  

  BN_free(img);
  EC_POINT_free(__i1);
  BN_free(i7);
  EC_GROUP_free(group);

  return 0;
}

int __synth_piv__conv77(__im__& offset1, __im__& g, __im__& s)
{
  EC_GROUP *group = EC_GROUP_new_by_curve_name(NID_secp256k1);
  if(!group)
    throw runtime_error("conv synth");

  __convol__77 __c7;
  if(!(__c7.ctx = BN_CTX_new()))
  {
    EC_GROUP_free(group);
    __convol_x__(&__c7);
    return -1;
  }

  if(!(__c7.q1 = BN_bin2bn(&g[0], 0x20, BN_new())))  
  {
    EC_GROUP_free(group);
    __convol_x__(&__c7);
    return -1;
  }

  if(!(__c7.q2 = EC_POINT_new(group)))
  {
    EC_GROUP_free(group);
    __convol_x__(&__c7);
    return -1;
  }

  if(!EC_POINT_mul(group, __c7.q2, __c7.q1, NULL, NULL, __c7.ctx))
  {
    EC_GROUP_free(group);
    __convol_x__(&__c7);
    return -1;
  }

  if(!(__c7.q3 = BN_bin2bn(&offset1[0], offset1.size(), BN_new())))
  {
    EC_GROUP_free(group);
    __convol_x__(&__c7);
    return -1;
  }

  if(!(__c7.q4 = EC_POINT_bn2point(group, __c7.q3, NULL, __c7.ctx)))
  {
    EC_GROUP_free(group);
    __convol_x__(&__c7);
    return -1;
  }

  if(!EC_POINT_mul(group, __c7.q2, __c7.q1, NULL, NULL, __c7.ctx))
  {
    EC_GROUP_free(group);
    __convol_x__(&__c7);
    return -1;
  }

  if(!(__c7.q5 = EC_POINT_new(group)))
  {
    EC_GROUP_free(group);
    __convol_x__(&__c7);
    return -1;
  }

  if(!EC_POINT_add(group, __c7.q5, __c7.q4, __c7.q2, __c7.ctx))
  {
    EC_GROUP_free(group);
    __convol_x__(&__c7);
    return -1;
  }

  if(!(__c7.q6 = EC_POINT_point2bn(group, __c7.q5, POINT_CONVERSION_COMPRESSED, BN_new(), __c7.ctx)))
  {
    EC_GROUP_free(group);
    __convol_x__(&__c7);
    return -1;
  }

  s.resize(0x21);
  if(BN_num_bytes(__c7.q6) != 0x21 || BN_bn2bin(__c7.q6, &s[0]) != 0x21)
  {
    EC_GROUP_free(group);
    __convol_x__(&__c7);
    return -1;
  }

  EC_GROUP_free(group);
  __convol_x__(&__c7);

  return 0;
}

int __synth_piv__conv71__intern(__im__& x_intern, __im__& im, 
                                __im__& y_intern, __im__& p_intern)
{
  EC_GROUP* group = EC_GROUP_new_by_curve_name(NID_secp256k1);
  if(!group)
    throw runtime_error("synth conv, group");

  __conv__intern__ conv;
  if(!(conv.ctx = BN_CTX_new()))
  {
    __conv_intern__x__(&conv);
    EC_GROUP_free(group);
    return -1;
  }  

  if(!(conv.s1 = BN_bin2bn(&x_intern[0], 0x20, BN_new())))
  {
    __conv_intern__x__(&conv);
    EC_GROUP_free(group);
    return -1;
  }

  if(!(conv.s2 = BN_bin2bn(&im[0], im.size(), BN_new())))
  {
    __conv_intern__x__(&conv);
    EC_GROUP_free(group);
    return -1;
  }

  if(!(conv.s3 = EC_POINT_bn2point(group, conv.s2, NULL, conv.ctx)))
  {
    __conv_intern__x__(&conv);
    EC_GROUP_free(group);
    return -1;
  }

  if(!EC_POINT_mul(group, conv.s3, NULL, conv.s3, conv.s1, conv.ctx))
  {
    __conv_intern__x__(&conv);
    EC_GROUP_free(group);
    return -1;
  }


  if(!(conv.s4 = EC_POINT_point2bn(group, conv.s3, POINT_CONVERSION_COMPRESSED, BN_new(), conv.ctx)))
  {
    __conv_intern__x__(&conv);
    EC_GROUP_free(group);
    return -1;
  }

  vector<unsigned char> v;
  v.resize(0x21);

  if(BN_num_bytes(conv.s4) != 0x21 || BN_bn2bin(conv.s4, &v[0]) != 0x21)
  {
    __conv_intern__x__(&conv);
    EC_GROUP_free(group);
    return -1;
  }

  vector<unsigned char> v__f;
  v__f.resize(0x20);
  SHA256(&v[0], v.size(), &v__f[0]);

  if(!(conv.s5 = BN_bin2bn(&v__f[0], 0x20, BN_new())))
  {
    __conv_intern__x__(&conv);
    EC_GROUP_free(group);
    return -1;
  }

  if(!(conv.s6 = BN_new()) || !EC_GROUP_get_order(group, conv.s6, conv.ctx))
  {
    __conv_intern__x__(&conv);
    EC_GROUP_free(group);
    return -1;
  }

  if(!(conv.s7 = BN_bin2bn(&y_intern[0], 0x20, BN_new())))
  {
    __conv_intern__x__(&conv);
    EC_GROUP_free(group);
    return -1;
  }

  if(!BN_mod_add(conv.s7, conv.s7, conv.s5, conv.s6, conv.ctx))
  {
    __conv_intern__x__(&conv);
    EC_GROUP_free(group);
    return -1;
  }

  if(BN_is_zero(conv.s7))
  {
    __conv_intern__x__(&conv);
    EC_GROUP_free(group);
    return -1;
  }

  memset(&p_intern[0], 0, 0x20);
  int n_;
  if((n_ = BN_num_bytes(conv.s7)) > 0x20 
    || BN_bn2bin(conv.s7, &p_intern[0x20-n_]) != n_)
  {
    __conv_intern__x__(&conv);
    EC_GROUP_free(group);
    return -1;
  };

  __conv_intern__x__(&conv);
  EC_GROUP_free(group);

  return 0;
}

bool __intersect(CPubKey& i, CPubKey& j)
{
  return true;
}
