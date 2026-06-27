#!/usr/bin/env python3
import argparse
import html
import re
import subprocess
import sys
from pathlib import Path


ANTLR_CP = ":".join(
    [
        "/usr/share/java/antlr4/antlr4.jar",
        "/usr/share/java/antlr4/antlr4-runtime.jar",
        "/usr/share/java/antlr3-runtime.jar",
        "/usr/share/java/stringtemplate4/ST4.jar",
        "/usr/share/java/treelayout/org.abego.treelayout.core.jar",
    ]
)


class Node:
    def __init__(self, label: str):
        self.label = label
        self.children = []


def run_antlr(grammar: Path, rule: str, input_file: Path) -> str:
    cmd = [
        "java",
        "-cp",
        ANTLR_CP,
        "org.antlr.v4.gui.Interpreter",
        str(grammar),
        rule,
        "-tree",
        str(input_file),
    ]
    result = subprocess.run(cmd, capture_output=True, text=True, check=False)
    if result.returncode != 0:
        sys.stderr.write(result.stderr)
        raise SystemExit(result.returncode)
    lines = [line.strip() for line in result.stdout.splitlines() if line.strip()]
    lines = [line for line in lines if not line.startswith("pyenv: cannot rehash:")]
    if not lines:
        raise SystemExit("ANTLR produced no parse tree output.")
    return lines[-1]


def normalize_tree_text(tree_text: str) -> str:
    return re.sub(r"\b([A-Za-z_][A-Za-z0-9_]*)\:\d+\b", r"\1", tree_text)


def tokenize(tree_text: str):
    return re.findall(r"\(|\)|[^\s()]+", tree_text)


def parse_node(tokens):
    stack = []
    root = None
    i = 0

    while i < len(tokens):
        token = tokens[i]

        if token == "(":
            if i + 1 >= len(tokens):
                raise ValueError("Unexpected end of input after '('")
            label = tokens[i + 1]
            node = Node(label)
            if stack:
                stack[-1].children.append(node)
            else:
                root = node
            stack.append(node)
            i += 2
            continue

        if token == ")":
            if not stack:
                if root is None:
                    raise ValueError("Unexpected ')' with empty stack")
                i += 1
                continue
            stack.pop()
            i += 1
            continue

        if not stack:
            if root is None:
                raise ValueError(f"Leaf token {token!r} found outside any node")
            root.children.append(Node(token))
            i += 1
            continue
        stack[-1].children.append(Node(token))
        i += 1

    if stack:
        raise ValueError(f"Missing closing ')' for node {stack[-1].label!r}")
    if root is None:
        raise ValueError("No root node parsed from token stream")
    return root


def to_dot(root: Node) -> str:
    lines = [
        "digraph ParseTree {",
        '  rankdir=TB;',
        '  graph [bgcolor="white", pad="0.3", nodesep="0.25", ranksep="0.35"];',
        '  node [shape=box, style="rounded,filled", fillcolor="#f8fafc", color="#334155", fontname="Helvetica"];',
        '  edge [color="#64748b"];',
    ]
    counter = 0

    def visit(node: Node):
        nonlocal counter
        node_id = f"n{counter}"
        counter += 1
        label = html.escape(node.label)
        lines.append(f'  {node_id} [label=<{label}>];')
        for child in node.children:
            child_id = visit(child)
            lines.append(f"  {node_id} -> {child_id};")
        return node_id

    visit(root)
    lines.append("}")
    return "\n".join(lines) + "\n"


def main():
    parser = argparse.ArgumentParser(description="Render an ANTLR parse tree to DOT/SVG.")
    parser.add_argument("--grammar", required=True, help="Path to .g4 grammar")
    parser.add_argument("--rule", required=True, help="Start rule")
    parser.add_argument("--input", required=True, help="Input source file")
    parser.add_argument(
        "--out-prefix",
        required=True,
        help="Output prefix path, without extension",
    )
    args = parser.parse_args()

    grammar = Path(args.grammar).resolve()
    input_file = Path(args.input).resolve()
    out_prefix = Path(args.out_prefix).resolve()
    out_prefix.parent.mkdir(parents=True, exist_ok=True)

    raw_tree = run_antlr(grammar, args.rule, input_file)
    cleaned_tree = normalize_tree_text(raw_tree)
    tokens = tokenize(cleaned_tree)
    root = parse_node(tokens)

    dot_path = out_prefix.with_suffix(".dot")
    txt_path = out_prefix.with_suffix(".tree.txt")
    svg_path = out_prefix.with_suffix(".svg")

    dot_path.write_text(to_dot(root), encoding="utf-8")
    txt_path.write_text(raw_tree + "\n", encoding="utf-8")

    dot_cmd = ["dot", "-Tsvg", str(dot_path), "-o", str(svg_path)]
    result = subprocess.run(dot_cmd, capture_output=True, text=True, check=False)
    if result.returncode != 0:
        sys.stderr.write(result.stderr)
        raise SystemExit(result.returncode)

    print(f"Tree text: {txt_path}")
    print(f"DOT file:  {dot_path}")
    print(f"SVG file:  {svg_path}")


if __name__ == "__main__":
    main()
