﻿using System;
using System.IO;
using System.Text;
using System.Collections.Generic;
namespace toc
{
    public class Builder
    {
        public static string getAnchorId(string title)
        {
            StringBuilder sb = new StringBuilder();
            title = title.Trim().ToLower();
            foreach (var ch in title)
            {
                if (ch >= 'a' && ch <= 'z' || ch >= '0' && ch <= '9'
                    || ch == '-')
                    sb.Append(ch);
                else if (ch==' ' || ch =='_')
                    sb.Append('-');
            }
            return sb.ToString();
        }

        public class Node
        {
            public List<string> fileNamePrefix = new List<string>();
            public string title;
            public string fileID;
            public List<string> sections = new List<string>();
            public List<Node> children = new List<Node>();
        }

        public static void buildTOC(StringBuilder sb, Node n)
        {
            sb.AppendFormat("<li data-link=\"{0}\"><span>{1}</span>\n", n.fileID, n.title);
            if (n.children.Count != 0)
            {
                sb.AppendLine("<ul class=\"toc_list\">");
                foreach(var c in n.children)
                    buildTOC(sb, c);
                sb.AppendLine("</ul>");
            }
            else if (n.sections.Count != 0)
            {
                sb.AppendLine("<ul class=\"toc_list\">");
                foreach (var s in n.sections)
                {
                    sb.AppendFormat("<li data-link=\"{0}#{1}\"><span>{2}</span></li>\n", n.fileID, getAnchorId(s), s);
                }
                sb.AppendLine("</ul>");
            }
            sb.AppendLine("</li>");
        }
        public static string buildTOC(Node n)
        {
            StringBuilder sb = new StringBuilder();
            sb.Append(@"<ul class=""toc_root_list"">");
            buildTOC(sb, n);
            sb.Append(@"</ul>");
            return sb.ToString();
        }

        public static bool isChild(Node parent, Node child)
        {
            if (parent.fileNamePrefix.Count < child.fileNamePrefix.Count)
            {
                bool equal = true;
                for (int k = 0; k < parent.fileNamePrefix.Count; k++)
                {
                    if (parent.fileNamePrefix[k] != child.fileNamePrefix[k])
                    {
                        equal = false;
                        break;
                    }
                }
                return equal;
            }
            return false;
        }
        public static string Run(string path)
        {
            StringBuilder outputSB = new StringBuilder();
            outputSB.AppendFormat("Building table of contents from {0}...\n", path);
            var files = System.IO.Directory.EnumerateFiles(path, "*.md");
            List<Node> nodes = new List<Node>();
            foreach (var f in files)
            {
                var content = File.ReadAllLines(f);
                Node node = new Node();
                node.fileID = Path.GetFileNameWithoutExtension(f);
                outputSB.AppendFormat("  {0}.md\n", node.fileID);
                for (int i = 1; i < content.Length; i++)
                {
                    if (content[i-1].StartsWith("layout: "))
                        continue;
                    if (content[i].StartsWith("==="))
                        node.title = content[i-1];
                    if (content[i].StartsWith("---"))
                        node.sections.Add(content[i-1]);
                    if (content[i].StartsWith("#") && !content[i].StartsWith("##") && node.title == null)
                        node.title = content[i].Substring(1, content[i].Length - 1).Trim();
                    if (content[i].StartsWith("##") && !content[i].StartsWith("###"))
                        node.sections.Add(content[i].Substring(2, content[i].Length - 2).Trim());
                }
                if (node.title == null)
                {
                    outputSB.AppendFormat("Error: {0} does not define a title.", f);
                    node.title = "Untitiled";
                }
                var titleSecs = Path.GetFileName(f).Split('-');
                foreach (var s in titleSecs)
                {
                    if (s.Length == 2 && s[1]>='0' && s[1] <= '9')
                    {
                        node.fileNamePrefix.Add(s);
                    }
                    else
                    {
                        break;
                    }
                }
                // Find parent node.
                Node parent=null;
                for (int l = nodes.Count-1; l>=0; l--)
                {
                    var n = nodes[l];
                    if (isChild(n, node))
                    {
                       parent = n;
                       break;
                    }
                }
                if (parent != null)
                    parent.children.Add(node);
                else
                {
                    // find child
                    foreach (var other in nodes)
                    {
                        if (isChild(node, other))
                        {
                            node.children.Add(other);
                        }
                    }
                    foreach (var c in node.children)
                    {
                        nodes.Remove(c);

                    }
                    nodes.Add(node);
                }
            }
            var root = nodes.Find(x=>x.fileID=="index");
            if (root != null)
            {
                var html = buildTOC(root);
                var outPath = Path.Combine(path, "user-guide-toc.html");
                File.WriteAllText(outPath, html);
                outputSB.AppendFormat("Output written to: {0}\n", outPath);
            }
            return outputSB.ToString();
        }
    }
}