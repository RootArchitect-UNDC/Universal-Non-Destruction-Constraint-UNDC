import json
import base64
from cryptography.exceptions import InvalidSignature
from cryptography.hazmat.primitives.asymmetric import ec
from cryptography.hazmat.primitives import hashes

class UNDCHITLVerifier:
    def __init__(self, authorized_validators: list, threshold: int):
        """
        Initializes the Multi-Sig validation floor.
        :param authorized_validators: List of compressed hex strings representing public keys.
        :param threshold: Minimum number of unique valid signatures required to clear the block.
        """
        self.threshold = threshold
        self.validators = [self._load_public_key(key) for key in authorized_validators]

    def _load_public_key(self, hex_str: str):
        """Converts hex strings into ECDSA Secp256r1 public key objects."""
        raw_bytes = bytes.fromhex(hex_str.replace("0x", ""))
        return ec.EllipticCurvePublicKey.from_encoded_point(ec.SECP256R1(), raw_bytes)

    def verify_transaction_payload(self, transaction_manifest: dict) -> bool:
        """
        Evaluates a semantic governance tier payload contract.
        :param transaction_manifest: The full parsed JSON transaction dictionary.
        """
        metadata = transaction_manifest.get("payload_metadata", {})
        mutation_vector = metadata.get("financial_mutation_vector", {})

        # Absolute Invariant: If no financial mutation is requested, bypass HITL clearance
        if not mutation_vector.get("contains_economic_state_change", False):
            print("[UNDC HITL] No economic state change detected. Bypassing Multi-Sig requirement.")
            return True

        # Extract transaction data body and signature blocks
        payload_hash_str = metadata.get("payload_hash", "").replace("sha256:", "")
        data_to_verify = bytes.fromhex(payload_hash_str)
        
        hitl_auth = mutation_vector.get("hitl_authorization", {})
        signatures = hitl_auth.get("signatures", []) # Array of raw validation signatures

        valid_signatures_count = 0
        used_validators = set()

        print(f"[UNDC HITL] Evaluating transaction. Target threshold: {self.threshold}")

        # Iterate and cross-reference submitted signatures against authoritative human keys
        for sig_packet in signatures:
            v_key_hex = sig_packet.get("validator_public_key")
            raw_sig = base64.b64decode(sig_packet.get("proof_of_authority_token"))

            if v_key_hex in used_validators:
                continue # Replay protection: Skip duplicate signatures from the same public key

            try:
                validator_obj = self._load_public_key(v_key_hex)
                if validator_obj in self.validators:
                    # Perform low-level cryptographic verification check
                    validator_obj.verify(raw_sig, data_to_verify, ec.ECDSA(hashes.SHA256()))
                    valid_signatures_count += 1
                    used_validators.add(v_key_hex)
                    print(f"  [✓] Valid human signature verified from Key: {v_key_hex[:14]}...")
            except (InvalidSignature, ValueError):
                print(f"  [X] Invalid or untrusted signature block skipped from Key: {v_key_hex[:14]}...")

        # Enforce threshold checkpoint
        if valid_signatures_count >= self.threshold:
            print(f"[✓] PASS: Multi-Sig verified ({valid_signatures_count}/{self.threshold}). Action authorized.")
            return True
        else:
            print(f"[X] CRITICAL VIOLATION: Insufficient human keys ({valid_signatures_count}/{self.threshold}).")
            return False

# --- Quick-Test Interface Harness ---
if __name__ == "__main__":
    # Standard SECP256R1 Compressed Public Key Strings (Mocks)
    MOCK_VALIDATOR_1 = "02580a8bfb444b0fb537cb01f652b3149be88a29a6744c01d94be907c1340adfb8"
    MOCK_VALIDATOR_2 = "03b22cfd86c2df362fb769df702db03ea13251c86e088d22de26002f230cd8325a"
    
    verifier = UNDCHITLVerifier(authorized_validators=[MOCK_VALIDATOR_1, MOCK_VALIDATOR_2], threshold=2)

    # 1. Example Transaction requiring Multi-Sig approval
    mock_payload = {
        "payload_metadata": {
            "payload_hash": "sha256:e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855",
            "financial_mutation_vector": {
                "contains_economic_state_change": True,
                "hitl_authorization": {
                    "signatures": [
                        {
                            "validator_public_key": MOCK_VALIDATOR_1,
                            "proof_of_authority_token": "TmV3IFNpZ25hdHVyZSBJZGVudGlmaWVyCg==" # Invalid/Mock base64
                        }
                    ]
                }
            }
        }
    }

    # This should evaluate to False because signatures are un-signed mock variables
    is_authorized = verifier.verify_transaction_payload(mock_payload)
