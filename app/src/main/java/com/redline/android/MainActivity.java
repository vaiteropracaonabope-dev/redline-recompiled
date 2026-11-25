package com.redline.android;

import android.app.Activity;
import android.app.ActivityManager;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.PackageManager;
import android.graphics.Color;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.StatFs;
import android.provider.DocumentsContract;
import android.view.Gravity;
import android.view.View;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.ScrollView;
import android.widget.Spinner;
import android.widget.ArrayAdapter;
import android.widget.TextView;

import java.util.Locale;

public final class MainActivity extends Activity {
    private static final int PICK_GAME_FOLDER = 41;
    private static final int BG = Color.rgb(9, 13, 18);
    private static final int PANEL = Color.rgb(20, 27, 35);
    private static final int WHITE = Color.rgb(240, 244, 248);
    private static final int MUTED = Color.rgb(151, 164, 178);
    private static final int ACCENT = Color.rgb(232, 73, 45);
    private LinearLayout checks;
    private TextView gameFolderStatus;
    private Uri gameTree;
    private SharedPreferences prefs;

    @Override
    protected void onCreate(Bundle state) {
        super.onCreate(state);
        prefs = getSharedPreferences("redline_settings", MODE_PRIVATE);
        String saved = prefs.getString("game_tree", null);
        if (saved != null) gameTree = Uri.parse(saved);
        buildScreen();
        refreshChecks();
    }

    private int dp(float value) {
        return (int) (value * getResources().getDisplayMetrics().density + 0.5f);
    }

    private TextView text(String value, float size, int color, boolean bold) {
        TextView t = new TextView(this);
        t.setText(value);
        t.setTextSize(size);
        t.setTextColor(color);
        if (bold) t.setTypeface(null, android.graphics.Typeface.BOLD);
        return t;
    }

    private LinearLayout vertical() {
        LinearLayout l = new LinearLayout(this);
        l.setOrientation(LinearLayout.VERTICAL);
        return l;
    }

    private void buildScreen() {
        ScrollView scroll = new ScrollView(this);
        scroll.setFillViewport(true);
        scroll.setBackgroundColor(BG);
        LinearLayout root = vertical();
        root.setPadding(dp(20), dp(22), dp(20), dp(28));
        scroll.addView(root);

        TextView eyebrow = text("REDLINE RECOMPILED  /  ANDROID", 11, ACCENT, true);
        root.addView(eyebrow);

        TextView title = text("FORZA MOTORSPORT", 29, WHITE, true);
        title.setPadding(0, dp(9), 0, 0);
        root.addView(title);
        TextView subtitle = text("GAME LOADER  •  CONFIGURAÇÃO E DIAGNÓSTICO", 11, MUTED, true);
        subtitle.setPadding(0, dp(4), 0, dp(18));
        root.addView(subtitle);

        LinearLayout banner = panel();
        TextView bannerTitle = text("CENTRAL DE INICIALIZAÇÃO", 13, WHITE, true);
        banner.addView(bannerTitle);
        TextView bannerBody = text("Verifique o aparelho e selecione a pasta do jogo antes de iniciar.", 13, MUTED, false);
        bannerBody.setPadding(0, dp(7), 0, 0);
        banner.addView(bannerBody);
        root.addView(banner);

        section(root, "01  /  VERIFICAÇÃO DO SISTEMA");
        checks = vertical();
        root.addView(checks);

        section(root, "02  /  ARQUIVOS DO JOGO");
        LinearLayout game = panel();
        gameFolderStatus = text("Pasta do jogo: não selecionada", 13, MUTED, false);
        game.addView(gameFolderStatus);
        Button choose = button("SELECIONAR PASTA DO FORZA");
        choose.setOnClickListener(v -> chooseGameFolder());
        LinearLayout.LayoutParams chooseLp = new LinearLayout.LayoutParams(-1, dp(48));
        chooseLp.topMargin = dp(12);
        game.addView(choose, chooseLp);
        TextView gameHint = text("Selecione uma pasta que contenha Forza.xbe e a pasta media. Use somente arquivos de uma cópia que você tenha direito de utilizar.", 11, MUTED, false);
        gameHint.setPadding(0, dp(10), 0, 0);
        game.addView(gameHint);
        root.addView(game);
        updateGameFolderLabel();

        section(root, "03  /  GRÁFICOS");
        LinearLayout graphics = panel();
        addSetting(graphics, "Backend gráfico preferido", "graphics_backend",
                new String[]{"Automático (recomendado)", "Vulkan (preferência)", "OpenGL ES (preferência)"}, "Automático (recomendado)");
        addSetting(graphics, "Qualidade visual", "graphics_quality",
                new String[]{"Baixa", "Média", "Alta"}, "Média");
        addSetting(graphics, "Escala de resolução", "resolution_scale",
                new String[]{"50%", "75%", "100%"}, "75%");
        addSetting(graphics, "Limite de quadros", "frame_limit",
                new String[]{"30 FPS", "60 FPS"}, "30 FPS");
        TextView note = text("As preferências ficam salvas neste aparelho. Elas ainda não são aplicadas ao renderizador: a integração Vulkan/OpenGL e o runtime do jogo estão em desenvolvimento.", 11, MUTED, false);
        note.setPadding(0, dp(12), 0, 0);
        graphics.addView(note);
        root.addView(graphics);

        section(root, "04  /  INICIALIZAÇÃO");
        LinearLayout launchPanel = panel();
        TextView launchStatus = text("RUNTIME DO JOGO NÃO CONECTADO", 13, ACCENT, true);
        launchPanel.addView(launchStatus);
        TextView launchInfo = text("Esta versão é a interface de configuração. O botão de iniciar ficará disponível quando o runtime Android estiver integrado e validado.", 12, MUTED, false);
        launchInfo.setPadding(0, dp(8), 0, 0);
        launchPanel.addView(launchInfo);
        Button rescan = button("VERIFICAR NOVAMENTE");
        rescan.setOnClickListener(v -> refreshChecks());
        LinearLayout.LayoutParams rescanLp = new LinearLayout.LayoutParams(-1, dp(46));
        rescanLp.topMargin = dp(14);
        launchPanel.addView(rescan, rescanLp);
        root.addView(launchPanel);

        TextView footer = text("REDLINE ANDROID • BUILD DE DESENVOLVIMENTO", 10, MUTED, true);
        footer.setGravity(Gravity.CENTER);
        footer.setPadding(0, dp(22), 0, 0);
        root.addView(footer);
        setContentView(scroll);
    }

