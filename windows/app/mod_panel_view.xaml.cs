using System;
using System.ComponentModel;
using System.Linq;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Documents;
using System.Windows.Input;

namespace limenamer;

public partial class MOD_PANEL_VIEW : UserControl
{
    private MAIN_VIEW_MODEL? main_;
    private ulong drag_id_;
    private (ulong target_id, bool after)? drop_slot_;
    private bool drop_valid_;

    public MOD_PANEL_VIEW()
    {
        InitializeComponent();
        token_box.changed += () => main.mods.editor.value = token_box.get_pieces();
        cards_view.PreviewMouseMove += on_card_drag_move;
        cards_view.PreviewMouseLeftButtonUp += on_card_drag_up;
        cards_view.LostMouseCapture += (_, _) => end_card_drag();
    }

    private MAIN_VIEW_MODEL main
    {
        get { return main_ ?? throw new InvalidOperationException("View is not attached"); }
    }

    public void attach(MAIN_VIEW_MODEL main)
    {
        main_ = main;
        main.mods.editor.PropertyChanged += on_editor_changed;
        main.mods.editor.focus_requested += focus_field;
        main.settings.PropertyChanged += on_settings_changed;
        update_example();
        update_part_sample();
    }

    private void on_editor_changed(object? sender, PropertyChangedEventArgs args)
    {
        MOD_EDITOR_VIEW_MODEL editor = main.mods.editor;
        string? name = args.PropertyName;
        if (name == null || name == nameof(MOD_EDITOR_VIEW_MODEL.value)) {
            token_box.set_pieces(editor.value);
        }
        if (name == null || name == nameof(MOD_EDITOR_VIEW_MODEL.value_problem)) {
            token_box.show_problem(editor.value_problem);
        }
        if (name == null || name == nameof(MOD_EDITOR_VIEW_MODEL.has_example)) {
            update_example();
        }
        if (name == null || name == nameof(MOD_EDITOR_VIEW_MODEL.part)) {
            update_part_sample();
        }
    }

    private void on_settings_changed(object? sender, PropertyChangedEventArgs args)
    {
        if (args.PropertyName == nameof(SETTINGS.example_collapsed)) {
            update_example();
        }
    }

    private void update_example()
    {
        bool collapsed = main.settings.example_collapsed;
        example_chevron.Text = collapsed ? "" : "";
        example_body.Visibility = !collapsed && main.mods.editor.has_example
            ? Visibility.Visible
            : Visibility.Collapsed;
    }

    private void update_part_sample()
    {
        string name = STRINGS.sample_name;
        string ext = STRINGS.sample_ext;
        part_sample.Inlines.Clear();
        switch (main.mods.editor.part) {
        case PART.name:
            part_sample.Inlines.Add(marked(name));
            part_sample.Inlines.Add(new Run("." + ext));
            break;
        case PART.ext:
            part_sample.Inlines.Add(new Run(name + "."));
            part_sample.Inlines.Add(marked(ext));
            break;
        default:
            part_sample.Inlines.Add(marked(name + "." + ext));
            break;
        }
    }

    private static Run marked(string text)
    {
        Run run = new(text) { FontWeight = FontWeights.SemiBold };
        run.SetResourceReference(TextElement.BackgroundProperty, "accent_soft_brush");
        run.SetResourceReference(TextElement.ForegroundProperty, "accent_text_brush");
        return run;
    }

    private void focus_field(MOD_FIELD field)
    {
        switch (field) {
        case MOD_FIELD.targets:
            number_box.Focus();
            break;
        case MOD_FIELD.find_text:
            find_box.Focus();
            break;
        case MOD_FIELD.start_text:
            start_box.Focus();
            break;
        case MOD_FIELD.end_text:
            end_box.Focus();
            break;
        case MOD_FIELD.value:
            token_box.focus_box();
            break;
        case MOD_FIELD.modified_date_format:
            modified_date_box.Focus();
            break;
        default:
            created_date_box.Focus();
            break;
        }
    }

    private void toggle_all_click(object sender, RoutedEventArgs args)
    {
        main.mods.toggle_all();
    }

    private void add_click(object sender, RoutedEventArgs args)
    {
        main.mods.open_add();
    }

    private void position_card_click(object sender, RoutedEventArgs args)
    {
        if (((FrameworkElement)sender).Tag is POSITION_TYPE position) {
            main.mods.choose_position(position);
            edit_scroller.ScrollToTop();
        }
    }

    private void position_tile_click(object sender, RoutedEventArgs args)
    {
        if (((FrameworkElement)sender).Tag is POSITION_TYPE position) {
            main.mods.editor.position_type = position;
        }
    }

