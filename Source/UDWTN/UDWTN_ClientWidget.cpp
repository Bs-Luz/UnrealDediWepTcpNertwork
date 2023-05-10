// Fill out your copyright notice in the Description page of Project Settings.

#include "UDWTNGameMode.h"
#include "UDWTN_ClientWidget.h"

void UUDWTN_ClientWidget::NativeConstruct()
{
    UButton* LoginButton = Cast<UButton>(GetWidgetFromName(TEXT("LoginButton")));
    UButton* SignUpButton = Cast<UButton>(GetWidgetFromName(TEXT("SignUpButton")));
    // 버튼 이벤트 핸들러 등록하기
    if (LoginButton)
    {
        LoginButton->OnClicked.AddDynamic(this, &UUDWTN_ClientWidget::OnButtonClickToLogin);
        //UE_LOG(LogTemp, Warning, TEXT("로그인 버튼 이벤트야!!"));
    }
    if (SignUpButton)
    {
        SignUpButton->OnClicked.AddDynamic(this, &UUDWTN_ClientWidget::OnButtonClickToSignUp);
        //UE_LOG(LogTemp, Warning, TEXT("회원가입창으로 넘어가는 버튼 이벤트야!!"));

    }

    UButton* CreateSignUpButton = Cast<UButton>(GetWidgetFromName(TEXT("CreateSignUpButton")));

    UButton* BackButton = Cast<UButton>(GetWidgetFromName(TEXT("BackButton")));
    // 버튼 이벤트 핸들러 등록하기
    if (CreateSignUpButton)
    {
        CreateSignUpButton->OnClicked.AddDynamic(this, &UUDWTN_ClientWidget::OnButtonClickToCreateSignUp);
    }
    if (BackButton)
    {
        BackButton->OnClicked.AddDynamic(this, &UUDWTN_ClientWidget::OnButtonClickToBack);
    }
}

void UUDWTN_ClientWidget::OnButtonClickToLogin()
{
    //무조건 접속하게하는 법 
    AUDWTNGameMode* gamemode = Cast<AUDWTNGameMode>(GetWorld()->GetAuthGameMode());
    if (gamemode != NULL) gamemode->ServerToInfoClient();

    //UE_LOG(LogTemp, Warning, TEXT("로그인 버튼 이벤트야"));

    //// Id와 Password 입력값 저장
    //if (Id && Id->HasKeyboardFocus())
    //{
    //    FId = Id->GetText().ToString();
    //}
    //else if (Password && Password->HasKeyboardFocus())
    //{
    //    FPassword = Password->GetText().ToString();
    //}

    //// JSON 데이터 생성
    //TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
    //JsonObject->SetStringField(TEXT("email"), FId);
    //JsonObject->SetStringField(TEXT("password"), FPassword);

    //// JSON 데이터를 문자열로 변환
    //FString JsonString;
    //TSharedRef<TJsonWriter<TCHAR>> JsonWriter = TJsonWriterFactory<>::Create(&JsonString);
    //FJsonSerializer::Serialize(JsonObject.ToSharedRef(), JsonWriter);

    //// HTTP 요청 생성 및 설정
    //TSharedRef<IHttpRequest> HttpRequest = FHttpModule::Get().CreateRequest();
    //HttpRequest->SetVerb(TEXT("POST"));
    //HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    //HttpRequest->SetURL(TEXT("http://192.168.0.10:8080/login"));
    //HttpRequest->SetContentAsString(JsonString);

    //// 요청 완료 시 호출할 함수 설정
    //HttpRequest->OnProcessRequestComplete().BindUObject(this, &UUDWTN_ClientWidget::HttpRequestComplete);

    //// 요청 전송
    //HttpRequest->ProcessRequest();
}

void UUDWTN_ClientWidget::OnButtonClickToSignUp()
{
    RemoveFromParent();

    FSoftClassPath testwidget(TEXT("/Script/UMGEditor.WidgetBlueprint'/Game/ThirdPerson/Blueprints/UDWTN_ClientSignUpWidget.UDWTN_ClientSignUpWidget_C'"));
    UClass* widgetClass = testwidget.TryLoadClass<UUDWTN_ClientWidget>();
    UUDWTN_ClientWidget* widgetObject = CreateWidget<UUDWTN_ClientWidget>(this, widgetClass);

    if (widgetObject)
    {
        widgetObject->AddToViewport();
    }

    UE_LOG(LogTemp, Warning, TEXT("회원가입창으로 넘어가는 버튼 이벤트야"));
}

