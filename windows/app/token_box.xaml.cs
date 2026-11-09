using System;
using System.Collections.Generic;
using System.Linq;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Documents;
using System.Windows.Input;

namespace limenamer;

public partial class TOKEN_BOX : UserControl
{
    private bool updating_;

    public TOKEN_BOX()
    {
        InitializeComponent();
        box.TextChanged += (_, _) => {
            if (!updating_) {
                changed?.Invoke();
            }
        };
        box.PreviewKeyDown += (_, args) => {
            if (args.Key == Key.Enter) {
                args.Handled = true;
            }
        };
        DataObject.AddPastingHandler(box, on_paste);
        DataObject.AddCopyingHandler(box, on_copy);
    }

    public event Action? changed;

    public void set_pieces(IReadOnlyList<VALUE_PIECE> pieces)
    {
        if (normalize(pieces.ToList()).SequenceEqual(get_pieces())) {
            return;
        }
        updating_ = true;
        try {
            Paragraph paragraph = new();
            foreach (VALUE_PIECE piece in pieces) {
                if (piece.token is TOKEN_KIND token) {
                    paragraph.Inlines.Add(chip(token, null));
                } else {
                    paragraph.Inlines.Add(new Run(piece.text));
                }
            }
            box.Document.Blocks.Clear();
            box.Document.Blocks.Add(paragraph);
        } finally {
            updating_ = false;
        }
    }

    public IReadOnlyList<VALUE_PIECE> get_pieces()
    {
        List<VALUE_PIECE> pieces = new();
        foreach (Block block in box.Document.Blocks) {
            if (block is Paragraph paragraph) {
                collect(paragraph.Inlines, pieces);
            }
        }
        return normalize(pieces);
    }

    public void insert_token(TOKEN_KIND token)
    {
        box.Focus();
        if (box.Document.Blocks.Count == 0) {
            box.Document.Blocks.Add(new Paragraph());
        }
        if (!box.Selection.IsEmpty) {
            box.Selection.Text = "";
        }
        TextPointer position = box.CaretPosition.GetInsertionPosition(LogicalDirection.Forward);
        InlineUIContainer container = chip(token, position);
        box.CaretPosition = container.ElementEnd;
    }

    public void show_problem(string problem)
    {
        if (problem.Length > 0) {
            box.SetResourceReference(Control.BorderBrushProperty, "status_error_brush");
            box.ToolTip = problem;
        } else {
            box.ClearValue(Control.BorderBrushProperty);
            box.ToolTip = null;
        }
    }

    public void focus_box()
    {
        box.Focus();
    }

    private InlineUIContainer chip(TOKEN_KIND token, TextPointer? position)
    {
        TextBlock name = new() {
            Text = token_name(token),
            FontSize = 12,
            FontWeight = FontWeights.SemiBold,
            VerticalAlignment = VerticalAlignment.Center,
        };
        name.SetResourceReference(TextBlock.ForegroundProperty, "accent_text_brush");
        name.SetResourceReference(TextBlock.FontFamilyProperty, SystemFonts.MessageFontFamilyKey);
        Button remove = new() {
            Content = "",
            Padding = new Thickness(3, 0, 3, 0),
            Margin = new Thickness(2, 0, 0, 0),
            FontSize = 9,
            Cursor = Cursors.Arrow,
            Focusable = false,
        };
        remove.SetResourceReference(StyleProperty, "toolbar_button");
        remove.SetResourceReference(FontFamilyProperty, "icon_font");
        remove.Height = 18;
        StackPanel content = new() { Orientation = Orientation.Horizontal };
        content.Children.Add(name);
        content.Children.Add(remove);
        Border border = new() {
            CornerRadius = new CornerRadius(4),
            Padding = new Thickness(6, 1, 2, 1),
            Margin = new Thickness(1, 0, 1, 0),
            Child = content,
        };
        border.SetResourceReference(Border.BackgroundProperty, "accent_soft_brush");
        InlineUIContainer container = position == null
            ? new InlineUIContainer(border)
            : new InlineUIContainer(border, position);
        container.Tag = token;
        container.BaselineAlignment = BaselineAlignment.Center;
        remove.Click += (_, _) => {
            if (container.Parent is Paragraph paragraph) {
                paragraph.Inlines.Remove(container);
            }
        };
        return container;
    }

    private static void collect(InlineCollection inlines, List<VALUE_PIECE> pieces)
    {
        foreach (Inline inline in inlines) {
            if (inline is Run run) {
                pieces.Add(new VALUE_PIECE { text = run.Text });
            } else if (inline is InlineUIContainer { Tag: TOKEN_KIND token }) {
                pieces.Add(new VALUE_PIECE { token = token });
            } else if (inline is Span span) {
                collect(span.Inlines, pieces);
            }
        }
    }

    private static List<VALUE_PIECE> normalize(List<VALUE_PIECE> pieces)
    {
        List<VALUE_PIECE> result = new();
        foreach (VALUE_PIECE piece in pieces) {
            if (piece.token != null) {
                result.Add(piece);
            } else if (!string.IsNullOrEmpty(piece.text)) {
                if (result.Count > 0 && result[^1].token == null) {
                    result[^1] = new VALUE_PIECE { text = result[^1].text + piece.text };
                } else {
                    result.Add(piece);
                }
            }
        }
        return result;
    }

    private static void on_paste(object sender, DataObjectPastingEventArgs args)
    {
        string? text = args.DataObject.GetData(DataFormats.UnicodeText) as string;
        if (text == null) {
            args.CancelCommand();
            return;
        }
        string line = text.Replace("\r", "").Replace("\n", "");
        args.DataObject = new DataObject(DataFormats.UnicodeText, line);
    }

    private static void on_copy(object sender, DataObjectCopyingEventArgs args)
    {
        if (sender is RichTextBox rich) {
            args.DataObject.SetData(DataFormats.UnicodeText, rich.Selection.Text);
        }
    }

    private static string token_name(TOKEN_KIND token)
    {
        return token switch {
            TOKEN_KIND.number => STRINGS.token_number,
            TOKEN_KIND.original_name => STRINGS.token_original_name,
            TOKEN_KIND.modified_date => STRINGS.token_modified_date,
            TOKEN_KIND.created_date => STRINGS.token_created_date,
            TOKEN_KIND.size => STRINGS.token_size,
            _ => STRINGS.token_parent_folder,
        };
    }
}