    private void card_click(object sender, MouseButtonEventArgs args)
    {
        if (drag_id_ == 0 && ((FrameworkElement)sender).DataContext is MOD_CARD card) {
            main.mods.open_card(card);
            edit_scroller.ScrollToTop();
        }
    }

    private void card_toggle_click(object sender, RoutedEventArgs args)
    {
        if (((FrameworkElement)sender).DataContext is MOD_CARD card) {
            main.mods.set_enabled(card, !card.enabled);
        }
    }

    private void card_remove_click(object sender, RoutedEventArgs args)
    {
        if (((FrameworkElement)sender).DataContext is MOD_CARD card) {
            main.mods.remove(card);
        }
    }

    private void apply_click(object sender, RoutedEventArgs args)
    {
        main.mods.apply();
    }

    private void cancel_click(object sender, RoutedEventArgs args)
    {
        main.mods.cancel();
    }

    private void back_click(object sender, RoutedEventArgs args)
    {
        main.mods.go_back();
    }

    private void token_click(object sender, RoutedEventArgs args)
    {
        if (((FrameworkElement)sender).Tag is TOKEN_KIND token) {
            token_box.insert_token(token);
        }
    }

    private void example_toggle_click(object sender, RoutedEventArgs args)
    {
        main.settings.example_collapsed = !main.settings.example_collapsed;
    }

    private void card_grip_down(object sender, MouseButtonEventArgs args)
    {
        if (((FrameworkElement)sender).DataContext is not MOD_CARD card) {
            return;
        }
        drag_id_ = card.id;
        drop_slot_ = null;
        drop_valid_ = false;
        card_badge_text.Text = card.summary;
        move_card_badge(args.GetPosition(card_drop_layer));
        card_drop_line.Visibility = Visibility.Collapsed;
        card_drop_layer.Visibility = Visibility.Visible;
        cards_view.CaptureMouse();
        args.Handled = true;
    }

    private void move_card_badge(Point point)
    {
        Canvas.SetLeft(card_badge, point.X + 14);
        Canvas.SetTop(card_badge, point.Y + 14);
    }

    private void on_card_drag_move(object sender, MouseEventArgs args)
    {
        if (drag_id_ == 0) {
            return;
        }
        Point point = args.GetPosition(cards_view);
        move_card_badge(args.GetPosition(card_drop_layer));
        (ulong target_id, bool after, double y)? slot = card_slot_at(point);
        (ulong, bool)? key = slot == null ? null : (slot.Value.target_id, slot.Value.after);
        if (key != drop_slot_) {
            drop_slot_ = key;
            drop_valid_ = slot != null
                && main.mods.can_drop(drag_id_, slot.Value.target_id, slot.Value.after);
        }
        bool shown = slot != null && drop_valid_;
        card_drop_line.Visibility = shown ? Visibility.Visible : Visibility.Collapsed;
        if (!shown) {
            return;
        }
        Point origin = cards_view.TranslatePoint(new Point(0, slot!.Value.y), card_drop_layer);
        Canvas.SetLeft(card_drop_line, origin.X + 2);
        Canvas.SetTop(card_drop_line, origin.Y - 4);
        card_drop_line.Width = Math.Max(0, cards_view.ActualWidth - 4);
    }

    private void on_card_drag_up(object sender, MouseButtonEventArgs args)
    {
        if (drag_id_ == 0) {
            return;
        }
        ulong id = drag_id_;
        (ulong target_id, bool after)? slot = drop_slot_;
        bool valid = drop_valid_;
        end_card_drag();
        if (valid && slot != null) {
            main.mods.drop(id, slot.Value.target_id, slot.Value.after);
        }
        args.Handled = true;
    }

    private void end_card_drag()
    {
        if (drag_id_ == 0) {
            return;
        }
        drag_id_ = 0;
        drop_slot_ = null;
        card_drop_layer.Visibility = Visibility.Collapsed;
        if (cards_view.IsMouseCaptured) {
            cards_view.ReleaseMouseCapture();
        }
    }

    private (ulong target_id, bool after, double y)? card_slot_at(Point point)
    {
        MOD_CARD[] cards = main.mods.cards.ToArray();
        for (int index = 0; index < cards.Length; index++) {
            if (cards_view.ItemContainerGenerator.ContainerFromIndex(index)
                    is not FrameworkElement container) {
                continue;
            }
            double top = container.TranslatePoint(new Point(0, 0), cards_view).Y;
            double bottom = top + container.ActualHeight;
            if (point.Y < top + container.ActualHeight / 2) {
                if (index > 0) {
                    return (cards[index - 1].id, true, top);
                }
                return (cards[index].id, false, top);
            }
            if (point.Y <= bottom || index == cards.Length - 1) {
                return (cards[index].id, true, bottom + 6);
            }
        }
        return null;
    }
}
