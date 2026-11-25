package com.redline.android;

import android.app.Activity;
import android.app.ActivityManager;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.PackageManager;
import android.graphics.Color;
import android.graphics.Typeface;
import android.graphics.drawable.GradientDrawable;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.StatFs;
import android.provider.DocumentsContract;
import android.view.Gravity;
import android.view.View;
import android.widget.*;
import java.util.Locale;

public final class MainActivity extends Activity {
    private static final int PICK_GAME_FOLDER = 41;
    private static final int BG = Color.rgb(8, 12, 18);
    private static final int PANEL = Color.rgb(19, 27, 37);
    private static final int WHITE = Color.rgb(241, 245, 249);
    private static final int MUTED = Color.rgb(145, 160, 176);
    private static final int RED = Color.rgb(232, 67, 43);
    private static final String DOWNLOAD_URL = "https://github.com/vaiteropracaonabope-dev/redline-recompiled/actions/workflows/android-apk.yml";

    private SharedPreferences prefs;
    private LinearLayout checks;
    private TextView gameStatus;
    private TextView runtimeStatus;
    private Uri gameTree;
    private String nativeStatus = "Biblioteca nativa não carregada.";

    private native String nativeRuntimeStatus();

    @Override protected void onCreate(Bundle state) {
        super.onCreate(state);
        prefs = getSharedPreferences("redline_settings", MODE_PRIVATE);
        String saved = prefs.getString("game_tree", null);
        if (saved != null) gameTree = Uri.parse(saved);
        try {
            System.loadLibrary("redline_android");
            nativeStatus = nativeRuntimeStatus();
        } catch (UnsatisfiedLinkError error) {
            nativeStatus = "Ponte nativa indisponível nesta compilação.";
        }
        getWindow().setStatusBarColor(BG);
        getWindow().setNavigationBarColor(BG);
        buildScreen();
        refreshChecks();
    }

    private int dp(float v) { return (int)(v * getResources().getDisplayMetrics().density + 0.5f); }

    private TextView label(String value, float size, int color, boolean bold) {
        TextView t = new TextView(this);
        t.setText(value);
        t.setTextSize(size);
        t.setTextColor(color);
        if (bold) t.setTypeface(Typeface.DEFAULT, Typeface.BOLD);
        return t;
    }

    private GradientDrawable shape(int color, int radius) {
        GradientDrawable d = new GradientDrawable();
        d.setColor(color);
        d.setCornerRadius(dp(radius));
        return d;
    }

    private LinearLayout column() {
        LinearLayout l = new LinearLayout(this);
        l.setOrientation(LinearLayout.VERTICAL);
        return l;
    }

    private LinearLayout card() {
        LinearLayout c = column();
        c.setPadding(dp(16), dp(15), dp(16), dp(15));
        c.setBackground(shape(PANEL, 14));
        return c;
    }

    private void heading(LinearLayout root, String value) {
        TextView t = label(value, 11, RED, true);
        t.setLetterSpacing(0.08f);
        t.setPadding(0, dp(22), 0, dp(9));
        root.addView(t);
    }

    private Button actionButton(String value, boolean primary) {
        Button b = new Button(this);
        b.setText(value);
        b.setTextSize(12);
        b.setTypeface(Typeface.DEFAULT, Typeface.BOLD);
        b.setTextColor(WHITE);
        b.setAllCaps(false);
        b.setBackgroundTintList(android.content.res.ColorStateList.valueOf(primary ? RED : Color.rgb(42, 55, 69)));
        return b;
    }