    private LinearLayout panel() {
        LinearLayout p = vertical();
        p.setPadding(dp(15), dp(15), dp(15), dp(15));
        android.graphics.drawable.GradientDrawable bg = new android.graphics.drawable.GradientDrawable();
        bg.setColor(PANEL);
        bg.setCornerRadius(dp(10));
        p.setBackground(bg);
        return p;
    }

    private void section(LinearLayout root, String label) {
        TextView t = text(label, 12, ACCENT, true);
        t.setPadding(0, dp(22), 0, dp(9));
        root.addView(t);
    }

    private Button button(String label) {
        Button b = new Button(this);
        b.setText(label);
        b.setTextSize(12);
        b.setTextColor(Color.WHITE);
        b.setAllCaps(false);
        b.setBackgroundTintList(android.content.res.ColorStateList.valueOf(ACCENT));
        return b;
    }

    private void addSetting(LinearLayout parent, String label, String key, String[] values, String fallback) {
        TextView title = text(label, 12, WHITE, true);
        title.setPadding(0, dp(8), 0, dp(4));
        parent.addView(title);
        Spinner spinner = new Spinner(this);
        ArrayAdapter<String> adapter = new ArrayAdapter<>(this, android.R.layout.simple_spinner_item, values);
        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        spinner.setAdapter(adapter);
        String selected = prefs.getString(key, fallback);
        int selectedIndex = 0;
        for (int i = 0; i < values.length; i++) if (values[i].equals(selected)) selectedIndex = i;
        spinner.setSelection(selectedIndex);
        spinner.setOnItemSelectedListener(new android.widget.AdapterView.OnItemSelectedListener() {
            @Override public void onItemSelected(android.widget.AdapterView<?> parentView, View view, int position, long id) {
                prefs.edit().putString(key, values[position]).apply();
            }
            @Override public void onNothingSelected(android.widget.AdapterView<?> parentView) { }
        });
        parent.addView(spinner);
    }

    private void addCheck(String name, String detail, boolean ok, boolean warning) {
        LinearLayout row = new LinearLayout(this);
        row.setOrientation(LinearLayout.HORIZONTAL);
        row.setGravity(Gravity.CENTER_VERTICAL);
        row.setPadding(dp(13), dp(11), dp(13), dp(11));
        android.graphics.drawable.GradientDrawable bg = new android.graphics.drawable.GradientDrawable();
        bg.setColor(PANEL);
        bg.setCornerRadius(dp(8));
        row.setBackground(bg);
        TextView mark = text(ok ? "✓" : (warning ? "!" : "×"), 18,
                ok ? Color.rgb(85, 205, 135) : (warning ? Color.rgb(245, 184, 66) : ACCENT), true);
        row.addView(mark, new LinearLayout.LayoutParams(dp(28), -2));
        LinearLayout labels = vertical();
        labels.addView(text(name, 12, WHITE, true));
        labels.addView(text(detail, 10, MUTED, false));
        row.addView(labels, new LinearLayout.LayoutParams(0, -2, 1));
        LinearLayout.LayoutParams lp = new LinearLayout.LayoutParams(-1, -2);
        lp.bottomMargin = dp(7);
        checks.addView(row, lp);
    }

