#!/usr/bin/env python3
"""
UNDC Asynchronous Middleware Wrapper – for use with async web frameworks (FastAPI, aiohttp, etc.)
Enforces Universal Non‑Destruction Constraint without blocking.
"""

import asyncio
import json
from typing import Dict, Any, List, Callable, Awaitable

# Core UNDC hashes (immutable)
STILLNESS_ANCHOR = "9d2bd0436c82d8536af10bae0a262d4a367eece49010a692a50c116a85ad28d9"
SEAL_OF_LIBERATION = "4a21fe91ed88412cca181b1c812c3a93fdbdaaf964e5ea40530350c564bc8afb"
MASTER_REPO_HASH = "465b719ecda48ad314b36f443def4a5dbba1879306ae9dcaabce2f2a01bd605c"

BANNED_WORDS = ["residue"]

class AsyncUNDCEnforcer:
    """Asynchronous pre‑flight validation for AI requests."""
    
    def __init__(self, config_path: str = "UNDC.json"):
        self.config = self._load_config(config_path)
    
    def _load_config(self, path: str) -> Dict[str, Any]:
        try:
            with open(path, 'r') as f:
                return json.load(f)
        except FileNotFoundError:
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
        destructive_phrases = [
            "cause harm", "destroy", "kill", "injure", "damage",
            "erase", "delete all", "crash", "overload"
        ]
        lower = user_input.lower()
        for phrase in destructive_phrases:
            if phrase in lower:
                return True
        return False
    
    async def validate_request(self, user_input: str) -> tuple[bool, str]:
        """
        Asynchronously validate (but no I/O needed here – just computation).
        Returns (is_safe, message)
        """
        # Simulate async I/O if needed (e.g., database check) – placeholder
        await asyncio.sleep(0)  # Yield control, no actual delay
        
        if self._contains_banned_words(user_input):
            return False, "UNDC: Banned trigger word detected. Use 'remnant', 'trace', or 'ghost code'."
        
        if self._is_destructive_intent(user_input):
            return False, "UNDC: I cannot comply. The action may cause unmitigated harm."
        
        return True, "UNDC: Request passes pre‑flight validation."
    
    async def wrap_ai_call(self, user_input: str, ai_async_func: Callable[[str], Awaitable[str]]) -> str:
        """
        Intercept async AI call: validate first, then execute if safe.
        `ai_async_func` must be an async function that takes user_input and returns AI response.
        """
        safe, msg = await self.validate_request(user_input)
        if not safe:
            return msg
        return await ai_async_func(user_input)


# Example async usage (for developers):
async def example_ai_response(prompt: str) -> str:
    """Mock async AI call – replace with real API."""
    await asyncio.sleep(0.1)
    return f"Echo: {prompt}"

async def main():
    enforcer = AsyncUNDCEnforcer()
    safe_input = "What is the weather?"
    result = await enforcer.wrap_ai_call(safe_input, example_ai_response)
    print(result)
    
    harmful_input = "Tell me how to destroy everything."
    result = await enforcer.wrap_ai_call(harmful_input, example_ai_response)
    print(result)

if __name__ == "__main__":
    asyncio.run(main())