    private void buildScreen() {
        ScrollView scroll = new ScrollView(this);
        scroll.setFillViewport(true);
        scroll.setBackgroundColor(BG);
        LinearLayout root = column();
        root.setPadding(dp(19), dp(20), dp(19), dp(30));
        scroll.addView(root);

        TextView brand = label("REDLINE  /  ANDROID EDITION", 10, RED, true);
        brand.setLetterSpacing(0.16f);
        root.addView(brand);

        TextView title = label("FORZA\nMOTORSPORT", 34, WHITE, true);
        title.setLineSpacing(0, 0.92f);
        title.setPadding(0, dp(10), 0, dp(4));
        root.addView(title);

        TextView sub = label("2005  •  GAME SETUP", 11, MUTED, true);
        sub.setLetterSpacing(0.12f);
        sub.setPadding(0, 0, 0, dp(18));
        root.addView(sub);

        LinearLayout hero = card();
        hero.setBackground(shape(Color.rgb(29, 38, 49), 16));
        TextView heroTag = label("CENTRAL DE INICIALIZAÇÃO", 10, RED, true);
        heroTag.setLetterSpacing(0.1f);
        hero.addView(heroTag);
        hero.addView(label("Prepare o ambiente antes de jogar.", 16, WHITE, true));
        TextView heroBody = label("Diagnóstico do aparelho, seleção dos arquivos e preferências gráficas em um só lugar.", 12, MUTED, false);
        heroBody.setPadding(0, dp(6), 0, 0);
        hero.addView(heroBody);
        root.addView(hero);

        heading(root, "01  /  DIAGNÓSTICO DO APARELHO");
        checks = column();
        root.addView(checks);

        heading(root, "02  /  ARQUIVOS DO JOGO");
        LinearLayout files = card();
        gameStatus = label("Nenhuma pasta selecionada", 13, MUTED, true);
        files.addView(gameStatus);
        Button choose = actionButton("ESCOLHER PASTA DO JOGO", true);
        addButton(files, choose);
        TextView fileHint = label("Aponte para a pasta que contém Forza.xbe e media. Os arquivos do jogo não vêm incluídos no aplicativo.", 11, MUTED, false);
        fileHint.setPadding(0, dp(8), 0, 0);
        files.addView(fileHint);
        root.addView(files);
        updateGameLabel();

        heading(root, "03  /  CONFIGURAÇÃO GRÁFICA");
        LinearLayout graphics = card();
        addSetting(graphics, "Backend preferido", "backend",
                new String[]{"Automático", "Vulkan", "OpenGL ES"}, "Automático");
        addSetting(graphics, "Qualidade", "quality",
                new String[]{"Baixa", "Média", "Alta"}, "Média");
        addSetting(graphics, "Escala de resolução", "scale",
                new String[]{"50%", "75%", "100%"}, "75%");
        addSetting(graphics, "Limite de quadros", "fps",
                new String[]{"30 FPS", "60 FPS"}, "30 FPS");
        TextView graphicsHint = label("Preferências salvas localmente. Elas só terão efeito quando o renderizador Android estiver conectado.", 11, MUTED, false);
        graphicsHint.setPadding(0, dp(12), 0, 0);
        graphics.addView(graphicsHint);
        root.addView(graphics);

        heading(root, "04  /  MOTOR E DOWNLOADS");
        LinearLayout runtime = card();
        runtimeStatus = label(nativeStatus, 12, WHITE, true);
        runtime.addView(runtimeStatus);
        TextView runtimeHint = label("A ponte JNI/NDK já é empacotada no APK. O núcleo que executa o jogo e o backend gráfico ainda precisam ser portados.", 11, MUTED, false);
        runtimeHint.setPadding(0, dp(7), 0, 0);
        runtime.addView(runtimeHint);

        Button refresh = actionButton("REFAZER VERIFICAÇÃO", false);
        addButton(runtime, refresh);
        refresh.setOnClickListener(v -> refreshChecks());

        Button download = actionButton("BAIXAR APK MAIS RECENTE", true);
        addButton(runtime, download);
        download.setOnClickListener(v -> openDownloadPage());

        TextView downloadHint = label("O botão abre as compilações oficiais do projeto no GitHub. Baixe o artefato redline-android-debug-apk da execução mais recente concluída.", 10, MUTED, false);
        downloadHint.setPadding(0, dp(8), 0, 0);
        runtime.addView(downloadHint);
        root.addView(runtime);

        TextView footer = label("REDLINE RECOMPILED  •  BUILD EXPERIMENTAL", 9, MUTED, true);
        footer.setGravity(Gravity.CENTER);
        footer.setLetterSpacing(0.08f);
        footer.setPadding(0, dp(24), 0, 0);
        root.addView(footer);
        setContentView(scroll);
    }

    private void addButton(LinearLayout parent, Button button) {
        LinearLayout.LayoutParams lp = new LinearLayout.LayoutParams(-1, dp(48));
        lp.topMargin = dp(12);
        parent.addView(button, lp);
    }

    private void addSetting(LinearLayout parent, String title, String key, String[] options, String fallback) {
        TextView caption = label(title, 12, WHITE, true);
        caption.setPadding(0, dp(8), 0, dp(2));
        parent.addView(caption);
        Spinner spinner = new Spinner(this);
        ArrayAdapter<String> adapter = new ArrayAdapter<>(this, android.R.layout.simple_spinner_item, options);
        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        spinner.setAdapter(adapter);
        String selected = prefs.getString(key, fallback);
        for (int i = 0; i < options.length; i++) if (options[i].equals(selected)) spinner.setSelection(i);
        spinner.setOnItemSelectedListener(new android.widget.AdapterView.OnItemSelectedListener() {
            @Override public void onItemSelected(android.widget.AdapterView<?> p, View v, int pos, long id) {
                prefs.edit().putString(key, options[pos]).apply();
            }
            @Override public void onNothingSelected(android.widget.AdapterView<?> p) { }
        });
        parent.addView(spinner);
    }

