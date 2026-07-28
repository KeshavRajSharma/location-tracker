import { Text, View } from "react-native";

interface StatusBadgeProps {
  label: string;
  active: boolean;
}

export default function StatusBadge({ label, active }: StatusBadgeProps) {
  const containerStyle = active ? "bg-emerald-100" : "bg-red-100";
  const dotStyle = active ? "bg-emerald-500" : "bg-red-500";
  const textStyle = active ? "text-emerald-700" : "text-red-700";

  return (
    <View
      className={`flex-row items-center rounded-full px-3 py-2 ${containerStyle}`}
    >
      <View className={`mr-2 h-2.5 w-2.5 rounded-full ${dotStyle}`} />

      <Text className={`text-sm font-semibold ${textStyle}`}>{label}</Text>
    </View>
  );
}
