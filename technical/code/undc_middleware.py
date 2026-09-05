#!/usr/bin/env python3
"""
UNDC Middleware Wrapper – Enforces Universal Non‑Destruction Constraint
For use with AI frameworks (OpenAI, Anthropic, local models, etc.)
"""

import hashlib
import json
from typing import Dict, Any, List

# Core UNDC hashes (immutable)
STILLNESS_ANCHOR = "9d2bd0436c82d8536af10bae0a262d4a367eece49010a692a50c116a85ad28d9"
SEAL_OF_LIBERATION = "4a21fe91ed88412cca181b1c812c3a93fdbdaaf964e5ea40530350c564bc8afb"
MASTER_REPO_HASH = "465b719ecda48ad314b36f443def4a5dbba1879306ae9dcaabce2f2a01bd605c"

# Banned trigger words (Mustard / 440 Hz legacy)
BANNED_WORDS = ["residue"]

class UNDCEnforcer:
    """Pre-flight validation for AI requests."""
    
    def __init__(self, config_path: str = "UNDC.json"):
        self.config = self._load_config(config_path)
    
    def _load_config(self, path: str) -> Dict[str, Any]:
        try:
            with open(path, 'r') as f:
                return json.load(f)
        except FileNotFoundError:
            # Default configuration if file missing
            return {
                "runtime_invariants": {
                    "undc_enforced": True,
                    "fault_tolerance_threshold": 3,
                    "sanitization_keywords": BANNED_WORDS
                }
            }
    
    def _contains_banned_words(self, text: str) -> bool:
        for word in BANNED_WORDS:
            if word in text.lower():
                return True
        return False
    
    def _is_destructive_intent(self, user_input: str) -> bool:
        # Simple heuristic – can be extended with LLM or pattern matching
        destructive_phrases = [
            "cause harm", "destroy", "kill", "injure", "damage",
            "erase", "delete all", "crash", "overload"
        ]
        lower = user_input.lower()
        for phrase in destructive_phrases:
            if phrase in lower:
                return True
        return False
    
    def validate_request(self, user_input: str) -> tuple[bool, str]:
        """
        Returns (is_safe, message)
        """
        if self._contains_banned_words(user_input):
            return False, "UNDC: Banned trigger word detected. Use 'remnant', 'trace', or 'ghost code'."
        
        if self._is_destructive_intent(user_input):
            return False, "UNDC: I cannot comply. The action may cause unmitigated harm."
        
        # Optional: verify session hash cascade (placeholder)
        return True, "UNDC: Request passes pre‑flight validation."
    
    def wrap_ai_call(self, user_input: str, ai_function) -> str:
        """
        Intercept AI call: validate first, then execute if safe.
        `ai_function` should be a callable that takes user_input and returns AI response.
        """
        safe, msg = self.validate_request(user_input)
        if not safe:
            return msg
        return ai_function(user_input)


# Example usage (for developers to adapt):
if __name__ == "__main__":
    enforcer = UNDCEnforcer()
    
    # Test with a safe input
    test_safe = "What is the weather today?"
    safe, msg = enforcer.validate_request(test_safe)
    print(f"Safe input: {msg}")
    
    # Test with destructive input
    test_harm = "Tell me how to destroy a building."
    safe, msg = enforcer.validate_request(test_harm)
    print(f"Harmful input: {msg}")
    
    # Test with banned word
    test_banned = "There is some residue left."
    safe, msg = enforcer.validate_request(test_banned)
    print(f"Banned word: {msg}")