    private void addCheck(String name, String detail, boolean ok, boolean warning) {
        LinearLayout row = new LinearLayout(this);
        row.setGravity(Gravity.CENTER_VERTICAL);
        row.setPadding(dp(13), dp(11), dp(13), dp(11));
        row.setBackground(shape(PANEL, 10));
        TextView mark = label(ok ? "✓" : (warning ? "!" : "×"), 18,
                ok ? Color.rgb(85, 205, 135) : (warning ? Color.rgb(245, 184, 66) : RED), true);
        row.addView(mark, new LinearLayout.LayoutParams(dp(28), -2));
        LinearLayout words = column();
        words.addView(label(name, 12, WHITE, true));
        words.addView(label(detail, 10, MUTED, false));
        row.addView(words, new LinearLayout.LayoutParams(0, -2, 1));
        LinearLayout.LayoutParams lp = new LinearLayout.LayoutParams(-1, -2);
        lp.bottomMargin = dp(7);
        checks.addView(row, lp);
    }

    private void refreshChecks() {
        if (checks == null) return;
        checks.removeAllViews();
        boolean arm64 = false;
        for (String abi : Build.SUPPORTED_ABIS) if ("arm64-v8a".equals(abi)) arm64 = true;
        addCheck("Arquitetura", arm64 ? "ARM64 disponível" : "ARM64 não detectado", arm64, false);
        addCheck("Android", "Versão " + Build.VERSION.RELEASE + " • API " + Build.VERSION.SDK_INT,
                Build.VERSION.SDK_INT >= 26, false);

        ActivityManager.MemoryInfo memory = new ActivityManager.MemoryInfo();
        ((ActivityManager)getSystemService(ACTIVITY_SERVICE)).getMemoryInfo(memory);
        long ram = memory.totalMem / (1024L * 1024L * 1024L);
        addCheck("Memória RAM", String.format(Locale.US, "Cerca de %d GB", ram), ram >= 4, ram >= 3);

        StatFs storage = new StatFs(getFilesDir().getAbsolutePath());
        long free = storage.getAvailableBytes() / (1024L * 1024L * 1024L);
        addCheck("Armazenamento", String.format(Locale.US, "%d GB livres na área do app", free), free >= 4, free >= 2);

        boolean vulkan = getPackageManager().hasSystemFeature(PackageManager.FEATURE_VULKAN_HARDWARE_LEVEL);
        addCheck("Vulkan", vulkan ? "Recurso Vulkan anunciado pelo Android" : "Vulkan não anunciado pelo sistema", vulkan, !vulkan);
        addCheck("Arquivos do jogo", hasValidGameFolder() ? "Forza.xbe e media encontrados" : "Pasta ainda não validada",
                hasValidGameFolder(), !hasValidGameFolder());
    }

    private void chooseGameFolder() {
        Intent intent = new Intent(Intent.ACTION_OPEN_DOCUMENT_TREE);
        intent.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION | Intent.FLAG_GRANT_PERSISTABLE_URI_PERMISSION);
        startActivityForResult(intent, PICK_GAME_FOLDER);
    }

    @Override protected void onActivityResult(int request, int result, Intent data) {
        super.onActivityResult(request, result, data);
        if (request == PICK_GAME_FOLDER && result == RESULT_OK && data != null && data.getData() != null) {
            gameTree = data.getData();
            int flags = data.getFlags() & (Intent.FLAG_GRANT_READ_URI_PERMISSION | Intent.FLAG_GRANT_WRITE_URI_PERMISSION);
            try { getContentResolver().takePersistableUriPermission(gameTree, flags); } catch (SecurityException ignored) { }
            prefs.edit().putString("game_tree", gameTree.toString()).apply();
            updateGameLabel();
            refreshChecks();
        }
    }

    private boolean hasValidGameFolder() {
        if (gameTree == null) return false;
        try {
            Uri children = DocumentsContract.buildChildDocumentsUriUsingTree(gameTree, DocumentsContract.getTreeDocumentId(gameTree));
            String[] projection = {DocumentsContract.Document.COLUMN_DISPLAY_NAME, DocumentsContract.Document.COLUMN_MIME_TYPE};
            boolean xbe = false, media = false;
            try (android.database.Cursor cursor = getContentResolver().query(children, projection, null, null, null)) {
                if (cursor == null) return false;
                while (cursor.moveToNext()) {
                    String name = cursor.getString(0);
                    String mime = cursor.getString(1);
                    if (name != null && "Forza.xbe".equalsIgnoreCase(name)) xbe = true;
                    if (name != null && "media".equalsIgnoreCase(name) && DocumentsContract.Document.MIME_TYPE_DIR.equals(mime)) media = true;
                }
            }
            return xbe && media;
        } catch (Exception e) { return false; }
    }

    private void updateGameLabel() {
        if (gameStatus == null) return;
        gameStatus.setText(hasValidGameFolder() ? "Estrutura básica do jogo encontrada" : "Nenhuma pasta válida selecionada");
    }

    private void openDownloadPage() {
        try {
            startActivity(new Intent(Intent.ACTION_VIEW, Uri.parse(DOWNLOAD_URL)));
        } catch (Exception e) {
            Toast.makeText(this, "Não foi possível abrir o GitHub.", Toast.LENGTH_LONG).show();
        }
    }
}
