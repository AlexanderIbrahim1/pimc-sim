"""
The ReplacementTemplate instance is a thin wrapper around a string that makes
it a bit more convenient to replace certain collections of substrings with other
collections of substrings.
"""

from pathlib import Path
from typing import Optional
from typing import Union

Pathlike = Union[str, Path]


class ReplacementTemplate:
    def __init__(self, template_contents: str) -> None:
        self._template_contents = template_contents

    def replace(self, original: Union[dict[str, Pathlike], str], new: Optional[Pathlike] = None) -> str:
        if isinstance(original, dict):
            if new is not None:
                raise RuntimeError(self._usage_message())
            else:
                return self._replace_with_dict(original)
        elif isinstance(original, Pathlike):
            if new is None:
                raise RuntimeError(self._usage_message())
            else:
                new = self._format_replacement(new)
                return self._template_contents.replace(original, new)

    def _replace_with_dict(self, replacements: dict[str, Pathlike]) -> str:
        new_string = self._template_contents
        for old, new in replacements.items():
            new = self._format_replacement(new)
            new_string = new_string.replace(old, new)

        return new_string

    def _usage_message(self) -> str:
        message = "\n".join(
            [
                "The `replace()` method can be called in one of two ways:",
                ".replace(old_string, new_string)",
                ".replace(dict_mapping_all_targets_with_replacements)",
            ]
        )

        return message

    def _format_replacement(self, replacement: Pathlike) -> str:
        """Helper function to make sure paths are output with double quotations around them."""
        if isinstance(replacement, str):
            return replacement
        else:
            return f'"{str(replacement)}"'
