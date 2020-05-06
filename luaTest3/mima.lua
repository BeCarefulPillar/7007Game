local Array = require("lockbox.util.array")
local Stream = require("lockbox.util.stream")
local ECBMode = require("lockbox.cipher.mode.ecb")
local PKCS7Padding = require("lockbox.padding.pkcs7")
local DESCipher = require("lockbox.cipher.des3")
local Base64 = require("lockbox.util.base64")
local Md5 = require("lockbox.digest.md5")

local bytes = Md5().init().update(Stream.fromArray((Array.fromString("ROD1KbAF6et9G4K3yADT6gC3A8")))).finish().asBytes()
local cipher = ECBMode.Cipher().setKey(bytes).setBlockCipher(DESCipher).setPadding(PKCS7Padding)
local res = cipher.init().update(Stream.fromArray(Array.fromString('223'))).finish().asBytes()
local out = Base64.fromArray(res)
print(out)


local SHA2_256 = require("lockbox.digest.sha2_256")
local HMAC = require("lockbox.mac.hmac")
local out = HMAC().setDigest(SHA2_256).setKey(Array.fromString("123")).init().update(Stream.fromArray((Array.fromString("123")))).finish().asHex()