    private void refreshChecks() {
        if (checks == null) return;
        checks.removeAllViews();
        boolean arm64 = false;
        for (String abi : Build.SUPPORTED_ABIS) if ("arm64-v8a".equals(abi)) arm64 = true;
        addCheck("Arquitetura ARM64", arm64 ? "ABI arm64-v8a disponível" : "Este aparelho não anuncia arm64-v8a", arm64, false);

        boolean version = Build.VERSION.SDK_INT >= 26;
        addCheck("Versão do Android", "Android " + Build.VERSION.RELEASE + " (API " + Build.VERSION.SDK_INT + "), mínimo 26",
                version, false);

        ActivityManager.MemoryInfo memory = new ActivityManager.MemoryInfo();
        ((ActivityManager) getSystemService(ACTIVITY_SERVICE)).getMemoryInfo(memory);
        long ramGb = memory.totalMem / (1024L * 1024L * 1024L);
        addCheck("Memória RAM", String.format(Locale.US, "Aproximadamente %d GB instalados", ramGb),
                ramGb >= 4, ramGb >= 3);

        StatFs storage = new StatFs(getFilesDir().getAbsolutePath());
        long freeGb = storage.getAvailableBytes() / (1024L * 1024L * 1024L);
        addCheck("Armazenamento livre", String.format(Locale.US, "%d GB disponíveis na área do aplicativo", freeGb),
                freeGb >= 4, freeGb >= 2);

        boolean vulkan = getPackageManager().hasSystemFeature(PackageManager.FEATURE_VULKAN_HARDWARE_LEVEL);
        int gl = getPackageManager().getSystemAvailableFeatures().length;
        addCheck("Suporte Vulkan", vulkan ? "O Android informa suporte Vulkan" : "Feature Vulkan não anunciada pelo sistema",
                vulkan, !vulkan);
        addCheck("Ambiente gráfico", "OpenGL ES / recursos do sistema: " + gl + " features listadas", true, true);

        boolean gameOk = hasValidGameFolder();
        addCheck("Arquivos do Forza", gameOk ? "Forza.xbe e pasta media encontrados" :
                "Selecione a pasta e confirme Forza.xbe + media", gameOk, !gameOk);
    }

    private void chooseGameFolder() {
        Intent intent = new Intent(Intent.ACTION_OPEN_DOCUMENT_TREE);
        intent.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION | Intent.FLAG_GRANT_PERSISTABLE_URI_PERMISSION);
        startActivityForResult(intent, PICK_GAME_FOLDER);
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if (requestCode == PICK_GAME_FOLDER && resultCode == RESULT_OK && data != null && data.getData() != null) {
            gameTree = data.getData();
            int flags = data.getFlags() & (Intent.FLAG_GRANT_READ_URI_PERMISSION | Intent.FLAG_GRANT_WRITE_URI_PERMISSION);
            try { getContentResolver().takePersistableUriPermission(gameTree, flags); } catch (SecurityException ignored) { }
            prefs.edit().putString("game_tree", gameTree.toString()).apply();
            updateGameFolderLabel();
            refreshChecks();
        }
    }

    private boolean hasValidGameFolder() {
        if (gameTree == null) return false;
        try {
            Uri children = DocumentsContract.buildChildDocumentsUriUsingTree(
                    gameTree, DocumentsContract.getTreeDocumentId(gameTree));
            String[] projection = {DocumentsContract.Document.COLUMN_DISPLAY_NAME,
                    DocumentsContract.Document.COLUMN_MIME_TYPE};
            boolean xbe = false, media = false;
            try (android.database.Cursor cursor = getContentResolver().query(children, projection, null, null, null)) {
                if (cursor == null) return false;
                while (cursor.moveToNext()) {
                    String name = cursor.getString(0);
                    String mime = cursor.getString(1);
                    if (name != null && "Forza.xbe".equalsIgnoreCase(name)) xbe = true;
                    if (name != null && "media".equalsIgnoreCase(name) &&
                            DocumentsContract.Document.MIME_TYPE_DIR.equals(mime)) media = true;
                }
            }
            return xbe && media;
        } catch (Exception e) {
            return false;
        }
    }

    private void updateGameFolderLabel() {
        if (gameFolderStatus == null) return;
        gameFolderStatus.setText(hasValidGameFolder()
                ? "Pasta selecionada: estrutura básica encontrada"
                : "Pasta do jogo: não selecionada ou arquivos incompletos");
    }
}