void UUDWTN_ClientWidget::OnButtonClickToCreateSignUp()
{
    UE_LOG(LogTemp, Warning, TEXT("회원가입 하는 버튼 이벤트야"));

    // Id와 Password 입력값 저장
    if (Id && Id->HasKeyboardFocus())
    {
        FId = Id->GetText().ToString();
    }
    else if (Password && Password->HasKeyboardFocus())
    {
        FPassword = Password->GetText().ToString();
    }

    // JSON 데이터 생성
    TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
    JsonObject->SetStringField(TEXT("email"), FId);
    JsonObject->SetStringField(TEXT("password"), FPassword);

    // JSON 데이터를 문자열로 변환
    FString JsonString;
    TSharedRef<TJsonWriter<TCHAR>> JsonWriter = TJsonWriterFactory<>::Create(&JsonString);
    FJsonSerializer::Serialize(JsonObject.ToSharedRef(), JsonWriter);

    // HTTP 요청 생성 및 설정
    TSharedRef<IHttpRequest> HttpRequest = FHttpModule::Get().CreateRequest();
    HttpRequest->SetVerb(TEXT("POST"));
    HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    HttpRequest->SetURL(TEXT("http://192.168.0.10:8080/signup"));
    HttpRequest->SetContentAsString(JsonString);

    // 요청 완료 시 호출할 함수 설정
    HttpRequest->OnProcessRequestComplete().BindUObject(this, &UUDWTN_ClientWidget::HttpRequestComplete);

    // 요청 전송
    HttpRequest->ProcessRequest();
}

void UUDWTN_ClientWidget::OnButtonClickToBack()
{
    // 현재 위젯 제거하기
    RemoveFromParent();

    FSoftClassPath testwidget(TEXT("/Script/UMGEditor.WidgetBlueprint'/Game/ThirdPerson/Blueprints/UDWTN_ClientLoginWidget.UDWTN_ClientLoginWidget_C'"));
    UClass* widgetClass = testwidget.TryLoadClass<UUDWTN_ClientWidget>();
    UUDWTN_ClientWidget* widgetObject = CreateWidget<UUDWTN_ClientWidget>(this, widgetClass);

    if (widgetObject)
    {
        widgetObject->AddToViewport();
    }

    //UE_LOG(LogTemp, Warning, TEXT("뒤로가기 버튼이야"));
}

void UUDWTN_ClientWidget::LoginInfoToTcpServer(FString)
{
}

bool UUDWTN_ClientWidget::ConnectToDedicatedServer()
{
    // TCP 소켓 생성
    Socket = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateSocket(NAME_Stream, TEXT("default"), false);

    // TCP 서버의 IP 주소와 포트 번호로 주소를 설정
    TSharedPtr<FInternetAddr> Addr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
    bool bIsValid;
    Addr->SetIp(*TcpSererIpAdd, bIsValid);
    Addr->SetPort(TcpServerPort);

    // TCP 서버에 접속 시도
    if (!Socket->Connect(*Addr))
    {
        UE_LOG(LogTemp, Warning, TEXT("TCP 서버에 접속할 수 없어"));
        return false;
    }

    // TCP 서버에서 데디서버의 정보 수신
    FString DedicateServerInfo;
    int32 ReceivedSize = 0;
    while (ReceivedSize == 0)
    {
        if (Socket->Recv(reinterpret_cast<uint8*>(DedicateServerInfo.GetCharArray().GetData()), DedicateServerInfo.Len(), ReceivedSize))
        {
            DedicateServerInfo = DedicateServerInfo.Left(ReceivedSize);
        }
    }

    // 데디서버에 접속
    if (DedicateServerInfo.Contains(ClientInfo))
    {
        UE_LOG(LogTemp, Warning, TEXT("데디서버에 접속했어"));

        return true;
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("데디서버에 접속 할 수 없어"));
        return false;
    }
}

void UUDWTN_ClientWidget::HttpRequestComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess)
{
    if (bSuccess && Response.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("Response: %s"), *Response->GetContentAsString());

        // Convert response json to object
        TSharedPtr<FJsonObject> JsonObject;
        TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create(Response->GetContentAsString());
        if (FJsonSerializer::Deserialize(JsonReader, JsonObject))
        {
            // Get response fields
            bool bLoginSuccess = JsonObject->GetBoolField("success");
            FString message = JsonObject->GetStringField("message");

            if (bLoginSuccess)
            {
                UE_LOG(LogTemp, Warning, TEXT("로그인 성공!!: %s"), *message);

                ConnectToDedicatedServer();
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("로그인 실패...: %s"), *message);
            }
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("웹서버에서 응답을 가져올 수 없어"));

        ConnectToDedicatedServer();
    }
}